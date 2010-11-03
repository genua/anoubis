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
#ifndef IPPROTO_SCTP
#define IPPROTO_SCTP	132
#endif
#endif

#ifdef LINUX
#include <bsdcompat.h>
#include <linux/anoubis_alf.h>
#include <linux/anoubis.h>
#endif
#ifdef OPENBSD
#include <sys/anoubis_alf.h>
#include <dev/anoubis.h>
#endif

#include <sys/queue.h>

#include "anoubisd.h"
#include "pe.h"

static int		 pe_alf_evaluate(struct pe_proc *, int, uid_t,
			     struct alf_event *, int *, u_int32_t *);
static int		 pe_alf_evaluate_rule(struct apn_rule *,
			     struct alf_event *, int *, u_int32_t *, time_t);
static char		*pe_dump_alfmsg(struct alf_event *);
static int		 pe_addrmatch_out(struct alf_event *, struct
			     apn_rule *);
static int		 pe_addrmatch_in(struct alf_event *, struct
			     apn_rule *);
static int		 pe_addrmatch_host(struct apn_host *, void *,
			     unsigned short);
static int		 pe_addrmatch_port(struct apn_port *, void *,
			     unsigned short);

/**
 * Evaluate an ALF event and decide if the event should be allow
 * according to the relevant policies. This is the main entry point
 * to evaluate ALF rules. This function evaluates both admin and
 * user policies.
 *
 * @param proc The process that triggered the event.
 * @hdr The event itslf. It is of type ANOUBIS_SOURCE_ALF.
 * @return The reply for the event. The structure is alloated dynamically
 *     and must be freed by the caller.
 */
struct anoubisd_reply *
pe_decide_alf(struct pe_proc *proc, struct eventdev_hdr *hdr)
{
	static char		*verdict[3] = { "allowed", "denied", "asked" };
	struct alf_event	*msg;
	struct anoubisd_reply	*reply;
	int			 i, ret, decision, log, thislog, prio;
	u_int32_t		 rule_id = 0;
	char			*dump = NULL;
	char			*context = NULL;

	if (hdr == NULL) {
		log_warnx("pe_decide_alf: empty header");
		return (NULL);
	}
	if (hdr->msg_size < (sizeof(struct eventdev_hdr) +
	    sizeof(struct alf_event))) {
		log_warnx("pe_decide_alf: short message");
		return (NULL);
	}
	msg = (struct alf_event *)(hdr + 1);

	log = APN_LOG_NONE;
	prio = -1;
	rule_id = 0;
	decision = -1;

	for (i = 0; i < PE_PRIO_MAX; i++) {
		DEBUG(DBG_PE_DECALF, "pe_decide_alf: prio %d context %p", i,
		    pe_proc_get_context(proc, i));

		thislog = APN_LOG_NONE;
		ret = pe_alf_evaluate(proc, i, hdr->msg_uid, msg,
		    &thislog, &rule_id);
		if (ret != -1) {
			/*
			 * User rules must not decrease the log level
			 * of admin rules. This still allows a user to
			 * change the reported rule ID, though!.
			 */
			if (thislog > log)
				log = thislog;
			decision = ret;
			prio = i;
		}
		if (ret == APN_ACTION_DENY) {
			break;
		}
	}
	DEBUG(DBG_PE_DECALF, "pe_decide_alf: decision %d", decision);

	/* If no default rule matched, decide on deny */
	if (decision == -1) {
		decision = APN_ACTION_DENY;
		log = APN_LOG_ALERT;
	}

	if (log != APN_LOG_NONE) {
		context = pe_context_dump(hdr, proc, prio);
		dump = pe_dump_alfmsg(msg);
	}

	/* Logging */
	switch (log) {
	case APN_LOG_NONE:
		break;
	case APN_LOG_NORMAL:
		log_info("token %u: ALF prio %d rule %d %s %s (%s)",
		    hdr->msg_token, prio, rule_id,
		    verdict[decision], dump, context);
		send_lognotify(proc, hdr, decision, log, rule_id, prio,
		    ANOUBIS_SFS_NONE);
		break;
	case APN_LOG_ALERT:
		log_warnx("token %u: ALF prio %d rule %d %s %s (%s)",
		    hdr->msg_token, prio, rule_id,
		    verdict[decision], dump, context);
		send_lognotify(proc, hdr, decision, log, rule_id, prio,
		    ANOUBIS_SFS_NONE);
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
		master_terminate();
	}

	reply->ask = 0;
	reply->hold = 0;
	reply->rule_id = rule_id;
	reply->prio = prio;
	reply->timeout = (time_t)0;
	reply->log = log;
	if (decision == APN_ACTION_DENY)
		reply->reply = EPERM;
	else
		reply->reply = 0;
	if (decision == APN_ACTION_ASK) {
		reply->ask = 1;
		reply->timeout = 300;
		reply->pident = pe_proc_ident(proc);
		reply->ctxident = pe_context_get_ident(
		    pe_proc_get_context(proc, reply->prio));
	}
	reply->sfsmatch = ANOUBIS_SFS_NONE;

	return (reply);
}

/**
 * Evaluate an ALF event according to the policy given by prio and uid.
 * If the user ID of the process is equal to the user ID in the event,
 * the rules from the processes context are used, otherwise the rules
 * for the application in the ruleset for the uid of the event is used.
 *
 * ALF rules always apply filter rules first. Capability rules are evaluated
 * only if no filter rule matches. Finally, default rules are evaluated.
 * Within one class of rules the evaluation order depends on the order
 * of the rules in the rule block.
 *
 * @param proc The process that triggered the event.
 * @param prio The rule set priority.
 * @param uid The user ID of the event.
 * @param msg The ALF event itself.
 * @param log The log level for the result is returned here. The
 *     caller should initialize this to APN_LOG_NONE.
 * @param rule_id The rule ID of the rule that matched the event is
 *     returned here. The caller should initialize the rule ID to zero.
 * @return The decision of the event. Possible values are the
 *     APN_ACTION_* defines from apn.h. A negative value indicates that
 *     no decision was made.
 */
static int
pe_alf_evaluate(struct pe_proc *proc, int prio, uid_t uid,
    struct alf_event *msg, int *log, u_int32_t *rule_id)
{
	struct apn_rule	*rule;
	int		 decision;
	time_t		 t;
	int		 ispg = (extract_pgid(&msg->common) != 0);

	if (proc && pe_proc_get_uid(proc) == uid) {
		rule = pe_context_get_alfrule(pe_proc_get_context(proc, prio));
	} else {
		struct apn_ruleset *rs = pe_user_get_ruleset(uid, prio, NULL);
		if (rs) {
			TAILQ_FOREACH(rule, &rs->alf_queue, entry) {
				if (!ispg && (rule->flags & APN_RULE_PGONLY))
					continue;
				if (rule->app == NULL)
					break;
			}
		} else {
			rule = NULL;
		}
	}
	if (rule == NULL || msg == NULL)
		return -1;
	if (time(&t) == (time_t)-1) {
		log_warn("Cannot get current time");
		master_terminate();
	}
	decision = pe_alf_evaluate_rule(rule, msg, log, rule_id, t);
	DEBUG(DBG_PE_DECALF, "pe_alf_evaluate: decision %d rule %p", decision,
	    rule);

	return (decision);
}

/**
 * Return true if capability rule appies to a packet that is
 * sent on a particular socket type. We support three different
 * capabilities:
 *
 * APN_ALF_CAPRAW: We allow packets with socket type SOCK_RAW
 *     and SOCK_PACKET (e.g. ping, dhclient)
 * APN_ALF_CAPOTHER: We allow all other socket types.
 * APN_ALF_CAPALL: We allow all socket types.
 *
 * @param rule The capability rule.
 * @param type The socket type.
 */
static int
pe_alf_capmatch(struct apn_rule *rule, int type)
{
	int		israw = 0;

	if (type == SOCK_STREAM || type == SOCK_DGRAM)
		return 0;
#ifdef SOCK_PACKET
	if (type == SOCK_RAW || type == SOCK_PACKET)
		israw = 1;
#else
	if (type == SOCK_RAW)
		israw = 1;
#endif
	switch (rule->rule.acap.capability) {
	case APN_ALF_CAPRAW:
		if (israw)
			return 1;
		break;
	case APN_ALF_CAPOTHER:
		if (!israw)
			return 1;
				break;
	case APN_ALF_CAPALL:
		return 1;
	}
	return 0;
}

/**
 * Evaluate all ALF rules in a rule block and return the decision for
 * the event. This function tries all filter, capability  and default
 * rules in an ALF application rule block and returns the action of the first
 * matching rule. A filter or capability rule always takes precedence over
 * a default rule even if the default rule appears before the filter rule.
 *
 * @param rule The ALF rule block.
 * @param msg The ALF event.
 * @param log The log level for the result is returned here. The
 *     caller should initialize this to APN_LOG_NONE.
 * @param rule_id The rule ID of the rule that matched the event is
 *     returned here. The caller should initialize the rule ID to zero.
 * @param now The current time for scope checks.
 * @return The decision (one of APN_ACTION_* from apn.h) or -1 if no
 *     matching rule was found. Only filter and default rules are taken
 *     into account. If the block contains at least one default rule that
 *     is in scope, this function must not return -1.
 */
static int
pe_alf_evaluate_rule(struct apn_rule *rule, struct alf_event *msg,
    int *log, u_int32_t *rule_id, time_t now)
{
	struct apn_rule		*hp;
	int			 default_decision = -1;
	int			 isfilter = 0;

	if (msg->protocol == IPPROTO_UDP || msg->protocol == IPPROTO_TCP ||
	    msg->protocol == IPPROTO_SCTP)
		isfilter = 1;

	/*
	 * For TCP/SCTP we validate ACCEPT/CONNECT and allow SEND/RECVMSG,
	 * for UDP, we always allow CONNECT events as these do not really
	 * generate network traffic.
	 */
	if ((msg->op == ALF_SENDMSG || msg->op == ALF_RECVMSG) &&
	    (msg->protocol == IPPROTO_TCP || msg->protocol == IPPROTO_SCTP))
		return APN_ACTION_ALLOW;
	if (msg->protocol == IPPROTO_UDP && msg->op == ALF_CONNECT)
		return APN_ACTION_ALLOW;


	TAILQ_FOREACH(hp, &rule->rule.chain, entry) {

		/* Skip rules that are not in scope. */
		if (!pe_in_scope(hp->scope, msg->common.task_cookie, now))
			continue;

		/* Save result of the first default rule. */
		if (hp->apn_type == APN_DEFAULT) {
			if (default_decision == -1) {
				*rule_id = hp->apn_id;
				*log = hp->rule.apndefault.log;
				default_decision = hp->rule.apndefault.action;
			}
			continue;
		}

		/* Apply capability rules. */
		if (hp->apn_type == APN_ALF_CAPABILITY) {
			if (isfilter)
				continue;
			if (!pe_alf_capmatch(hp, msg->type))
				continue;
			*log = hp->rule.afilt.filtspec.log;
			*rule_id = hp->apn_id;
			return hp->rule.afilt.action;
		}

		/*
		 * Skip other non-filter rules. From this point on we
		 * only deal with filter rules.
		 */
		if (hp->apn_type != APN_ALF_FILTER)
			continue;
		if (!isfilter)
			continue;

		/* Check socket type and protocol. */
		if (msg->type != SOCK_STREAM && msg->type != SOCK_DGRAM)
			continue;
		if (msg->protocol != hp->rule.afilt.filtspec.proto)
			continue;

		DEBUG(DBG_PE_DECALF, "pe_alf_evalute_rule: msg family %d, "
		    "msg operation %d", msg->family, msg->op);
		/*
		 * TCP/SCTP SEND and RECEIVE events do not reach this
		 * point at all. Other SEND and RECEIVE events should be
		 * treated by the connect/accept rules.
		 */
		switch (msg->op) {
		case ALF_SENDMSG:
		case ALF_CONNECT:
			if (hp->rule.afilt.filtspec.netaccess != APN_CONNECT &&
			    hp->rule.afilt.filtspec.netaccess != APN_SEND &&
			    hp->rule.afilt.filtspec.netaccess != APN_BOTH)
				continue;
			if (pe_addrmatch_out(msg, hp) == 0)
				continue;
			break;

		case ALF_RECVMSG:
		case ALF_ACCEPT:
			if (hp->rule.afilt.filtspec.netaccess != APN_ACCEPT &&
			    hp->rule.afilt.filtspec.netaccess != APN_RECEIVE &&
			    hp->rule.afilt.filtspec.netaccess != APN_BOTH)
				continue;
			if (pe_addrmatch_in(msg, hp) == 0)
				continue;
			break;
		default:
			continue;
		}

		/* The rule matches */
		*log = hp->rule.afilt.filtspec.log;
		*rule_id = hp->apn_id;
		DEBUG(DBG_PE_DECALF, "pe_alf_evalute_rule: decision %d",
		    hp->rule.afilt.action);
		return hp->rule.afilt.action;
	}
	return default_decision;
}

/**
 * Decode an ALF message into a printable string. The string is
 * allocated dynamically and must to be freed by the caller.
 *
 * @param msg The ALF message.
 * @return The printable version of the ALF message.
 */
static char *
pe_dump_alfmsg(struct alf_event *msg)
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
	case IPPROTO_SCTP:
		proto = "sctp";
		break;
	default:
		proto = "<unknown>";
	}

	if ((msg->op == ALF_ACCEPT) || (msg->op == ALF_RECVMSG)) {
		/* switch local and peer */
		if (asprintf(&dump,
		    "%s %s %s %s from %s port %hu to %s port %hu",
		    op, af, type, proto, peer, pport, local, lport) == -1) {
			dump = NULL;
		}
	} else {
		if (asprintf(&dump,
		    "%s %s %s %s from %s port %hu to %s port %hu",
		    op, af, type, proto, local, lport, peer, pport) == -1) {
			dump = NULL;
		}
	}

	return (dump);
}

/**
 * Match an outgoing message against an ALF filter rule. Outgoing messages
 * compare the fromhost in the filter rule with the local address of the
 * packet and the tohost with the remote address.
 *
 * @param msg The ALF message.
 * @param rule The filter rule.
 * @return True if the rule matches for the outgoing packet.
 */
static int
pe_addrmatch_out(struct alf_event *msg, struct apn_rule *rule)
{
	struct apn_host	*fromhost, *tohost;
	struct apn_port	*fromport, *toport;

	DEBUG(DBG_PE_DECALF, "pe_addrmatch_out: msg %p rule %p", msg, rule);

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

/**
 * Match an incoming message against an ALF filter rule. Incoming messages
 * compare the fromhost in the filter rule with the remote address of the
 * packet and the tohost with the local address.
 *
 * @param msg The ALF message.
 * @param rule The filter rule.
 * @return True if the rule matches for the outgoing packet.
 */
static int
pe_addrmatch_in(struct alf_event *msg, struct apn_rule *rule)
{
	struct apn_host	*fromhost, *tohost;
	struct apn_port	*fromport, *toport;

	DEBUG(DBG_PE_DECALF, "pe_addrmatch_in: msg %p rule %p", msg, rule);

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

/**
 * Compare two ipv6 addresses after applying a netmask of len bits.
 *
 * @param addr1 The first address.
 * @param addr2 The second address.
 * @param len The length of the netmaks.
 * @return True if the first len bits of both addresses are equal.
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

/**
 * Compare an apn_host from a filter rule to a socket address and return
 * true if the socket address matches the apn_host.
 *
 * @param host The apn_host from an apn rule. This can contain
 *     negates and netmasks.
 * @param addr The address of the socket.
 * @param af The address familiy of the socket address.
 * @return True in case of a match.
 */
static int
pe_addrmatch_host(struct apn_host *host, void *addr, unsigned short af)
{
	struct apn_host		*hp;
	struct sockaddr_in	*in;
	struct sockaddr_in6	*in6;
	in_addr_t		 mask;
	int			 match = 0;
	int			 negate = 0;

	DEBUG(DBG_PE_DECALF, "pe_addrmatch_host: host %p addr %p af %d", host,
	    addr, af);

	/* Empty host means "any", ie. match always. */
	if (host == NULL)
		return (1);

	hp = host;
	while (hp) {
		/*
		 * Do not consider addresses of the wrong family even
		 * in case of negate.
		 */
		if (hp->addr.af != af) {
			hp = hp->next;
			continue;
		}
		negate = hp->negate;
		match = 0;
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
		}
		if ((!negate && match) || (negate && !match))
			break;
		hp = hp->next;
	}

	DEBUG(DBG_PE_DECALF, "pe_addrmatch_host: match %d, negate %d", match,
	    negate);

	if (negate)
		return !match;
	else
		return match;
}

/**
 * Compare an apn_port from an ALF filter rule to the port in a
 * socket address and return true if the socket port matches the
 * apn_port.
 *
 * @param port The apn_port.
 * @param addr The socket address.
 * @param af The address family of the socket address.
 */
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
