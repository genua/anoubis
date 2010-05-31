/*
 * Copyright (c) 2010 GeNUA mbH <info@genua.de>
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

#ifndef _ANOUBIS_PLAYGROUND_H_
#define _ANOUBIS_PLAYGROUND_H_

#include <sys/types.h>
#include <sys/cdefs.h>

#include <config.h>

#ifdef LINUX

__BEGIN_DECLS

/**
 * Set playground marker.
 * This will enter the playground. It will tell the kernel to mark this
 * process to be a playground process.
 * @param None.
 * @return 0 on success else negative error code.
 */
int playground_setMarker(void);

/**
 * Start new process within playground.
 * This will run given command (and its arguments) by using exec().
 * The current program is marked and replaced.
 * @note It is the duty of the caller to close all open file handles!
 * @see playground_setMarker()
 * @param[in] 1st List of strings with arguments.
 * @return error of exec() or return code of command.
 */
int playground_start_exec(char **);

/**
 * Start new process within playground.
 * This will create a new child process with fork() and run given command
 * (playground_start_exec() is used).
 * @param[in] 1st List of strings with arguments.
 * @return < 0 if an error occures or > 0 as pid of child.
 */
int playground_start_fork(char **);

__END_DECLS

#endif /* LINUX */

#endif	/* _ANOUBIS_PLAYGROUND_H_ */
