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

#include <sys/types.h>
#include <sys/wait.h>

#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>


#include "apncmd.h"

static int	apncmd_addarg(struct apncmd *cmd, const char *arg);

/*
 * Create a new apncmd. Upon execution the command will be chdir'ed to
 * workdir (no chdir is done if workdir == NULL). The full path of the
 * command to be executed is fullpath, the 0th argument is cmdarg.
 * Usually cmdarg should be the final path component of fullpath,
 * e.g. fullpath = "/usr/bin/cvs" and cmdarg = "cvs".
 * The remaining arguments are command line arguments for the command.
 * The list must be terminated with a NULL pointer.
 *
 * Returns NULL on error (out of memory) or a pointer to the new
 * struct apncmd on success. The caller must call apncmd_free(...)
 * after executing the command.
 *
 * NOTE:
 * This function is declared with the "sentinel" attribute. This means
 * that gcc will warn if the last argument in the argument list is not
 * a NULL pointer (defined a value of zero with any pointer type.
 * Unfortunately, on OpenBSD NULL is defined to 0 without a cast to
 * void*. This will trigger a warning from gcc. Thus the final argument
 * of this function should always be "(void*)NULL".
 */
struct apncmd *
apncmd_create(const char *workdir, const char *fullpath,
    const char *cmdarg, ...)
{
	struct apncmd	*ret = malloc(sizeof(struct apncmd));
	va_list		 ap;
	const char	*arg;

	if (ret == NULL)
		return NULL;
	ret->pid = 0;
	memset(ret, 0, sizeof(struct apncmd));
	if (workdir) {
		ret->workdir = strdup(workdir);
		if (ret->workdir == NULL)
			goto out;
	}
	ret->fullpath = strdup(fullpath);
	if (ret->fullpath == NULL)
		goto out;
	ret->argv[0] = NULL;
	if (apncmd_addarg(ret, cmdarg) < 0)
		goto out;
	va_start(ap, cmdarg);
	while((arg = va_arg(ap, const char *))) {
		if (apncmd_addarg(ret, arg) < 0) {
			va_end(ap);
			goto out;
		}
	}
	va_end(ap);
	return ret;
out:
	apncmd_free(ret);
	return  NULL;
}

/*
 * Append the string @arg to the argument list of @cmd.
 * Returns zero on success or a negative number in case of an error.
 */
static int
apncmd_addarg(struct apncmd *cmd, const char *arg)
{
	if (cmd->argc + 1 >= APNCMD_MAXARGS)
		return -EFAULT;
	cmd->argv[cmd->argc] = strdup(arg);
	if (cmd->argv[cmd->argc] == NULL)
		return -ENOMEM;
	cmd->argc++;
	cmd->argv[cmd->argc] = NULL;
	return 0;
}

void
apncmd_free(struct apncmd *cmd)
{
	int i;
	for (i=0; i<cmd->argc; ++i) {
		if (cmd->argv[i])
			free(cmd->argv[i]);
	}
	if (cmd->fullpath)
		free(cmd->fullpath);
	if (cmd->workdir)
		free(cmd->workdir);
	free(cmd);
}

/*
 * Start the given apncmd. This will fill out the cmd->pid field.
 * The return value is zero if the fork succeeded or a negative errno
 * number if fork failed.
 */
int
apncmd_start(struct apncmd *cmd)
{
	int	fd;

	/* Cmd already running. */
	if (cmd->pid != 0)
		return -EINVAL;
	cmd->pid = fork();
	if (cmd->pid == (pid_t)-1)
		return -errno;
	if (cmd->pid > 0)
		return 0;
	/* Child */
	if (cmd->workdir) {
		if (chdir(cmd->workdir) < 0)
			exit(10);
	}
	fd = open("/dev/null", O_WRONLY);
	if (fd < 0)
		exit(11);
	close(1);
	close(2);
	if (dup(fd) != 1 || dup(fd) != 2)
		exit(12);
	close(fd);
	execvp(cmd->fullpath, cmd->argv);
	exit(13);	/* Error in exec */
}

/*
 * Start the given apncmd as a pipe. This will fill out the cmd->pid field.
 * The return value is a file descriptor that can be used to read the data
 * sent to stdout by the command. A negative return value indicates an
 * error in the setup.
 *
 * Errors in the child process cannot be detected by the return
 * value of this command. Use the exit code and apncmd_wait instead.
 */
int
apncmd_start_pipe(struct apncmd *cmd)
{
	int	p[2];

	/* Cmd alread running. */
	if (cmd->pid != 0)
		return -EINVAL;
	if (pipe(p) < 0)
		return -errno;
	cmd->pid = fork();
	if (cmd->pid == (pid_t)-1) {
		int ret = -errno;
		close(p[0]);
		close(p[1]);
		return ret;
	}
	if (cmd->pid > 0) {
		close(p[1]);
		return p[0];
	}
	/* Child */
	if (cmd->workdir) {
		if (chdir(cmd->workdir) < 0)
			exit(1);
	}
	close(1);
	close(2);
	if (dup(p[1]) != 1 || open("/dev/null", O_WRONLY) != 2)
		exit(1);
	close(p[0]);
	close(p[1]);
	execvp(cmd->fullpath, cmd->argv);
	exit(1);	/* Error in exec */
}

/*
 * Wait for the termination of an apncmd. Returns the exit status in case
 * of success or a negative errno number in case of an error.
 */
int
apncmd_wait(struct apncmd *cmd)
{
	int	status;

	if (cmd->pid == 0 || cmd->pid == (pid_t)-1)
		return -EINVAL;
	while(waitpid(cmd->pid, &status, 0) < 0) {
		if (errno != EINTR)
			return -errno;
	}
	cmd->pid = 0;
	return status;
}
