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

#ifndef _ANOUBIS_POLICY_H_
#define _ANOUBIS_POLICY_H_

#include <anoubischat.h>
#include <anoubis_msg.h>

struct anoubis_server;
struct anoubis_policy_comm;

typedef int (*anoubis_policy_comm_dispatcher_t)(struct anoubis_policy_comm *,
    u_int64_t token, u_int32_t uid, void * buf, size_t len, void * arg,
    int flags);

typedef int (*anoubis_process_control_dispatcher_t)(struct anoubis_msg *,
    struct achat_channel *);

/*@null@*/ /*@only@*/
struct anoubis_policy_comm * anoubis_policy_comm_create(
    anoubis_policy_comm_dispatcher_t dispatch, /*@dependent@*/ void *arg);

int anoubis_policy_comm_process(struct anoubis_policy_comm * comm,
    /*@keep@*/ struct anoubis_msg * m, u_int32_t uid,
    /*@dependent@*/ struct achat_channel * chan);

int anoubis_policy_comm_answer(struct anoubis_policy_comm * comm,
    u_int64_t token, int error, void * data, int len, int end);

void anoubis_policy_comm_abort(struct anoubis_policy_comm * comm,
    /*@notnull@*/ struct achat_channel * chan);

#endif	/* _ANOUBIS_POLICY_H_ */
