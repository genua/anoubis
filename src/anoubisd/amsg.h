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

#ifndef _AMSG_H
#define _AMSG_H

#include <anoubis_msg.h>

#ifdef LINUX
#include <linux/eventdev.h>
#include <linux/anoubis.h>
#endif
#ifdef OPENBSD
#include <dev/eventdev.h>
#include <dev/anoubis.h>
#endif


/*
 * The MSG_BUF_SIZE must be larger than the largest message that the
 * kernel will ever send.  The mechanism is to allocate two times this size and
 * thus be always able to read an unprocessed message from the kernel, while
 * still processing a previously read message.  This is probably "overkill", as
 * the whole mechanism is to assure that we can always get a whole message from
 * the kernel in a single read. The kernel only provides messages as units, thus
 * this might not be really needed.  However, it is an additional guarantee and
 * check.
 */
#define MSG_BUF_SIZE 4096

struct msg_bug;

extern void msg_init(int, /*@dependent@*/ char *);
extern void msg_release(int);

/*@null@*/
extern anoubisd_msg_t *get_msg(int);

/*@null@*/
extern anoubisd_msg_t *get_event(int);
extern int get_client_msg(int, struct anoubis_msg **);
extern int send_msg(int, anoubisd_msg_t *);
extern int msg_pending(int);
extern int msg_eof(int);
extern void amsg_verify(struct anoubisd_msg *);

#endif /* !_AMSG_H */
