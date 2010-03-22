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

#ifndef PROTOCOL_UTILS_H
#define PROTOCOL_UTILS_H

#include <anoubis_chat.h>
#include <anoubis_protocol.h>
#include <anoubis_client.h>
#include <anoubis_sig.h>
#include <anoubis_auth.h>

__BEGIN_DECLS

/**
 * Create a new channel. If the callback function is not NULL, it is
 * used as the authentication callback for anoubis_client_create.
 *
 * @param 1st The channel for the client connection will be stored here.
 * @param 2nd The anoubis_client protocol object will be stored here.
 * @param 3rd If non-NULL this function is used as the callback funcion.
 *     The authentication is set to ANOUBIS_AUTH_TRANSPORT if this value
 *     is NULL.
 * @return None. assert(3) is used to indicate errors.
 */
extern void create_channel(struct achat_channel **, struct anoubis_client **,
    anoubis_client_auth_callback_t);

/**
 * NOTE: You really should not be using this function unless you know what
 * NOTE: you are doing!
 *
 * The same as create channel, except the the protocol version is patched
 * to at most maxversion after a successful connect. All warnings related
 * to anoubis_client_connect_old apply!
 * @param 1st The channel for the client connection will be stored here.
 * @param 2nd The anoubis_client protocol object will be stored here.
 * @param 3rd If non-NULL this function is used as the callback funcion.
 *     The authentication is set to ANOUBIS_AUTH_TRANSPORT if this value
 *     is NULL.
 * @param 4th The maximum version. A value of -1 will use the default
 *     protocol version handling.
 * @return None. assert(3) is used to indicate errors.
 */
extern void create_channel_old(struct achat_channel **,
    struct anoubis_client **, anoubis_client_auth_callback_t, int maxversion);

/**
 * Destroy a previously created channel.
 *
 * @param 1st The anoubis chat channel.
 * @param 2nd The anoubis_client object associated with the channel.
 * @return None. assert(3) is used to indicate errors.
 */
extern void destroy_channel(struct achat_channel *, struct anoubis_client *);

/**
 * Read private and public keys from ./xanoubis/default*. The private key
 * must not be protected by a password.
 *
 * @param None.
 * @return An anoubis_sig structure with the private and public key if
 *     present. NULL in case of an error.
 * NOTE: This function neither detects nor uses the home directory of the
 * NOTE: current user.
 */
extern struct anoubis_sig *create_sig(void);

__END_DECLS

#endif
