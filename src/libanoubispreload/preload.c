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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/param.h>
#include <sys/stat.h>

#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* Handle to libc */
static void* libc_handle = NULL;

/*
 * Handles to libc-functions.
 */
static int (*true_open)(const char *, int, ...) = NULL;
static int (*true_creat)(const char *, mode_t) = NULL;
static int (*true_close)(int) = NULL;
static int (*true_dup)(int) = NULL;
static int (*true_dup2)(int, int) = NULL;
static FILE* (*true_fopen)(const char *, const char *) = NULL;
static FILE* (*true_fdopen)(int, const char *) = NULL;
static FILE* (*true_freopen)(const char *, const char *, FILE *) = NULL;
static int (*true_fclose)(FILE *) = NULL;
static int (*true_truncate)(const char *, off_t) = NULL;
static int (*true_ftruncate)(int, off_t) = NULL;
static int (*true_rename)(const char *, const char *) = NULL;
static int (*true_unlink)(const char *) = NULL;
static int (*true_symlink)(const char *, const char *) = NULL;
static int (*true_link)(const char *, const char *) = NULL;
static pid_t (*true_fork)(void) = NULL;
static int (*true_execve)(const char *, char *const[], char *const[]) = NULL;

#ifdef _LARGE_FILES
static int (*true_open64)(const char *, int, ...) = NULL;
static int (*true_creat64)(const char *, mode_t) = NULL;
static FILE* (*true_fopen64)(const char *, const char *) = NULL;
static int (*true_ftruncate64)(int, off64_t) = NULL;
#endif

/*
 * XXX
 * Filedescriptor used for writing debug-messages.
 * Can be removed in a later state of the library.
 */
static int fd;

/* Flags shows whether library is initialized or not. */
static int initialized = 0;

/**
 * Writes a debug-message to the logfile.
 *
 * XXX This is a temporary solution and can be removed in a later state of the
 * library.
 */
static void
debug(const char *format, ...)
{
	char msg[1024];
	int num;

	va_list ap;
	va_start(ap, format);
	num = vsnprintf(msg, 1024, format, ap);
	va_end(ap);

	if (num != -1) {
		msg[num] = '\0';
		num = write(fd, msg, num);
	}
}

/**
 * Function initializes the library.
 *
 * Should be called from every wrapper to make sure, that library is really
 * initialized.
 */
static void
initialize(void)
{
	char logfile[PATH_MAX];

	if (initialized) {
		/* Already initialized */
		return;
	}

	/* Determine handle to libc */
	#ifdef RTLD_NEXT
	libc_handle = RTLD_NEXT;
	#else
	libc_handle = dlopen(PATH_LIBC, RTLD_LAZY);
	#endif

	if (libc_handle == NULL) {
		/* No handle detected */
		fprintf(stderr, "%s: %s\n", PATH_LIBC, dlerror());
		return;
	}

	true_open = dlsym(libc_handle, "open");
	true_creat = dlsym(libc_handle, "creat");
	true_close = dlsym(libc_handle, "close");
	true_dup = dlsym(libc_handle, "dup");
	true_dup2 = dlsym(libc_handle, "dup2");
	true_fopen = dlsym(libc_handle, "fopen");
	true_fdopen = dlsym(libc_handle, "fdopen");
	true_freopen = dlsym(libc_handle, "freopen");
	true_fclose = dlsym(libc_handle, "fclose");
	true_truncate = dlsym(libc_handle, "truncate");
	true_ftruncate = dlsym(libc_handle, "ftruncate");
	true_rename = dlsym(libc_handle, "rename");
	true_unlink = dlsym(libc_handle, "unlink");
	true_symlink = dlsym(libc_handle, "symlink");
	true_link = dlsym(libc_handle, "link");
	true_fork = dlsym(libc_handle, "fork");
	true_execve = dlsym(libc_handle, "execve");

	#ifdef _LARGE_FILES
	true_open64 = dlsym(libc_handle, "open64");
	true_creat64 = dlsym(libc_handle, "creat64");
	true_fopen64 = dlsym(libc_handle, "fopen64");
	true_ftruncate64 = dlsym(libc_handle, "ftruncate64");
	#endif

	/*
	 * XXX
	 * Temp. logfile. Can be removed in a later state of the library
	 */

	sprintf(logfile, "/tmp/anoubispreload-%i.log", getuid());
	fd = true_open(logfile, O_WRONLY|O_APPEND|O_CREAT,S_IRUSR|S_IWUSR);

	if (fd == -1) {
		perror(logfile);
		return;
	}

	if (fcntl(fd, F_SETFD, FD_CLOEXEC) == -1) {
		perror(logfile);
		return;
	}

	/* Library is initialized */
	initialized = 1;
}

/**
 * Tests whether a symbol is loaded.
 *
 * First, the library is initialized, if it is not already done. Second it
 * checks the symbols.
 *
 * The first argument points to one of the true_*-handles, which is checked by
 * the function. The second argument should be the name of the corresponding
 * libc-function. This name is used to print out an error message, if the
 * symbol is not loaded.
 */
static int
check_symbol(void *func, const char *func_str) {
	if (libc_handle == NULL)
		return (0);

	if (func == NULL) {
		fprintf(stderr, "Symbol \"%s\" not loaded\n", func_str);
		return (0);
	}

	return (1);
}

/**
 * Checks the given libc-function.
 *
 * Tests whether the symbol is loaded.
 */
#define CHECK_SYMBOL(func) check_symbol(true_##func, #func)

int
open(const char *pathname, int flags, ...)
{
	va_list ap;
	mode_t mode;
	int result;

	initialize();
	if (!CHECK_SYMBOL(open)) {
		errno = EINVAL;
		return (-1);
	}

	va_start(ap, flags);
	mode = va_arg(ap, mode_t);
	va_end(ap);

	result = true_open(pathname, flags, mode);
	debug("(%i) true_open(\"%s\", %i, %i) = %i\n",
	    getpid(), pathname, flags, mode, result);

	return (result);
}

int
creat(const char *pathname, mode_t mode)
{
	int result;

	initialize();
	if (!CHECK_SYMBOL(creat)) {
		errno = EINVAL;
		return (-1);
	}

	result = true_creat(pathname, mode);
	debug("(%i) true_creat(\"%s\", %i) = %i",
	    getpid(), pathname, mode, result);

	return (result);
}

int
close(int fd)
{
	int result;

	initialize();
	if (!CHECK_SYMBOL(close)) {
		errno = EINVAL;
		return (-1);
	}

	result = true_close(fd);
	debug("(%i) true_close(%i) = %i\n", getpid(), fd, result);

	return (result);
}

int
dup(int oldfd)
{
	int result;

	initialize();
	if (!CHECK_SYMBOL(dup)) {
		errno = EINVAL;
		return (-1);
	}

	result = true_dup(oldfd);
	debug("(%i) true_dup(%i) = %i", getpid(), oldfd, result);

	return (result);
}

int
dup2(int oldfd, int newfd)
{
	int result;

	initialize();
	if (!CHECK_SYMBOL(dup2)) {
		errno = EINVAL;
		return (-1);
	}

	result = true_dup2(oldfd, newfd);
	debug("(%i) true_dup2(%i, %i) = %i", getpid(), oldfd, newfd, result);

	return (result);
}

FILE *
fopen(const char *path, const char *mode)
{
	FILE *result;

	initialize();
	if (!CHECK_SYMBOL(fopen)) {
		errno = EINVAL;
		return (NULL);
	}

	result = true_fopen(path, mode);
	debug("(%i) true_fopen(\"%s\", \"%s\") = %p\n",
	    getpid(), path, mode, result);

	return (result);
}

FILE *
fdopen(int fildes, const char *mode)
{
	FILE *result;

	initialize();
	if (!CHECK_SYMBOL(fdopen)) {
		errno = EINVAL;
		return (NULL);
	}

	result = true_fdopen(fildes, mode);
	debug("(%i) true_fdopen(%i, \"%s\") = %p\n",
	    getpid(), fildes, mode, result);

	return (result);
}

FILE *
freopen(const char *path, const char *mode, FILE *stream)
{
	FILE *result;

	initialize();
	if (!CHECK_SYMBOL(freopen)) {
		errno = EINVAL;
		return (NULL);
	}

	result = true_freopen(path, mode, stream);
	debug("(%i) freopen(\"%s\", \"%s\", %p) = %p\n",
	    getpid(), path, mode, stream, result);

	return (result);
}

int
fclose(FILE *fp)
{
	int result;

	initialize();
	if (!CHECK_SYMBOL(fclose)) {
		errno = EINVAL;
		return (EOF);
	}

	debug("(%i) true_fclose(%p)\n", getpid(), fp);
	result = true_fclose(fp);

	return (result);
}

int
truncate(const char *path, off_t length)
{
	int result;

	initialize();
	if (!CHECK_SYMBOL(truncate)) {
		errno = EINVAL;
		return (-1);
	}

	result = true_truncate(path, length);
	debug("(%i) true_truncate(\"%s\", %i) = %i\n",
	    getpid(), path, length, result);

	return (result);
}

int
ftruncate(int fildes, off_t length)
{
	int result;

	initialize();
	if (!CHECK_SYMBOL(ftruncate)) {
		errno = EINVAL;
		return (-1);
	}

	result = true_ftruncate(fildes, length);
	debug("(%i) true_ftruncate(%i, %i) = %i\n",
	    getpid(), fildes, length, result);

	return (result);
}

int
rename(const char *old, const char *new)
{
	int result;

	initialize();
	if (!CHECK_SYMBOL(rename)) {
		errno = EINVAL;
		return (-1);
	}

	result = true_rename(old, new);
	debug("(%i) true_rename(\"%s\", \"%s\") = %i\n",
	    getpid(), old, new, result);

	return (result);
}

int
unlink(const char *path)
{
	int result;

	initialize();
	if (!CHECK_SYMBOL(unlink)) {
		errno = EINVAL;
		return (-1);
	}

	result = true_unlink(path);
	debug("(%i) true_unlink(\"%s\") = %i\n", getpid(), path, result);

	return (result);
}

int
symlink(const char *path1, const char *path2)
{
	int result;

	initialize();
	if (!CHECK_SYMBOL(symlink)) {
		errno = EINVAL;
		return (-1);
	}

	result = true_symlink(path1, path2);
	debug("(%i) true_symlink(\"%s\", \"%s\") = %i\n",
	    getpid(), path1, path2, result);

	return (result);
}

int
link(const char *path1, const char *path2)
{
	int result;

	initialize();
	if (!CHECK_SYMBOL(link)) {
		errno = EINVAL;
		return (-1);
	}

	result = true_link(path1, path2);
	debug("(%i) true_link(\"%s\", \"%s\") = %i\n",
	    getpid(), path1, path2, result);

	return (result);
}

pid_t
fork(void)
{
	int result;

	initialize();
	if (!CHECK_SYMBOL(fork)) {
		errno = EINVAL;
		return (-1);
	}

	result = true_fork();
	debug("(%i) true_fork() = %i\n", getpid(), result);

	return (result);
}

int
execve(const char *filename, char *const argv[], char *const envp[])
{
	int result;

	initialize();
	if (!CHECK_SYMBOL(execve)) {
		errno = EINVAL;
		return (-1);
	}

	debug("(%i) true_execve(\"%s\")...\n", getpid, filename);
	result = true_execve(filename, argv, envp);

	return (result);
}

#ifdef _LARGE_FILES
int
open64(const char *path, int oflag, ...)
{
	va_list ap;
	mode_t mode;
	int result;

	initialize();
	if (!CHECK_SYMBOL(open64)) {
		errno = EINVAL;
		return (-1);
	}

	va_start(ap, oflag);
	mode = va_arg(ap, mode_t);
	va_end(ap);

	result = true_open64(path, oflag, mode);
	debug("(%i) true_open64(\"%s\", %i, %i) = %i\n",
	    getpid(), path, oflag, mode, result);

	return (result);
}

int
creat64(const char *pathname, mode_t mode)
{
	int result;

	initialize();
	if (!CHECK_SYMBOL(creat64)) {
		errno = EINVAL;
		return (-1);
	}

	result = true_creat64(pathname, mode);
	debug("(%i) true_creat64(\"%s\", %i) = %i",
	    getpid(), pathname, mode, result);

	return (result);
}

FILE*
fopen64(const char *path, const char *mode)
{
	FILE *result;

	initialize();
	if (!CHECK_SYMBOL(fopen64)) {
		errno = EINVAL;
		return (NULL);
	}

	result = true_fopen64(path, mode);
	debug("(%i) true_fopen64(\"%s\", \"%s\") = %p\n",
	    getpid(), path, mode, result);

	return (result);
}

int
ftruncate64(int fildes, off64_t length)
{
	int result;

	initialize();
	if (!CHECK_SYMBOL(ftruncate64)) {
		errno = EINVAL;
		return (-1);
	}

	result = true_ftruncate64(fildes, length);
	debug("(%i) true_ftruncate64(%i, %i) = %i\n",
	    getpid(), fildes, length, result);

	return (result);
}
#endif /* _LARGE_FILES */
