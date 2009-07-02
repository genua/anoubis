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

/*
 * WARNING: You are not supposed to include this file unless
 * WARNING: you are pe_proc.c or a unit test thereof.
 */

#ifdef _PE_PROC_INTERNALS_H_

#define PE_PROC_FLAGS_UPGRADE		0x0001UL
#define PE_PROC_FLAGS_UPGRADE_PARENT	0x0002UL

struct pe_proc {
	TAILQ_ENTRY(pe_proc)	 entry;
	int			 refcount;
	int			 instances;
	pid_t			 pid;
	uid_t			 uid;
	uid_t			 sfsdisable_uid;
	pid_t			 sfsdisable_pid;
	unsigned int		 flags;

	struct pe_proc_ident	 ident;
	anoubis_cookie_t	 task_cookie;
	anoubis_cookie_t	 borrow_cookie;

	/* Per priority contexts */
	struct pe_context	*context[PE_PRIO_MAX];
	struct pe_context	*saved_ctx[PE_PRIO_MAX];
};

#endif	/* _PE_PROC_INTERNALS_H_ */
