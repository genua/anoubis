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
#include <config.h>
#include <stdlib.h>

#include <anoubis_protocol.h>
#include <anoubis_msg.h>
#include <anoubis_transaction.h>

/* XXX tartler: The anoubis client library is pretty challenging to
 *              annotate properly, since I don't quite understand the
 *              memory managment implications between struct
 *              anoubis_client and struct anoubis_msg. Since for MS7 the
 *              priority is on anoubisd, we defer enabling memory checks
 *              for this part of libanoubisprotocol.
 */

/*@-memchecks@*/
struct anoubis_transaction * anoubis_transaction_create(anoubis_token_t token,
    unsigned int flags, anoubis_transaction_process_t process,
    anoubis_transaction_callback_t finish, void * cbdata)
{
	struct anoubis_transaction * ret;

	if ((flags & ~ANOUBIS_T_MASK))
		return NULL;
	if ((flags & ANOUBIS_T_INITSELF) && (flags & ANOUBIS_T_INITPEER))
		return NULL;
	if ((flags & ANOUBIS_T_WANTMESSAGE) && (flags & ANOUBIS_T_WANT_ALL))
		return NULL;
	ret = malloc(sizeof(struct anoubis_transaction));
	if (!ret)
		return NULL;
	ret->token = token;
	ret->flags = flags;
	ret->result = -1;
	ret->opcodes = NULL;
	ret->process = process;
	ret->finish = finish;
	ret->cbdata = cbdata;
	ret->stage = -1;
	ret->msg = NULL;
	return ret;
}
void anoubis_transaction_destroy(struct anoubis_transaction * t)
{
	if (t->flags & ANOUBIS_T_FREECBDATA)
		free(t->cbdata);
	if (t->flags & ANOUBIS_T_WANT_ALL) {
		while(t->msg) {
			struct anoubis_msg * m = t->msg;
			t->msg = m->next;
			anoubis_msg_free(m);
		}
	} else {
		if (t->msg)
			anoubis_msg_free(t->msg);
	}
	free(t);
}

void anoubis_transaction_setopcodes(struct anoubis_transaction * t,
    const u_int32_t * ocp)
{
	t->opcodes = ocp;
}

int anoubis_transaction_match(struct anoubis_transaction * t,
    anoubis_token_t token, int self, u_int32_t opcode)
{
	int i;
	if (!t->opcodes)
		return 0;
	if (t->token != token)
		return 0;
	if (token) {
		if ((t->flags & ANOUBIS_T_INITSELF) && !self)
			return 0;
		if ((t->flags & ANOUBIS_T_INITPEER) && self)
			return 0;
	}
	for (i=0; t->opcodes[i] > 0; ++i)
		if (t->opcodes[i] == opcode)
			return 1;
	return 0;
}

void anoubis_transaction_process(struct anoubis_transaction * t,
    struct anoubis_msg * m)
{
	if (t->process)
		t->process(t, m);
	if (t->flags & ANOUBIS_T_WANT_ALL) {
		struct anoubis_msg * tmp;
		m->next = NULL;
		tmp = t->msg;
		if (tmp) {
			while (tmp->next)
				tmp = tmp->next;
			tmp->next = m;
		} else {
			t->msg = m;
		}
	}
	if (t->flags & ANOUBIS_T_DONE) {
		if (t->flags & ANOUBIS_T_WANTMESSAGE)
			t->msg = m;
		if (t->finish)
			t->finish(t);
		if (t->flags & ANOUBIS_T_DEQUEUE)
			LIST_REMOVE(t, next);
		if (t->flags & ANOUBIS_T_DESTROY)
			anoubis_transaction_destroy(t);
	}
}

void anoubis_transaction_progress(struct anoubis_transaction * t,
    const u_int32_t * ocp)
{
	t->opcodes = ocp;
	t->stage++;
}

void anoubis_transaction_done(struct anoubis_transaction * t, int error)
{
	t->result = error;
	t->flags |= ANOUBIS_T_DONE;
	t->opcodes = NULL;
	t->stage = -1;
	t->process = NULL;
}
/*@=memchecks@*/
