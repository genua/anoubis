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
#ifndef ANOUBIS_NOTIFY_H
#define ANOUBIS_NOTIFY_H

#include <sys/types.h>
#include <queue.h>

#include <anoubischat.h>
#include <anoubis_msg.h>

struct anoubis_notify_reg;
struct anoubis_notify_event;
struct anoubis_notify_group;

typedef u_int64_t task_cookie_t; /* XXX Use kernel header! -- ceh 02/09*/
typedef void (*anoubis_notify_callback_t)(void * caller, int verdict,
    int delegate);

struct anoubis_notify_group * anoubis_notify_create(struct achat_channel * chan,
    uid_t uid);
void anoubis_notify_destroy(struct anoubis_notify_group *);
int anoubis_notify(struct anoubis_notify_group *, task_cookie_t task,
    struct anoubis_msg * m, anoubis_notify_callback_t finish, void * data);
int anoubis_notify_register(struct anoubis_notify_group * g,
    uid_t uid, task_cookie_t task, u_int32_t ruleid, u_int32_t subsystem);
int anoubis_notify_unregister(struct anoubis_notify_group * g,
    uid_t uid, task_cookie_t task, u_int32_t ruleid, u_int32_t subsystem);
int anoubis_notify_answer(struct anoubis_notify_group * ng,
    anoubis_token_t token, int verdict, int delegate);
int anoubis_notify_end(struct anoubis_notify_group * ng,
    anoubis_token_t token, int verdict, uid_t uid, int you);

#endif
