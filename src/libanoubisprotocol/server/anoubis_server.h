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
#ifndef ANOUBIS_SERVER_H
#define ANOUBIS_SERVER_H

#include <anoubischat.h>
#include <anoubis_protocol.h>
#include <anoubis_policy.h>

struct anoubis_server;
struct anoubis_notify_group;

/*@null@*/ /*@only@*/
struct anoubis_server * anoubis_server_create(
   /*@dependent@*/ struct achat_channel * chan,
   /*@dependent@*/ struct anoubis_policy_comm *);

void anoubis_server_destroy(/*@only@*/ struct anoubis_server *);

int anoubis_server_start(struct anoubis_server *);

int anoubis_server_process(/*@dependent@*/ struct anoubis_server *, void * buf,
    size_t len);

int anoubis_server_eof(struct anoubis_server * server);

/*@exposed@*/
struct anoubis_notify_group * anoubis_server_getnotify(
    struct anoubis_server * server);

#endif
