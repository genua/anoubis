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

#include <config.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>

#ifdef NEEDBSDCOMPAT
#include <bsdcompat.h>
#endif

#include "keygen.h"

static const char *countryKey		= "C";
static const char *stateKey		= "ST";
static const char *localityKey		= "L";
static const char *organizationKey	= "O";
static const char *orgunitKey		= "OU";
static const char *nameKey		= "CN";
static const char *emailKey		= "emailAddress";


/**
 * Return the default value for the country. The default value are the
 * first two capital letters in LC_CTYPE or "US" if LC_CTYPE is not set
 * or not enough capital letters could be found.
 *
 * @return A malloced string containing only the capital letters.
 */
static char *
default_country(void)
{
	char	*ctype = getenv("LC_CTYPE");
	int	 i;
	char	*result = strdup("US");

	if (!result)
		return NULL;
	if (ctype) {
		for (i=0; ctype[i]; ++i) {
			if ('A' <= ctype[i] && ctype[i] <= 'Z'
			    && 'A' <= ctype[i+1] && ctype[i+1] <= 'Z') {
				result[0] = ctype[i];
				result[1] = ctype[i+1];
			}
		}
	}
	return result;
}


/**
 * Allocate a new subject structure and fill in default values derived
 * from system settings. Defaults chosen are:
 * Country:              Derived from LC_CTYPE or US if this is not set.
 * State:                Empty (NULL)
 * Locality:             Empty (NULL)
 * Organization:         Defaults to the string "Private"
 * Organizational Unit:  Empty (NULL)
 * Name:		 Derived from the GCOS field in /etc/passwd
 * Email Address:	 The login name taken from /etc/passwd
 * @return The anoubis_keysubject structure or NULL in case of an error.
 */
struct anoubis_keysubject *
anoubis_keysubject_defaults(void)
{
	struct anoubis_keysubject	*ret;
	struct passwd			*pw;

	ret = malloc(sizeof(struct anoubis_keysubject));
	if (!ret)
		return NULL;
	ret->country = default_country();
	ret->state = NULL;
	ret->locality = NULL;
	ret->organization = strdup("Private");
	ret->orgunit = NULL;
	ret->name = NULL;
	ret->email = NULL;

	pw = getpwuid(getuid());
	if (pw) {
		ret->name = strdup(pw->pw_gecos);
		ret->email = strdup(pw->pw_name);
	}
	return ret;
}

/**
 * Create a new anoubis_keysubject structure and fill it with the information
 * contained in the first parameter.
 *
 * @param string_in A string that contains subject information in the format
 *                  used for the -subj option of openssl.
 * @return The anoubis_keysubject structure with information from the
 *         string where available. NULL in case of an error.
 */
struct anoubis_keysubject *
anoubis_keysubject_fromstring(const char *string_in)
{
	char				 *string;
	struct anoubis_keysubject	 *ret;
	char				 *p, *next;
	char				**pp;

	if (!string_in)
		return NULL;
	string = strdup(string_in);
	if (!string)
		return NULL;
	ret = malloc(sizeof(struct anoubis_keysubject));
	if (!ret)
		goto err;
	ret->country = NULL;
	ret->state = NULL;
	ret->locality = NULL;
	ret->organization = NULL;
	ret->orgunit = NULL;
	ret->name = NULL;
	ret->email = NULL;
	next = string;
	/* Skip leading bytes that do not start with a slash. */
	while (*next && *next != '/')
		next++;
	if (*next == '/') {
		*next = 0;
		next++;
	}
	while(*(p = next)) {
		char		*key, *value;
		/*
		 * @p points to the string that will be analyzed in this
		 * iteration of the loop. Forward @next to the next string
		 * and insert a NUL-Byte.
		 */
		while(*next && *next != '/')
			next++;
		if (*next == '/') {
			*next = 0;
			next++;
		}
		/*
		 * @p is a string that should be of the form key=value.
		 */
		key = p;
		while(*p && *p != '=')
			p++;
		/* No equal sign found. Skip this part of the string. */
		if (*p == 0)
			continue;
		(*p) = 0;
		value = p+1;
		pp = NULL;

#define CMPONE(FIELD)		\
		if (pp == NULL && strcasecmp(key, FIELD ## Key) == 0) \
			pp = &ret->FIELD

		CMPONE(country);
		CMPONE(state);
		CMPONE(locality);
		CMPONE(organization);
		CMPONE(orgunit);
		CMPONE(name);
		CMPONE(email);
#undef CMPONE

		/* No matching entry, skip it. */
		if (pp == NULL)
			continue;
		/* Duplicate entry: This is an error. */
		if (*pp)
			goto err;
		value = strdup(value);
		if (value == NULL)
			goto err;
		(*pp) = value;
	}

	if (string)
		free(string);
	return ret;
err:
	if (string)
		free(string);
	if (ret)
		anoubis_keysubject_destroy(ret);
	return NULL;
}

/**
 * Convert an anoubis_keysubject to a subject string that can be
 * used as an argument to the -subj option of the openssl command.
 *
 * @param subject The subject structure to convert.
 * @return Returns a malloced string with the converted subject or NULL
 *         in case of errors.
 */
char *
anoubis_keysubject_tostring(const struct anoubis_keysubject *subject)
{
	int		 len = 0;
	char		*ret;

/* Add two more bytes for the initial slash and the equal sign. */
#define LENONE(FIELD)				\
	if (subject->FIELD)			\
		len += strlen(FIELD ## Key) + strlen(subject->FIELD) + 2
	LENONE(country);
	LENONE(state);
	LENONE(locality);
	LENONE(organization);
	LENONE(orgunit);
	LENONE(name);
	LENONE(email);
#undef LENONE

	if (len == 0)
		return NULL;
	ret = malloc(len+1);
	if (!ret)
		return NULL;
	*ret = 0;
#define COPYONE(FIELD)		if (subject->FIELD) {		\
		strlcat(ret, "/", len + 1);			\
		strlcat(ret, FIELD ## Key, len + 1);		\
		strlcat(ret, "=", len + 1);			\
		strlcat(ret, subject->FIELD, len + 1);		\
	}
	COPYONE(country);
	COPYONE(state);
	COPYONE(locality);
	COPYONE(organization);
	COPYONE(orgunit);
	COPYONE(name);
	COPYONE(email);
#undef COPYONE

	return ret;
}

/**
 * Free the contents of an anobuis_keysubject structure.
 * @param subject The subject.
 * @return None.
 */
void
anoubis_keysubject_destroy(struct anoubis_keysubject *subject)
{
	if (!subject)
		return;
	if (subject->country)
		free(subject->country);
	if (subject->state)
		free(subject->state);
	if (subject->locality)
		free(subject->locality);
	if (subject->organization)
		free(subject->organization);
	if (subject->orgunit)
		free(subject->orgunit);
	if (subject->name)
		free(subject->name);
	if (subject->email)
		free(subject->email);
	free(subject);
}

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
		return -ERANGE;
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
