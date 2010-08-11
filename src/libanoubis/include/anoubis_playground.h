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
#include <stdint.h>

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


/* See playground_files.c for documentation of these functions. */
int	 pgfile_check(uint64_t dev, uint64_t ino, const char *names[]);
int	 pgfile_process(uint64_t dev, uint64_t ino, const char *names[]);
int	 pgfile_composename(char **, uint64_t dev, uint64_t ino, const char *);
void	 pgfile_normalize_file(char *);

/**
 * Convert a device number as returned by stat into a device number as used
 * inside the kernel and inside anoubis events. See include/linux/kdev_t.h
 * for the source code. The core facts are:
 * Kernel uses
 *   - low 20 Bits for the minor number
 *   - the rest is used for the major number.
 * User space uses
 * - low 8 Bits for the low 8 bits of the minor number
 * - next 12 Bits are used for the major number
 * - next 12 Bits are used for higher 12 Bits of the minor number.
 * The reason for this encoding are historical. Note that legacy code that
 * assumes a 16-Bit dev_t with 8 Bit major and 8 Bit minor number will work
 * correctly with this encoding (provided that the major and minor numbers
 * fit into 8 bits).
 *
 * @param dev A dev_t type from a stat structures st_dev field.
 * @return A kernel encoded device number.
 */
static inline uint64_t
expand_dev(dev_t dev)
{
	uint64_t	major = (dev & 0xfff00) >> 8;
	uint64_t	minor = (dev & 0xff) | ((dev >> 12) & 0xfff00);

	return (major << 20) | minor;
}

__END_DECLS

#endif /* LINUX */

#endif	/* _ANOUBIS_PLAYGROUND_H_ */
