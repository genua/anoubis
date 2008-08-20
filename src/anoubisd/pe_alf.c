/*
 * Copyright (c) 2008 GeNUA mbH <info@genua.de>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"

#ifdef S_SPLINT_S
#include "splint-includes.h"
#endif

#include <sys/types.h>
#include <sys/socket.h>
#ifdef OPENBSD
#include <sys/limits.h>
#endif

#include <arpa/inet.h>
#include <netinet/in.h>

#include <apn.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#ifdef OPENBSD
#ifndef s6_addr32
#define s6_addr32 __u6_addr.__u6_addr32
#endif
#endif

#ifdef LINUX
#include <queue.h>
#include <bsdcompat.h>
#include <linux/anoubis_alf.h>
#include <linux/anoubis.h>
#endif
#ifdef OPENBSD
#include <sys/queue.h>
#include <dev/anoubis_alf.h>
#include <dev/anoubis.h>
#endif

#include "anoubisd.h"
#include "pe.h"

static int		 pe_alf_evaluate(struct pe_proc *, struct pe_context *,
			     struct alf_event *, int *, u_int32_t *);
static int		 pe_decide_alffilt(struct pe_proc *, struct apn_rule *,
			     struct alf_event *, int *, u_int32_t *, time_t);
static int		 pe_decide_alfcap(struct apn_rule *, struct
			     alf_event *, int *, u_int32_t *, time_t);
static int		 pe_decide_alfdflt(struct apn_rule *, struct
			     alf_event *, int *, u_int32_t *, time_t);
static void		 pe_kcache_alf(struct apn_alfrule *, int,
			     struct pe_proc *, struct alf_event *);
static char		*pe_dump_alfmsg(struct alf_event *,
			     struct eventdev_hdr *, int);
static int		 pe_addrmatch_out(struct alf_event *, struct
			     apn_alfrule *);
static int		 pe_addrmatch_in(struct alf_event *, struct
			     apn_alfrule *);
static int		 pe_addrmatch_host(struct apn_host *, void *,
			     unsigned short);
static int		 pe_addrmatch_port(struct apn_port *, void *,
			     unsigned short);


anoubisd_reply_t *
pe_decide_alf(struct pe_proc *proc, struct eventdev_hdr *hdr)
{
	static char		 prefix[] = "ALF";
	static char		*verdict[3] = { "allowed", "denied", "asked" };
	struct alf_event	*msg;
	anoubisd_reply_t	*reply;
	int			 i, ret, decision, log, prio;
	u_int32_t		 rule_id = 0;
	char			*dump = NULL;
	char			*context = NULL;

	if (hdr == NULL) {
		log_warnx("pe_decide_alf: empty header");
		return (NULL);
	}
	if ((unsigned short)hdr->msg_size < (sizeof(struct eventdev_hdr) +
	    sizeof(struct alf_event))) {
		log_warnx("pe_decide_alf: short message");
		return (NULL);
	}
	msg = (struct alf_event *)(hdr + 1);

	log = 0;
	prio = -1;
	rule_id = 0;
	decision = -1;

	for (i = 0; i < PE_PRIO_MAX; i++) {
		DEBUG(DBG_PE_DECALF, "pe_decide_alf: prio %d context %p", i,
		    pe_proc_get_context(proc, i));

		ret = pe_alf_evaluate(proc, pe_proc_get_context(proc, i),
		    msg, &log, &rule_id);

		if (ret != -1) {
			decision = ret;
			prio = i;
		}
		if (ret == POLICY_DENY) {
			break;
		}
	}
	DEBUG(DBG_PE_DECALF, "pe_decide_alf: decision %d", decision);

	/* If no default rule matched, decide on deny */
	if (decision == -1) {
		decision = POLICY_DENY;
		log = APN_LOG_ALERT;
	}

	if (log != APN_LOG_NONE) {
		context = pe_context_dump(hdr, proc, prio);
		dump = pe_dump_alfmsg(msg, hdr, ANOUBISD_LOG_APN);
	}

	/* Logging */
	switch (log) {
	case APN_LOG_NONE:
		break;
	case APN_LOG_NORMAL:
		log_info("%s prio %d rule %d %s %s (%s)", prefix, prio,
		    rule_id, verdict[decision], dump, context);
		send_lognotify(hdr, decision, log, rule_id);
		break;
	case APN_LOG_ALERT:
		log_warnx("%s prio %d rule %d %s %s (%s)", prefix, prio,
		    rule_id, verdict[decision], dump, context);
		send_lognotify(hdr, decision, log, rule_id);
		break;
	default:
		log_warnx("pe_decide_alf: unknown log type %d", log);
	}

	if (dump)
		free(dump);
	if (context)
		free(context);

	if ((reply = calloc(1, sizeof(struct anoubisd_reply))) == NULL) {
		log_warn("pe_decide_alf: cannot allocate memory");
		master_terminate(ENOMEM);
		return (NULL);
	}

	reply->ask = 0;
	reply->rule_id = rule_id;
	reply->timeout = (time_t)0;
	if (decision == POLICY_ASK) {
		struct pe_proc_ident *pident = pe_proc_ident(proc);
		reply->ask = 1;
		reply->timeout = 300;	/* XXX 5 Minutes for now. */
		reply->csum = pident->csum;
		reply->path = pident->pathhint;
	}
	reply->reply = decision;
	reply->len = 0;

	return (reply);
}

static int
pe_alf_evaluate(struct pe_proc *proc, struct pe_context *context,
    struct alf_event *msg, int *log, u_int32_t *rule_id)
{
	struct apn_rule	*rule;
	int		 decision;
	time_t		 t;

	if (context == NULL)
		return (-1);
	if ((rule = pe_context_get_rule(context)) == NULL) {
		log_warnx("pe_alf_evaluate: empty rule");
		return (-1);
	}
	if (msg == NULL) {
		log_warnx("pe_alf_evaluate:: empty alf event");
		return (-1);
	}

	if (time(&t) == (time_t)-1) {
		int code = errno;
		log_warn("Cannot get current time");
		master_terminate(code);
		return -1;
	}
	decision = pe_decide_alffilt(proc, rule, msg, log, rule_id, t);
	if (decision == -1)
		decision = pe_decide_alfcap(rule, msg, log, rule_id, t);
	if (decision == -1)
		decision = pe_decide_alfdflt(rule, msg, log, rule_id, t);

	DEBUG(DBG_PE_DECALF, "pe_alf_evaluate: decision %d rule %p", decision,
	    rule);

	return (decision);
}

/*
 * ALF filter logic.
 */
static int
pe_decide_alffilt(struct pe_proc *proc, struct apn_rule *rule,
    struct alf_event *msg, int *log, u_int32_t *rule_id, time_t now)
{
	struct apn_alfrule	*hp;
	int			 decision;

	if (rule == NULL) {
		log_warnx("pe_decide_alffilt: empty rule");
		return (-1);
	}
	if (msg == NULL) {
		log_warnx("pe_decide_alffilt: empty alf event");
		return (-1);
	}

	/* We only decide on UDP and TCP. */
	if (msg->protocol != IPPROTO_UDP && msg->protocol != IPPROTO_TCP)
		return (-1);

	/* For TCP, we only decide on ACCEPT/CONNECT but allow SEND/RECVMSG */
	if ((msg->op == ALF_SENDMSG || msg->op == ALF_RECVMSG) &&
	    msg->protocol == IPPROTO_TCP) {
		if (log)
			*log = APN_LOG_NONE;
		return POLICY_ALLOW;
	}


	decision = -1;
	for (hp = rule->rule.alf; hp; hp = hp->next) {
		/*
		 * Skip non-filter rules.
		 */
		if (hp->type != APN_ALF_FILTER
		    || !pe_in_scope(hp->scope, &msg->common, now))
			continue;

		DEBUG(DBG_PE_DECALF, "pe_decide_alffilt: family %d == %d/%p?",
		    msg->family, hp->rule.afilt.filtspec.af, hp);
		/*
		 * Address family:
		 * If the rule does not specify a certain family (ie.
		 * AF_UNSPEC), we accept any address family.  Otherwise,
		 * families have to match.
		 */
		if (msg->family != hp->rule.afilt.filtspec.af &&
		    hp->rule.afilt.filtspec.af != AF_UNSPEC)
			continue;

		DEBUG(DBG_PE_DECALF, "pe_decide_alffilt: operation %d",
		    msg->op);
		/*
		 * Operation
		 */
		switch (msg->op) {
		case ALF_ANY:
			/* XXX HSH ? */
			continue;

		case ALF_CONNECT:
			if (hp->rule.afilt.filtspec.netaccess != APN_CONNECT &&
			    hp->rule.afilt.filtspec.netaccess != APN_SEND &&
			    hp->rule.afilt.filtspec.netaccess != APN_BOTH)
				continue;
			break;

		case ALF_ACCEPT:
			if (hp->rule.afilt.filtspec.netaccess != APN_ACCEPT &&
			    hp->rule.afilt.filtspec.netaccess != APN_RECEIVE &&
			    hp->rule.afilt.filtspec.netaccess != APN_BOTH)
				continue;
			break;

		case ALF_SENDMSG:
		case ALF_RECVMSG:
			break;

		default:
			continue;
		}

		DEBUG(DBG_PE_DECALF, "pe_decide_alffilt: socket type %d",
		   msg->type);
		/*
		 * Socket type
		 */
		if (msg->type != SOCK_STREAM && msg->type != SOCK_DGRAM)
			continue;

		DEBUG(DBG_PE_DECALF, "pe_decide_alffilt: protocol %d",
		    msg->protocol);
		/*
		 * Protocol
		 */
		if (msg->protocol != hp->rule.afilt.filtspec.proto)
			continue;

		/*
		 * Check addresses.
		 */
		DEBUG(DBG_PE_DECALF, "pe_decide_alffilt: address check op %d",
		    msg->op);
		switch (msg->protocol) {
		case IPPROTO_TCP:
			if (msg->op == ALF_CONNECT) {
				if (pe_addrmatch_out(msg, hp) == 0)
					continue;
			} else if (msg->op == ALF_ACCEPT) {
				if (pe_addrmatch_in(msg, hp) == 0)
					continue;
			} else if (msg->op == ALF_SENDMSG || msg->op ==
			    ALF_RECVMSG) {
				/*
				 * this case is now handled above, this code
				 * is no longer in use
				 */
				if (pe_addrmatch_out(msg, hp) == 0 &&
				    pe_addrmatch_in(msg, hp) == 0)
					continue;
			} else
				continue;
			break;

		case IPPROTO_UDP:
			if (msg->op == ALF_CONNECT || msg->op == ALF_SENDMSG) {
				if (pe_addrmatch_out(msg, hp) == 0)
					continue;
			} else if (msg->op == ALF_ACCEPT || msg->op ==
			    ALF_RECVMSG) {
				if (pe_addrmatch_in(msg, hp) == 0)
					continue;
			} else
				continue;
			break;

		default:
			continue;
		}

		/*
		 * Decide.
		 */
		switch (hp->rule.afilt.action) {
		case APN_ACTION_ALLOW:
			decision = POLICY_ALLOW;
			pe_kcache_alf(hp, decision, proc, msg);
			break;
		case APN_ACTION_DENY:
			decision = POLICY_DENY;
			pe_kcache_alf(hp, decision, proc, msg);
			break;
		case APN_ACTION_ASK:
			decision = POLICY_ASK;
			break;
		default:
			log_warnx("pe_decide_alffilt: invalid action %d",
			    hp->rule.afilt.action);
			continue;
		}

		DEBUG(DBG_PE_DECALF, "pe_decide_alffilt: decision %d",
		    decision);

		if (log)
			*log = hp->rule.afilt.filtspec.log;
		if (rule_id)
			*rule_id = hp->id;
		break;
	}

	return (decision);
}

static void
pe_kcache_alf(struct apn_alfrule *rule, int decision, struct pe_proc *proc,
    struct alf_event *msg)
{
	struct anoubis_kernel_policy *policy;
	struct alf_rule *alfrule;

	if (msg->op != ALF_SENDMSG && msg->op != ALF_RECVMSG)
		return;

	policy = malloc(sizeof(struct anoubis_kernel_policy) +
	    sizeof(struct alf_rule));
	if (policy == NULL)
		return;

	policy->anoubis_source = ANOUBIS_SOURCE_ALF;
	policy->rule_len = sizeof(struct alf_rule);
	policy->decision = decision;
	if (rule->rule.afilt.filtspec.statetimeout > 0) {
		policy->expire = rule->rule.afilt.filtspec.statetimeout +
		    time(NULL);
	} else {
		policy->expire = 0;
	}

	alfrule = (struct alf_rule*)(policy->rule);

	alfrule->family = msg->family;
	alfrule->protocol = msg->protocol;
	alfrule->type = msg->type;
	alfrule->op = msg->op;

	switch(alfrule->family) {
	case AF_INET:
		alfrule->local.port_min = msg->local.in_addr.sin_port;
		alfrule->local.port_max = msg->local.in_addr.sin_port;
		alfrule->peer.port_min = msg->peer.in_addr.sin_port;
		alfrule->peer.port_max = msg->peer.in_addr.sin_port;
		alfrule->local.addr.in_addr =
		    msg->local.in_addr.sin_addr;
		alfrule->peer.addr.in_addr =
		    msg->peer.in_addr.sin_addr;
		break;

	case AF_INET6:
		alfrule->local.port_min = msg->local.in6_addr.sin6_port;
		alfrule->local.port_max = msg->local.in6_addr.sin6_port;
		alfrule->peer.port_min = msg->peer.in6_addr.sin6_port;
		alfrule->peer.port_max = msg->peer.in6_addr.sin6_port;
		alfrule->local.addr.in6_addr =
		    msg->local.in6_addr.sin6_addr;
		alfrule->peer.addr.in6_addr =
		    msg->peer.in6_addr.sin6_addr;
		break;
	}

	pe_proc_kcache_add(proc, policy);

	free(policy);
}

/*
 * ALF capability logic.
 */
static int
pe_decide_alfcap(struct apn_rule *rule, struct alf_event *msg, int *log,
    u_int32_t *rule_id, time_t now)
{
	int			 decision;
	struct apn_alfrule	*hp;

	if (rule == NULL) {
		log_warnx("pe_decide_alfcap: empty rule");
		return (POLICY_DENY);
	}
	if (msg == NULL) {
		log_warnx("pe_decide_alfcap: empty alf event");
		return (POLICY_DENY);
	}

	/* We decide on everything except UDP and TCP */
	if (msg->protocol == IPPROTO_UDP || msg->protocol == IPPROTO_TCP)
		return (-1);

	decision = -1;
	hp = rule->rule.alf;
	while (hp) {
		/* Skip non-capability rules. */
		if (hp->type != APN_ALF_CAPABILITY ||
		    !pe_in_scope(hp->scope, &msg->common, now)) {
			hp = hp->next;
			continue;
		}
		/*
		 * We have three types of capabilities:
		 *
		 * APN_ALF_CAPRAW: We allow packets with socket type SOCK_RAW
		 * and SOCK_PACKET (ie. ping, dhclient)
		 *
		 * APN_ALF_CAPOTHER: We allow everything else.
		 *
		 * APN_ALF_CAPALL: Allow anything.
		 */
		switch (hp->rule.acap.capability) {
		case APN_ALF_CAPRAW:
			if (msg->type == SOCK_RAW)
				decision = POLICY_ALLOW;
#ifdef LINUX
			if (msg->type == SOCK_PACKET)
				decision = POLICY_ALLOW;
#endif
			break;
		case APN_ALF_CAPOTHER:
#ifdef LINUX
			if (msg->type != SOCK_RAW && msg->type != SOCK_PACKET)
#else
			if (msg->type != SOCK_RAW)
#endif
				decision = POLICY_ALLOW;
			break;
		case APN_ALF_CAPALL:
			decision = POLICY_ALLOW;
			break;
		default:
			log_warnx("pe_decide_alfcap: unknown capability %d",
			    hp->rule.acap.capability);
			decision = -1;
		}

		if (decision != -1) {
			if (log)
				*log = hp->rule.acap.log;
			if (rule_id)
				*rule_id = hp->id;
			break;
		}
		hp = hp->next;
	}

	return (decision);
}

/*
 * ALF default logic.
 */
static int
pe_decide_alfdflt(struct apn_rule *rule, struct alf_event *msg, int *log,
    u_int32_t *rule_id, time_t now)
{
	struct apn_alfrule	*hp;
	int			 decision;

	if (rule == NULL) {
		log_warnx("pe_decide_alfdflt: empty rule");
		return (-1);
	}

	decision = -1;
	hp = rule->rule.alf;
	while (hp) {
		/* Skip non-default rules. */
		if (hp->type != APN_ALF_DEFAULT
		    || !pe_in_scope(hp->scope, &msg->common, now)) {
			hp = hp->next;
			continue;
		}
		/*
		 * Default rules are easy, just pick the specified action.
		 */
		switch (hp->rule.apndefault.action) {
		case APN_ACTION_ALLOW:
			decision = POLICY_ALLOW;
			break;
		case APN_ACTION_DENY:
			decision = POLICY_DENY;
			break;
		case APN_ACTION_ASK:
			decision = POLICY_ASK;
			break;
		default:
		log_warnx("pe_decide_alfdflt: unknown action %d",
			    hp->rule.apndefault.action);
			decision = -1;
		}

		if (decision != -1) {
			if (log)
				*log = hp->rule.apndefault.log;
			if (rule_id)
				*rule_id = hp->id;
			break;
		}
		hp = hp->next;
	}

	return (decision);
}

/*
 * Decode an ALF message into a printable string.  This strings is
 * allocated and needs to be freed by the caller.
 */
static char *
pe_dump_alfmsg(struct alf_event *msg, struct eventdev_hdr *hdr, int format)
{
	unsigned short	 lport, pport;
	char		*op, *af, *type, *proto, *dump;

	/*
	 * v4 address: 4 * 3 digits + 3 dots + 1 \0 = 16
	 * v6 address: 16 * 4 digits + 15 colons + 1 \0 = 79
	 *
	 * v6 address might also have some leading colons, etc. so 128
	 * bytes are more than sufficient and safe.
	 */
	char		 local[128], peer[128];

	if (msg == NULL)
		return (NULL);

	pport = lport = 0;
	snprintf(local, 128, "<unknown>");
	snprintf(peer, 128, "<unknown>");

	switch (msg->op) {
	case ALF_ANY:
		op = "any";
		break;
	case ALF_CONNECT:
		op = "connect";
		break;
	case ALF_ACCEPT:
		op = "accept";
		break;
	case ALF_SENDMSG:
		op = "sendmsg";
		break;
	case ALF_RECVMSG:
		op = "recvmsg";
		break;
	default:
		op = "<unknown>";
	}

	switch(msg->family) {
	case AF_INET:
		af = "inet";
		lport = ntohs(msg->local.in_addr.sin_port);
		pport = ntohs(msg->peer.in_addr.sin_port);

		if (inet_ntop(msg->family, &msg->local.in_addr.sin_addr,
		    local, sizeof(local)) == NULL)
			snprintf(local, 128, "<unknown>");
		if (inet_ntop(msg->family, &msg->peer.in_addr.sin_addr, peer,
		    sizeof(peer)) == NULL)
			snprintf(peer, 128, "<unknown>");
		break;

	case AF_INET6:
		af = "inet6";
		lport = ntohs(msg->local.in6_addr.sin6_port);
		pport = ntohs(msg->peer.in6_addr.sin6_port);

		if (inet_ntop(msg->family, &msg->local.in6_addr.sin6_addr,
		    local, sizeof(local)))
			snprintf(local, 128, "<unknown>");
		if (inet_ntop(msg->family, &msg->peer.in6_addr.sin6_addr, peer,
		    sizeof(peer)) == NULL)
			snprintf(peer, 128, "<unknown>");
		break;
#ifdef LINUX
	case AF_PACKET:
		af = "packet";
		break;
#endif
	case AF_UNSPEC:
		af = "unspec";
		break;
	default:
		af = "<unknown>";
	}
	switch(msg->type) {
	case SOCK_STREAM:
		type = "stream";
		break;
	case SOCK_DGRAM:
		type = "dgram";
		break;
	case SOCK_RAW:
		type = "raw";
		break;
	case SOCK_RDM:
		type = "rdm";
		break;
	case SOCK_SEQPACKET:
		type = "seqpacket";
		break;
#ifdef LINUX
	case SOCK_PACKET:
		type = "packet";
		break;
#endif
	default:
		type = "<unknown>";
	}
	switch (msg->protocol) {
	case IPPROTO_ICMP:
		proto = "icmp";
		break;
	case IPPROTO_UDP:
		proto = "udp";
		break;
	case IPPROTO_TCP:
		proto = "tcp";
		break;
	default:
		proto = "<unknown>";
	}

	switch (format) {
	case ANOUBISD_LOG_RAW:
		if (asprintf(&dump, "%s (%u): uid %u pid %u %s (%u) %s (%u) "
		    "%s (%u) local %s:%hu peer %s:%hu", op, msg->op,
		    hdr->msg_uid, hdr->msg_pid, type, msg->type, proto,
		    msg->protocol, af, msg->family, local, lport, peer, pport)
		    == -1) {
			dump = NULL;
		}
		break;

	case ANOUBISD_LOG_APN:
		if (asprintf(&dump, "%s %s %s from %s port %hu to %s port %hu",
		    op, af, proto, local, lport, peer, pport) == -1) {
			dump = NULL;
		}
		break;

	default:
		log_warnx("pe_dump_alfmsg: illegal logging format %d", format);
		dump = NULL;
	}

	return (dump);
}

/*
 * Check outgoing connection. 1 on match, 0 otherwise.
 */
static int
pe_addrmatch_out(struct alf_event *msg, struct apn_alfrule *rule)
{
	struct apn_host	*fromhost, *tohost;
	struct apn_port	*fromport, *toport;

	DEBUG(DBG_PE_DECALF, "pe_addrmatch_out: msg %p rule %p", msg, rule);

	if (rule == NULL) {
		log_warnx("pe_addrmatch_out: empty rule");
		return (0);
	}
	if (msg == NULL) {
		log_warnx("pe_addrmatch_out: empty event");
		return (0);
	}

	fromhost = rule->rule.afilt.filtspec.fromhost;
	fromport = rule->rule.afilt.filtspec.fromport;;
	tohost = rule->rule.afilt.filtspec.tohost;
	toport = rule->rule.afilt.filtspec.toport;

	/*
	 * fromhost == local?
	 */
	if (pe_addrmatch_host(fromhost, (void *)&msg->local, msg->family) == 0)
		return (0);
	if (pe_addrmatch_port(fromport, (void *)&msg->local, msg->family) == 0)
		return (0);

	/*
	 * tohost == peer?
	 */
	if (pe_addrmatch_host(tohost, (void *)&msg->peer, msg->family) == 0)
		return (0);
	if (pe_addrmatch_port(toport, (void *)&msg->peer, msg->family) == 0)
		return (0);

	return (1);
}

/*
 * Check incoming connection. 1 on match, 0 otherwise.
 */
static int
pe_addrmatch_in(struct alf_event *msg, struct apn_alfrule *rule)
{
	struct apn_host	*fromhost, *tohost;
	struct apn_port	*fromport, *toport;

	DEBUG(DBG_PE_DECALF, "pe_addrmatch_in: msg %p rule %p", msg, rule);

	if (rule == NULL) {
		log_warnx("pe_addrmatch_out: empty rule");
		return (0);
	}
	if (msg == NULL) {
		log_warnx("pe_addrmatch_out: empty event");
		return (0);
	}

	fromhost = rule->rule.afilt.filtspec.fromhost;
	fromport = rule->rule.afilt.filtspec.fromport;;
	tohost = rule->rule.afilt.filtspec.tohost;
	toport = rule->rule.afilt.filtspec.toport;

	/* fromhost == peer? */
	if (pe_addrmatch_host(fromhost, (void *)&msg->peer, msg->family) == 0)
		return (0);
	if (pe_addrmatch_port(fromport, (void *)&msg->peer, msg->family) == 0)
		return (0);

	/* tohost == local? */
	if (pe_addrmatch_host(tohost, (void *)&msg->local, msg->family) == 0)
		return (0);
	if (pe_addrmatch_port(toport, (void *)&msg->local, msg->family) == 0)
		return (0);

	return (1);
}

/*
 * compare addr1 and addr2 with netmask of length len
 */
static int
compare_ip6_mask(struct in6_addr *addr1, struct in6_addr *addr2, int len)
{
	int		 bits, match, i;
	struct in6_addr	 mask;
	u_int32_t	*pmask;

	if (len > 128)
		len = 128;

	if (len == 128)
		return !bcmp(addr1, addr2, sizeof(struct in6_addr));

	if (len < 0)
		len = 0;

	bits = len;
	bzero(&mask, sizeof(struct in6_addr));
	pmask = &mask.s6_addr32[0];
	while (bits >= 32) {
		*pmask = 0xffffffff;
		pmask++;
		bits -= 32;
	}
	if (bits > 0) {
		*pmask = htonl(0xffffffff << (32 - bits));
	}

	match = 1;
	for (i=0; i <=3 ; i++) {
		if ((addr1->s6_addr32[i] & mask.s6_addr32[i]) !=
		    (addr2->s6_addr32[i] & mask.s6_addr32[i])) {
			match = 0;
			break;
		}
	}

	return match;
}

/*
 * Compare addresses. 1 on match, 0 otherwise.
 */
static int
pe_addrmatch_host(struct apn_host *host, void *addr, unsigned short af)
{
	struct apn_host		*hp;
	struct sockaddr_in	*in;
	struct sockaddr_in6	*in6;
	in_addr_t		 mask;
	int			 match;
	int			 negate;

	DEBUG(DBG_PE_DECALF, "pe_addrmatch_host: host %p addr %p af %d", host,
	    addr, af);

	/* Empty host means "any", ie. match always. */
	if (host == NULL)
		return (1);

	if (addr == NULL) {
		log_warnx("pe_addrmatch_host: no address specified");
		return (0);
	}

	if (host->addr.af != af)
		return (0);

	hp = host;
	while (hp) {
		negate = hp->negate;
		switch (af) {
		case AF_INET:
			in = (struct sockaddr_in *)addr;
			mask = htonl(~0UL << (32 - hp->addr.len));
			match = ((in->sin_addr.s_addr & mask) ==
			    (hp->addr.apa.v4.s_addr & mask));
			break;

		case AF_INET6:
			in6 = (struct sockaddr_in6 *)addr;
			match = compare_ip6_mask(&in6->sin6_addr,
			    &hp->addr.apa.v6, hp->addr.len);
			break;

		default:
			log_warnx("pe_addrmatch_host: unknown address family "
			    "%d", af);
			match = negate = 0;
		}
		if ((!negate && match) || (negate && !match))
			break;
		hp = hp->next;
	}

	DEBUG(DBG_PE_DECALF, "pe_addrmatch_host: match %d, negate %d", match,
	    negate);

	if (negate)
	    return (!match);
	else
	    return (match);
}

static int
pe_addrmatch_port(struct apn_port *port, void *addr, unsigned short af)
{
	struct apn_port		*pp;
	struct sockaddr_in	*in;
	struct sockaddr_in6	*in6;
	int			 match;

	DEBUG(DBG_PE_DECALF, "pe_addrmatch_port: port %p addr %p af %d", port,
	    addr, af);

	/* Empty port means "any", ie. match always. */
	if (port == NULL)
		return (1);

	if (addr == NULL) {
		log_warnx("pe_addrmatch_port: no address specified");
		return (0);
	}

	pp = port;
	while (pp) {
		switch (af) {
		case AF_INET:
			in = (struct sockaddr_in *)addr;
			if (pp->port2) {
				match = (in->sin_port >= pp->port &&
					in->sin_port <= pp->port2);
			}
			else
				match = (in->sin_port == pp->port);
			break;

		case AF_INET6:
			in6 = (struct sockaddr_in6 *)addr;
			if (pp->port2) {
				match = (in6->sin6_port >= pp->port &&
					in6->sin6_port <= pp->port2);
			}
			else
				match = (in6->sin6_port == pp->port);
			break;

		default:
			log_warnx("pe_addrmatch_port: unknown address "
			    "family %d", af);
			match = 0;
		}
		if (match)
			break;
		pp = pp->next;
	}

	DEBUG(DBG_PE_DECALF, "pe_addrmatch_port: match %d", match);

	return (match);
}
