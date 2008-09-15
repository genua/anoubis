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
#ifndef ANOUBIS_CLIENT_H
#define ANOUBIS_CLIENT_H

#include <anoubischat.h>
#include <anoubis_protocol.h>

struct anoubis_client;
struct anoubis_msg;
struct anoubis_transaction;

__BEGIN_DECLS

struct anoubis_msg * anoubis_client_getnotify(struct anoubis_client * client);
struct anoubis_client * anoubis_client_create(
    struct achat_channel * chan);
void anoubis_client_destroy(struct anoubis_client *);
int anoubis_client_connect(struct anoubis_client *, unsigned int);
struct anoubis_transaction * anoubis_client_connect_start(
    struct anoubis_client *, unsigned int);
void anoubis_client_close(struct anoubis_client *);
struct anoubis_transaction * anoubis_client_close_start(
    struct anoubis_client *);
struct anoubis_transaction * anoubis_client_policyrequest_start(
    struct anoubis_client *, void * data, size_t datalen);
int anoubis_client_process(struct anoubis_client *,
    struct anoubis_msg *);
struct anoubis_transaction * anoubis_client_register_start(
    struct anoubis_client * client, anoubis_token_t token, uid_t uid,
    u_int32_t rule_id, u_int32_t subsystem);
struct anoubis_transaction * anoubis_client_unregister_start(
    struct anoubis_client * client, anoubis_token_t token, uid_t uid,
    u_int32_t rule_id, u_int32_t subsystem);
struct anoubis_msg * anoubis_client_getnotify(struct anoubis_client * client);
int anoubis_client_notifyreply(struct anoubis_client * client,
    anoubis_token_t, int error, int delegate);
int anoubis_client_hasnotifies(struct anoubis_client * client);
int anoubis_client_wait(struct anoubis_client * client);
struct anoubis_transaction *anoubis_client_csumrequest_start(
    struct anoubis_client *client, int op, char *path, u_int8_t *, short);
struct anoubis_transaction *anoubis_client_sfsdisable_start(
    struct anoubis_client *client, pid_t pid);

__END_DECLS

#endif
