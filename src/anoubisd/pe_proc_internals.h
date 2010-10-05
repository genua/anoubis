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

/* Process flags. */
#define PE_PROC_FLAGS_UPGRADE		0x0001U
#define PE_PROC_FLAGS_UPGRADE_PARENT	0x0002U
#define PE_PROC_FLAGS_HOLD		0x0004U
#define PE_PROC_FLAGS_SECUREEXEC	0x0008U

/**
 * This structure describes a single process. Its fields are private to
 * pe_proc.c and should not be accessed outside of that file. It is only
 * exported in this header file for the benefit of unit tests.
 *
 * On Linux this structure does not really track processes (or threads).
 * Instead it in fact tracks credentials structures. The total number of
 * credentials structures that share a particular task cookie are counted
 * in the field instances and pe_proc structure is only freed after all
 * instances are freed, too.
 *
 * Processes (or threads) that use that actually use one of these
 * credentials are counted in the threads field and all threads that
 * share the same credentials, i.e. they share the same task cookie are
 * assumed to be the same process for the purpose of the policy engine.
 */
struct pe_proc {
	/**
	 * Used to link all processes in a list.
	 */
	TAILQ_ENTRY(pe_proc)	 entry;

	/**
	 * The reference count of this sturcture. The structure is
	 * kept alive as long as there is at least one reference count
	 * remaining.
	 */
	int			 refcount;

	/**
	 * The total number of credential structures that use the
	 * same task cookie.
	 */
	int			 instances;

	/**
	 * The total number of actual threads that use the credentials
	 * tracked by this pe_proc structure. On OpenBSD this is always set
	 * to one as OpenBSD tracks processes directly.
	 */
	int			 threads;

	/**
	 * The process ID of the process being tracked. This is not always
	 * known and may be -1 in this case.
	 */
	pid_t			 pid;

	/**
	 * The user ID of the process.
	 */
	uid_t			 uid;

	/**
	 * The process flags. Possible values are.
	 * - PE_PROC_FLAGS_UPGRADE: The process takes part in an ongoing
	 *   upgrade.
	 * - PE_PROC_FLAGS_UPGRADE_PARENT: This is the process that started
	 *   the current upgrade.
	 * - PE_PROC_FLAGS_HOLD: Answers to event triggered by this process
	 *   should be held back and delayed until the current upgrade finishes.
	 * - PE_PROC_FLAGS_SECUREEXEC: The process die a secure exec. The
	 *   nosfs flag on a process context is only honoured if the process
	 *   did a secure exec (the policy engine enforces this if the process
	 *   does a context switch).
	 */
	unsigned int		 flags;

	/**
	 * The path/checksum of the process.
	 */
	struct pe_proc_ident	 ident;

	/**
	 * The task cookie of the current process.
	 */
	anoubis_cookie_t	 task_cookie;

	/**
	 * The playground ID of the process, zero if the process is not
	 * in a playground.
	 */
	anoubis_cookie_t	 pgid;

	/**
	 * The cookie of connection that we used to borrow our
	 * context from. The saved context will be restored as soon as the
	 * connection with this cookie closes.
	 */
	anoubis_cookie_t	 borrow_cookie[PE_PRIO_MAX];

	/**
	 * The admin and user contexts that are currently active for
	 * the process.
	 */
	struct pe_context	*context[PE_PRIO_MAX];

	/**
	 * The saved admin and user contexts. These contexts will
	 * be restored if connection to the task closes that we borrow our
	 * context from. This is NULL if the current context is not borrowed.
	 */
	struct pe_context	*saved_ctx[PE_PRIO_MAX];
};

#endif	/* _PE_PROC_INTERNALS_H_ */
