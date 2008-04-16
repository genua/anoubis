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

#include "config.h"

#ifndef NEEDBSDCOMPAT
#include <sys/queue.h>
#else
#include <queue.h>
#endif

#include <sys/types.h>

#include <anoubischat.h>
#include <anoubis_msg.h>

struct anoubis_notify_group;
struct anoubis_notify_head;

typedef void (*anoubis_notify_callback_t)(struct anoubis_notify_head * head,
    int verdict, void * cbdata);

struct anoubis_notify_group * anoubis_notify_create(struct achat_channel * chan,
    uid_t uid);
void anoubis_notify_destroy(struct anoubis_notify_group *);
struct anoubis_notify_head * anoubis_notify_create_head(
    struct anoubis_msg * m, anoubis_notify_callback_t finish, void * cbdata);
void anoubis_notify_destroy_head(struct anoubis_notify_head * h);
int anoubis_notify_register(struct anoubis_notify_group * g,
    uid_t uid, u_int32_t ruleid, u_int32_t subsystem);
int anoubis_notify_unregister(struct anoubis_notify_group * g,
    uid_t uid, u_int32_t ruleid, u_int32_t subsystem);
int anoubis_notify(struct anoubis_notify_group *, struct anoubis_notify_head *);
int anoubis_notify_sendreply(struct anoubis_notify_head * head,
    int verdict, void * you, uid_t uid);
int anoubis_notify_answer(struct anoubis_notify_group * ng,
    anoubis_token_t token, int verdict, int delegate);

#endif
