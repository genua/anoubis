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
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>


/**
 * Check if a given file exists.
 * @praram file The filename
 * @return True if the given file does not exist, i.e. lstat returns false
 *         and the error code is ENOENT.
 */
static int
noexist(const char *file)
{
	struct stat	statbuf;
	int		ret;

	ret = lstat(file, &statbuf);
	if (ret == 0)
		return -EEXIST;
	if (errno != ENOENT)
		return -errno;
	return 0;
}

/**
 * Wait for the termination of a specific child.
 * @param pid The process ID of the child process.
 * @return Zero if the child exited successfully, otherwise -ECHILD.
 */
static int
anoubis_waitfor(pid_t pid)
{
	int	status, ret;
	while(1) {
		ret = waitpid(pid, &status, 0);
		if (ret >= 0) {
			if (WIFSTOPPED(status) || WIFCONTINUED(status))
				continue;
			break;
		}
		if (errno == ECHILD)
			return -ECHILD;
	}
	if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
		return 0;
	return -ECHILD;
}

/**
 * Execute the openssl command.
 *
 * Execute the command line utility openssl with the current environment.
 * The command's arguments must be setup such that the openssl command
 * expects the passphrase from the standard input. The functions waits
 * for the termination of the command.
 * @param argv The argument vector for the command. The vector must be
 *             setup such that it expect the passphrase (if any) on stdin.
 * @param pass The passphrase for the private key. This string (if not NULL)
 *             is sent to the openssl command via stdin.
 * @param umaskadd The bits in this parameter are added the umask of the child.
 * @return Zero if the command could be executed successfully. A negative
 *         errno code in case of an error. The special return code -ECHILD
 *         indicates that the child command did not complete successfully.
 */
static int
anoubis_exec_openssl(char *argv[], const char *pass, mode_t umaskadd)
{
	int			 p[2], len = 0, ret = 0;
	pid_t			 pid;

	if (pipe(p) < 0)
		return -errno;
	if ((pid = fork()) < 0)
		return -errno;

	if (pid == 0) {		/* Child */
		/* We don't have any stdio file handles ==> use _exit */
		mode_t	nmask = umask(0777) | umaskadd;
		umask(nmask);
		close(0);
		if (dup(p[0]) != 0)
			_exit(126);
		close(p[0]);
		close(p[1]);
		execvp("openssl", argv);
		/* Only reached if execvp returns an error. */
		_exit(126);
	}
	/*
	 * Write into the pipe while we still have it open. This is ok
	 * as long as we do not exceed the pipe's buffer and avoids the
	 * potential of a SIGPIPE.
	 */
	if (pass) {
		len = strlen(pass);
		ret = write(p[1], pass, len);
	}
	close(p[0]);
	close(p[1]);
	if (pass && ret != len) {
		anoubis_waitfor(pid);
		return -ECHILD;
	}
	ret = anoubis_waitfor(pid);
	if (ret < 0) {
		return ret;
	}
	return 0;
}

/**
 * Generate a public/private key pair.
 *
 * Generate a public/private key pair and store the private key in @private
 * and the certificate in @public.
 *
 * NOTE: This function spawns child processes and waits for them. Do not
 *       ignore SIGCHLD or the exit code will be lost.
 * NOTE: Unfortunately execvp does not allow pointers to const char strings,
 *       Thus we must cast away the const from the argument. It is better
 *       to do this here than to do it in every caller. This is 100% safe
 *       because only the child ever uses the data but the const applies to
 *       the parent.
 *
 * @param private The location of the private key. The file must not exist.
 * @param public The location of the public key. The file must not exist.
 * @param pass The passphrase used to encrypt the private key.
 * @param subject The subject to be used for the certificate. This string
 *                must be in a format that is acceptable to the -subj option
 *                of openssl x509.
 * @return A negative error code in case of an error, in particular ECHILD
 *         indicates an unsuccessful termination of one of the openssl
 *         commands. Zero in case of success.
 */
int
anoubis_keygen(const char *private, const char *public, const char *pass,
    const char *subject, int bits)
{
	int			 ret, k;
	char			*argv[20];
	char			 bitstring[30];

	ret = noexist(private);
	if (ret < 0)
		return ret;
	ret = noexist(public);
	if (ret < 0)
		return ret;
	if (bits < 128 || bits > 1000000)
		return -EINVAL;
	if (pass && strlen(pass) == 0)
		pass = NULL;

	sprintf(bitstring, "%d", bits);
	k = 0;
	argv[k++] = "openssl";
	argv[k++] = "genrsa";
	argv[k++] = "-out";
	argv[k++] = (char*)private;
	if (pass) {
		argv[k++] = "-aes256";
		argv[k++] = "-passout";
		argv[k++] = "fd:0";
	}
	argv[k++] = bitstring;
	argv[k] = NULL;

	/* Make sure the private key is only readable by the user. */
	ret = anoubis_exec_openssl(argv, pass, 0077);
	if (ret < 0) {
		unlink(private);
		return ret;
	}

	k = 0;
	argv[k++] = "openssl";
	argv[k++] = "req";
	argv[k++] = "-new";
	argv[k++] = "-x509";
	argv[k++] = "-days";
	argv[k++] = "999";
	argv[k++] = "-batch";
	argv[k++] = "-subj";
	argv[k++] = (char *)subject;
	argv[k++] = "-out";
	argv[k++] = (char *)public;
	argv[k++] = "-key";
	argv[k++] = (char *)private;
	if (pass) {
		argv[k++] = "-passin";
		argv[k++] = "fd:0";
	}
	argv[k] = NULL;

	ret = anoubis_exec_openssl(argv, pass, 0);
	if (ret < 0) {
		unlink(private);
		unlink(public);
		return ret;
	}

	return 0;
}
