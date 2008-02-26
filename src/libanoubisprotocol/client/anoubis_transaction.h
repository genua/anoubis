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
#ifndef ANOUBIS_TRANSACTION_H
#define ANOUBIS_TRANSACTION_H

#include <anoubis_protocol.h>
#include <anoubis_msg.h>
#include "queue.h"

#define ANOUBIS_T_INITSELF	0x0001	/* Self-initiated transaction */
#define ANOUBIS_T_INITPEER	0x0002	/* Peer-initiated transaction. */
#define ANOUBIS_T_FREECBDATA	0x0004	/* Free cbdata on destroy. */
#define	ANOUBIS_T_DESTROY	0x0008	/* Destroy after final process. */
#define ANOUBIS_T_DONE		0x0010	/* Transaction is done. */
#define	ANOUBIS_T_DEQUEUE	0x0020	/* Dequeue transaction from list */

#define ANOUBIS_T_MASK		0x003fU


struct anoubis_transaction;

typedef void (*anoubis_transaction_process_t)(struct anoubis_transaction *,
    struct anoubis_msg * m);
typedef	void (*anoubis_transaction_callback_t)(struct anoubis_transaction *);

struct anoubis_transaction {
	anoubis_token_t token;	/* Token of transaction. */
	LIST_ENTRY(anoubis_transaction) next;
	unsigned int flags;	/* Transaction flags (ANOUBIS_T_xxx). */
	int result;		/* Result of the transaction. */
	const int * opcodes;	/* List of permitted opcodes. */
	anoubis_transaction_process_t process;
	anoubis_transaction_callback_t finish;
	void * cbdata;
	int stage;		/* Stage of the transaction. Used by caller */
};

struct anoubis_transaction * anoubis_transaction_create(anoubis_token_t token,
    unsigned int flags, anoubis_transaction_process_t process,
    anoubis_transaction_callback_t finish, void * cbdata);
void anoubis_transaction_destroy(struct anoubis_transaction * t);
void anoubis_transaction_setopcodes(struct anoubis_transaction *, const int *);
int anoubis_transaction_match(struct anoubis_transaction * t,
    anoubis_token_t token, int self, int opcode);
void anoubis_transaction_process(struct anoubis_transaction * t,
    struct anoubis_msg * m);
void anoubis_transaction_progress(struct anoubis_transaction *, const int *);
void anoubis_transaction_done(struct anoubis_transaction * t, int error);

#endif
