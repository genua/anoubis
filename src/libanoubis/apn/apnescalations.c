/*
 * Copyright (c) 2009 GeNUA mbH <info@genua.de>
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

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "apn.h"
#include "apninternals.h"

#include <sys/queue.h>

#ifdef LINUX
#include "linux/anoubis.h"
#include "linux/anoubis_alf.h"
#endif

#ifdef OPENBSD
#include "dev/anoubis.h"
#include "sys/anoubis_alf.h"
#endif

#include <anoubis_protocol.h>

/**
 * Apply a scope to all rules in a chain. Additionally retrict the
 * applicability of all rules until a particular timeout in seconds since
 * the epoch and to a particular task.
 * The timeout and/or task can be 0 to match all rules.
 * 
 * @param chain The chain.
 * @param old The scope
 * @param timeout The timeout
 * @param task The task which should be matched.
 *
 * @return zero on success or a negative errno code on failure.
 */
int
apn_escalation_addscope(struct apn_chain *chain, struct apn_scope *old,
    time_t timeout, anoubis_cookie_t task)
{
	struct apn_rule		*rule;

	if (old) {
		if (old->timeout && (!timeout || old->timeout < timeout))
			timeout = old->timeout;
		if (task && old->task && task != old->task)
			return -EINVAL;
		if (old->task)
			task = old->task;
	}
	if (!task && !timeout)
		return 0;
	TAILQ_FOREACH(rule, chain, entry) {
		if (!rule->scope) {
			rule->scope = malloc(sizeof(struct apn_scope));
			if (!rule->scope)
				return -ENOMEM;
			rule->scope->task = task;
			rule->scope->timeout = timeout;
		} else {
			if (task && rule->scope->task
			    && task != rule->scope->task)
				return -EINVAL;
			if (task)
				rule->scope->task = task;
			if (timeout) {
				if (!rule->scope->timeout)
					rule->scope->timeout = timeout;
				else if (timeout < rule->scope->timeout)
					rule->scope->timeout = timeout;
			}
		}
	}
	return 0;
}

/**
 * Insert the rules in @chain into the rule chain that @anchor is part
 * of. This makes sure that rules in @chain are considered before the
 * rule @anchor. In particular this means:
 *   - If @anchor is not the last rule its chain and it is a default rule
 *     all default rules in @chain are inserted immediately before @anchor,
 *     all other rules in @chain are appended that the end.
 *   - If @anchor is not a default rule all rules in @chain are inserted
 *     before @anchor. Any default rules in @chain cause an error in this
 *     case.
 * @anchor must be part of an application block. If that application block
 * is already part of some rule set, this rule set must be given in @rs.
 * Otherwise @rs must be NULL.
 * The rule-IDs of all rules in @chain are set to 0. If @rs is not NULL
 * this mean that new rule IDs will be assigned to all rules.
 * 
 * @param rs The ruleset
 * @param anchor The anchor
 * @param chain The rule chain
 *
 * @return 0 or a negative errno
 */
int
apn_escalation_splice(struct apn_ruleset *rs, struct apn_rule *anchor,
    struct apn_chain *chain)
{
	int			 append = 0;
	struct apn_rule		*rule;

	switch (anchor->apn_type) {
	case APN_DEFAULT:
	case APN_SFS_DEFAULT:
		/*
		 * @anchor is a default rule. We must append non-default
		 * rules in chain at the end unless @anchor is the last
		 * rule in its own chain.
		 */
		if (TAILQ_NEXT(anchor, entry))
			append = 1;
		break;
	default:
		/*
		 * @anchor is not a default rule. Any default rules in
		 * @chain are an error because these will always be considered
		 * after @anchor.
		 */
		TAILQ_FOREACH(rule, chain, entry) {
			if (rule->apn_type == APN_DEFAULT
			    || rule->apn_type == APN_SFS_DEFAULT)
				return -EINVAL;
		}
		break;
	}
	while (!TAILQ_EMPTY(chain)) {
		rule = TAILQ_FIRST(chain);
		TAILQ_REMOVE(chain, rule, entry);
		switch (rule->apn_type) {
		case APN_DEFAULT:
		case APN_SFS_DEFAULT:
			TAILQ_INSERT_BEFORE(anchor, rule, entry);
			break;
		default:
			if (append) {
				TAILQ_INSERT_TAIL(anchor->pchain, rule, entry);
			} else {
				TAILQ_INSERT_BEFORE(anchor, rule, entry);
			}
			break;
		}
		rule->pchain = anchor->pchain;
		if (rs)
			apn_assign_id(rs, &rule->_rbentry, rule);
		else
			rule->apn_id = 0;
	}
	return 0;
}

/**
 * Create an otherwise empty rule set of a particular type.
 *
 * @param type The ruleset type
 * @return an initialized ruleset or NULL on error.
 */
static struct apn_rule *
empty_rule(int type)
{
	struct apn_rule		*rule;

	rule = malloc(sizeof(struct apn_rule));
	if (!rule)
		return NULL;
	rule->apn_type = type;
	rule->apn_id = 0;
	rule->userdata = NULL;
	rule->scope = NULL;
	rule->app = NULL;
	rule->pchain = NULL;
	rule->flags = 0;
	switch(type) {
	case APN_ALF:
	case APN_SFS:
	case APN_SB:
	case APN_CTX:
		TAILQ_INIT(&rule->rule.chain);
		break;
	case APN_DEFAULT:
	case APN_ALF_FILTER:
	case APN_ALF_CAPABILITY:
	case APN_CTX_RULE:
	case APN_SFS_ACCESS:
	case APN_SFS_DEFAULT:
	case APN_SB_ACCESS:
		memset(&rule->rule, 0, sizeof(rule->rule));
		break;
	default:
		free(rule);
		return NULL;
	}
	return rule;
}

/**
 * Create an ALF filter rule with the given parameters.
 * 
 * @param action The action and log level for the new rule
 * @param access The network access (connect, accept, send, receive)
 * @param peer Create a rule that only matches this peer. A copy of the
 *     structure pointed to by @peer is created.
 * @param port The port that the new rule applies to. For all operations except
 *     accept this is the port on the remote host. Use 0 if the rule should
 *     apply to all scopes.
 * @return The rule or NULL in case of an error.
 */
static struct apn_rule *
alf_filter_rule(struct apn_default *action, int access,
    struct apn_host *peer, int port)
{
	struct apn_rule		*rule;
	struct apn_afiltrule	*afilt;
	struct apn_host		*peercp = NULL;
	struct apn_port		*apnport = NULL;
	int			 proto;

	switch(access) {
	case APN_CONNECT:
	case APN_ACCEPT:
		proto = IPPROTO_TCP;
		break;
	case APN_SEND:
	case APN_RECEIVE:
		proto = IPPROTO_UDP;
		break;
	default:
		return NULL;
	}
	rule = empty_rule(APN_ALF_FILTER);
	if (!rule)
		return NULL;
	afilt = &rule->rule.afilt;
	afilt->action = action->action;
	afilt->filtspec.log = action->log;
	afilt->filtspec.proto = proto;
	afilt->filtspec.netaccess = access;
	afilt->filtspec.fromhost = NULL;
	afilt->filtspec.fromport = NULL;
	afilt->filtspec.tohost = NULL;
	afilt->filtspec.toport = NULL;
	if (peer) {
		peercp = apn_copy_hosts(peer);
		if (!peercp)
			goto err;
	}
	if (port) {
		apnport = malloc(sizeof(struct apn_port));
		if (!apnport)
			goto err;
		apnport->port = htons(port);
		apnport->port2 = 0;
		apnport->next = NULL;
	}
	switch (access) {
	case APN_SEND:
	case APN_CONNECT:
		afilt->filtspec.tohost = peercp;
		afilt->filtspec.toport = apnport;
		break;
	case APN_RECEIVE:
		afilt->filtspec.fromhost = peercp;
		afilt->filtspec.fromport = apnport;
		break;
	case APN_ACCEPT:
		afilt->filtspec.fromhost = peercp;
		afilt->filtspec.toport = apnport;
		break;
	default:
		goto err;
	}
	return rule;
err:
	if (peercp)
		apn_free_host(peercp);
	if (apnport)
		apn_free_port(apnport);
	apn_free_one_rule(rule, NULL);
	return NULL;
}

/**
 * Create an ALF capability rule with the given parameters.
 * @param action The action and log priority for the rule
 * @param cap The capability.
 * @return The rule or NULL in case of an error.
 */
static struct apn_rule *
alf_cap_rule(struct apn_default *action, int cap)
{
	struct apn_rule		*rule;

	rule = empty_rule(APN_ALF_CAPABILITY);
	if (!rule)
		return NULL;
	rule->rule.acap.action = action->action;
	rule->rule.acap.log = action->log;
	rule->rule.acap.capability = cap;
	return rule;
}

/**
 * Create an apn_host structure based on a sockaddr struct.
 * @param addr the sockaddr struct
 * @return The generated apn_host or NULL on error.
 */
static struct apn_host *
apn_kerneladdr_to_host(struct sockaddr *addr)
{
	struct apn_host	*ret;

	ret = malloc(sizeof(struct apn_host));
	if (!ret)
		return NULL;
	ret->next = NULL;
	ret->negate = 0;

	switch(addr->sa_family) {
	case AF_INET:
		ret->addr.af = AF_INET;
		ret->addr.len = 32;
		ret->addr.apa.v4 = ((struct sockaddr_in *)addr)->sin_addr;
		break;
	case AF_INET6:
		ret->addr.af = AF_INET6;
		ret->addr.len = 128;
		ret->addr.apa.v6 = ((struct sockaddr_in6 *)addr)->sin6_addr;
		break;
	default:
		goto err;
	}
	return ret;
err:
	free(ret);
	return NULL;
}

/**
 * Return the port number of a sockaddr struct.
 * @param addr the sockaddr struct
 * @return The port in host byteorder or 0 when the socket's 
 *	adress family is not supported.
 */
static int
apn_extract_port(struct sockaddr *addr)
{
	switch(addr->sa_family) {
	case AF_INET:
		return ntohs(((struct sockaddr_in *)addr)->sin_port);
	case AF_INET6:
		return ntohs(((struct sockaddr_in6 *)addr)->sin6_port);
	}
	return 0;
}

/**
 * Create a dynamic rule based on an ALF event, the default action and 
 * flags supplied by the user.
 * @param dst The apn rule chain
 * @param event the ALF event
 * @param action The default action
 * @param flags 
 * @return 0 or a negative ernno when errors are encoutered.
 */
int
apn_escalation_rule_alf(struct apn_chain *dst, const struct alf_event *event,
    struct apn_default *action, unsigned long flags)
{
	struct apn_rule		*rule, *first = NULL;
	int			 port = 0;
	struct apn_host		*peer = NULL;
	int			 tcpconnect = 0, tcpaccept = 0;
	int			 udpsend = 0, udpreceive = 0;
	int			 doit[4];
	int			 i, k;

	if (event->protocol == IPPROTO_TCP) {
		if (event->op == ALF_CONNECT) {
			tcpconnect = 1;
		} else if (event->op == ALF_ACCEPT) {
			tcpaccept = 1;
		} else {
			return -EINVAL;
		}
	} else if (event->protocol == IPPROTO_UDP) {
		if (event->op == ALF_SENDMSG) {
			udpsend = 1;
		} else if (event->op == ALF_RECVMSG) {
			udpreceive = 1;
		} else {
			return -EINVAL;
		}
	} else if (event->protocol == IPPROTO_ICMP) {
		if (flags)
			return -EINVAL;
		rule = alf_cap_rule(action, APN_ALF_CAPRAW);
		TAILQ_INSERT_TAIL(dst, rule, entry);
		rule->pchain = dst;
		return 0;
	} else {
		return -EINVAL;
	}
	if ((flags & ALF_EV_ALLPORT) == 0) {
		switch(event->op) {
		case ALF_ACCEPT:
			port = apn_extract_port(
			    (struct sockaddr *)&event->local);
			break;
		case ALF_CONNECT:
		case ALF_SENDMSG:
		case ALF_RECVMSG:
			port = apn_extract_port(
			    (struct sockaddr *)&event->peer);
			break;
		}
	} else {
		flags &= ~ALF_EV_ALLPORT;
	}
	if ((flags & ALF_EV_ALLPEER) == 0) {
		peer = apn_kerneladdr_to_host((struct sockaddr *)&event->peer);
		if (!peer)
			goto err;
	} else {
		flags &= ~ALF_EV_ALLPEER;
	}
	if (flags & ALF_EV_ALLDIR) {
		if (tcpconnect || tcpaccept)
			tcpconnect = tcpaccept = 1;
		if (udpsend || udpreceive)
			udpsend = udpreceive = 1;
		flags &= ~ALF_EV_ALLDIR;
	}
	if (flags & ALF_EV_ALLPROTO) {
		if (tcpconnect || udpsend)
			tcpconnect = udpsend = 1;
		if (tcpaccept || udpreceive)
			tcpaccept = udpreceive = 1;
		flags &= ~ALF_EV_ALLPROTO;
	}
	if (flags)
		goto err;
	k = 0;
	if (tcpconnect)
		doit[k++] = APN_CONNECT;
	if (tcpaccept)
		doit[k++] = APN_ACCEPT;
	if (udpsend)
		doit[k++] = APN_SEND;
	if (udpreceive)
		doit[k++] = APN_RECEIVE;
	for (i=0; i<k; ++i) {
		rule = alf_filter_rule(action, doit[i], peer, port);
		if (!rule)
			goto err;
		TAILQ_INSERT_TAIL(dst, rule, entry);
		rule->pchain = dst;
		if (!first)
			first = rule;
	}
	if (peer)
		apn_free_host(peer);
	return 0;
err:
	while (first) {
		rule = TAILQ_NEXT(first, entry);
		TAILQ_REMOVE(dst, first, entry);
		apn_free_one_rule(first, NULL);
		first = rule;
	}
	if (peer)
		apn_free_host(peer);
	return -ENOMEM;
}

/**
 * Create a dynamic rule based on a SB event, the rule which triggered the 
 * event, the default action and the pathname. 
 * @param dst The apn rule chain
 * @param trigger The rule which triggered the event.
 * @param action The default action
 * @param prefix The affected path
 * @param mask The optional read/write/execute flags.
 * @return 0 or a negative ernno when errors are encoutered.
 */
int
apn_escalation_rule_sb(struct apn_chain *dst, struct apn_rule *trigger,
    struct apn_default *action, const char *prefix, unsigned long mask)
{
	struct apn_rule		*rule;

	if (!prefix || prefix[0] != '/')
		return -EINVAL;
	if ((mask & APN_SBA_ALL) != mask)
		return -EINVAL;
	if (trigger->apn_type == APN_DEFAULT) {
		rule = empty_rule(APN_SB_ACCESS);
		if (!rule)
			return -ENOMEM;
		rule->rule.sbaccess.cs.type = APN_CS_NONE;
	} else if (trigger->apn_type == APN_SB_ACCESS) {
		rule = apn_copy_one_rule(trigger);
		if (!rule)
			return -ENOMEM;
		if (rule->rule.sbaccess.path) {
			free(rule->rule.sbaccess.path);
			rule->rule.sbaccess.path = NULL;
		}
	} else {
		return -EINVAL;
	}
	rule->rule.sbaccess.amask = mask;
	rule->rule.sbaccess.action = action->action;
	rule->rule.sbaccess.log = action->log;
	rule->rule.sbaccess.path = strdup(prefix);
	if (!rule->rule.sbaccess.path) {
		apn_free_one_rule(rule, NULL);
		return -ENOMEM;
	}
	rule->apn_id = 0;
	TAILQ_INSERT_TAIL(dst, rule, entry);
	return 0;
}

/**
 * Create a dynamic rule based on a SFS event, the rule which triggered the 
 * event, the default action and the pathname. 
 * @param dst The apn rule chain
 * @param trigger The rule which triggered the event.
 * @param action The default action
 * @param prefix The affected path
 * @param sfsmatch The type of SFS match generated.
 * @return 0 or a negative ernno when errors are encoutered.
 */
int
apn_escalation_rule_sfs(struct apn_chain *dst, struct apn_rule *trigger,
    struct apn_default *action, const char *prefix, int sfsmatch)
{
	struct apn_rule		*rule;

	if (!prefix || prefix[0] != '/')
		return -EINVAL;
	switch(sfsmatch) {
	case ANOUBIS_SFS_DEFAULT:
		if (trigger->apn_type != APN_SFS_DEFAULT)
			return -1;
		break;
	case ANOUBIS_SFS_VALID:
	case ANOUBIS_SFS_INVALID:
	case ANOUBIS_SFS_UNKNOWN:
		if (trigger->apn_type != APN_SFS_ACCESS)
			return -EINVAL;
		break;
	default:
		return -EINVAL;
	}
	rule = apn_copy_one_rule(trigger);
	if (!rule)
		return -ENOMEM;
	rule->apn_id = 0;
	if (trigger->apn_type == APN_SFS_ACCESS) {
		int			 i;
		struct apn_default	*actions[3];
		static const int	 sfsmatches[3] = {
			ANOUBIS_SFS_VALID,
			ANOUBIS_SFS_INVALID,
			ANOUBIS_SFS_UNKNOWN
		};

		actions[0] = &rule->rule.sfsaccess.valid;
		actions[1] = &rule->rule.sfsaccess.invalid;
		actions[2] = &rule->rule.sfsaccess.unknown;

		rule->rule.sfsaccess.path = strdup(prefix);
		if (rule->rule.sfsaccess.path == NULL) {
			apn_free_one_rule(rule, NULL);
			return -ENOMEM;
		}
		for (i=0; i<3; ++i) {
			if (sfsmatch == sfsmatches[i]) {
				actions[i]->action = action->action;
				actions[i]->log = action->log;
			} else {
				actions[i]->action = APN_ACTION_CONTINUE;
				actions[i]->log = APN_LOG_NONE;
			}
		}
	} else if (trigger->apn_type == APN_SFS_DEFAULT) {
		rule->rule.sfsdefault.path = strdup(prefix);
		if(rule->rule.sfsdefault.path == NULL) {
			apn_free_one_rule(rule, NULL);
			return -ENOMEM;
		}
		rule->rule.sfsdefault.action = action->action;
		rule->rule.sfsdefault.log = action->log;
	}
	TAILQ_INSERT_TAIL(dst, rule, entry);
	return 0;
}
