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

#include <config.h>

#ifdef S_SPLINT_S
#include "splint-includes.h"
#endif

#include <sys/queue.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef LINUX

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

/**
 * This structure describes a single mount point that is currently in
 * use on the system. Fields are:
 * dirfd: An open file descriptor for the root directory of the mounted
 *     file system.
 * prefix: The path prefix (mount point) where the file system is mounted.
 *     A single prefix is chosen for file systems that are mounted
 *     multiple times. If the prefix is a single slash an empty string
 *     is stored instead. This allows a user to compose file names as
 *     prefix "/" suffix even if the file system is the root file system.
 * dev: The device number of the mounted file system.
 * next: The next entry on the mount list.
 */
struct pgmount {
	int			 dirfd;
	char			*prefix;
	uint64_t		 dev;
	LIST_ENTRY(pgmount)	 next;
};

/**
 * A list of all known mount points. This list is initialized automatically
 * as needed. It is re-constructed if a mount point is not found on the
 * list.
 */
static LIST_HEAD(, pgmount)	pgmounts = LIST_HEAD_INITIALIZER(pgmounts);

/**
 * Iterate through the list of mounted devices and return the
 * pgmounts structure for the given device.
 *
 * @param dev The device to search for.
 * @return The pgmounts structure for the device or NULL if the device
 *     is not in the list.
 */
static struct pgmount *
pgmounts_find_once(uint64_t dev)
{
	struct pgmount	*mnt;

	LIST_FOREACH(mnt, &pgmounts, next)
		if (mnt->dev == dev)
			return mnt;
	return NULL;
}

/**
 * Allocate a new pgmount structure and fill it with the data from
 * the mount point given by prefix. This should be an absolute path.
 * If the path prefix is in fact the root of a mounted file system
 * the pmount structure is inserted into the global list of mounts.
 * If the device is already in the list of mounted file systems,
 * the existing entry is retained and no new entry is created.
 *
 * @param The path prefix of the mount.
 * @return None.
 */
static void
pgmounts_readone(const char *prefix)
{
	int		 fd = open(prefix, O_RDONLY | O_DIRECTORY);
	struct stat	 statbuf;
	uint64_t	 dev, ino;
	struct pgmount	*mnt = NULL;

	if (fd < 0)
		return;
	if (fstat(fd, &statbuf) < 0)
		goto err;
	dev = expand_dev(statbuf.st_dev);
	ino = statbuf.st_ino;
	if (pgmounts_find_once(dev))
		goto err;
	if (fstatat(fd, "..", &statbuf, 0) < 0)
		goto err;
	/*
	 * Parent directory is on the same mountpoint but it is a different
	 * inode => Skip this entry.
	 */
	if (expand_dev(statbuf.st_dev) == dev && statbuf.st_ino != ino)
		goto err;
	mnt = malloc(sizeof(struct pgmount));
	if (mnt == NULL)
		goto err;
	/* Avoid duplicate slashes for the root file system. */
	if (strcmp(prefix, "/") == 0)
		prefix++;
	mnt->prefix = strdup(prefix);
	if (mnt->prefix == NULL)
		goto err;
	mnt->dirfd = fd;
	mnt->dev = dev;
	LIST_INSERT_HEAD(&pgmounts, mnt, next);
	return;
err:
	close(fd);
	if (mnt)
		free(mnt);
}

/**
 * Free the list of known pgmounts and re-build the list from
 * /proc/mounts.
 *
 * @param None.
 * @return None.
 */
static void
pgmounts_read(void)
{
	FILE		*file;
	char		 line[512];
	int		 i;
	char		*prefix;

	/* First free old list of mounted file systems. */
	while (!LIST_EMPTY(&pgmounts)) {
		struct pgmount		*mnt = LIST_FIRST(&pgmounts);

		LIST_REMOVE(mnt, next);
		if (mnt->prefix)
			free(mnt->prefix);
		if (mnt->dirfd >= 0)
			close(mnt->dirfd);
		free(mnt);
	}
	file = fopen("/proc/mounts", "r");
	if (!file)
		return;
	while (fgets(line, sizeof(line), file)) {
		i=0;
		while (line[i] && !isspace(line[i]))
			i++;
		if (!isspace(line[i]))
			continue;
		i++;
		prefix = line+i;
		if (*prefix != '/')
			continue;
		while (line[i] && !isspace(line[i]))
			i++;
		line[i] = 0;
		pgmounts_readone(prefix);
	}
	fclose(file);
}

/**
 * Find the pgmount structure for the given device. This function
 * first tries to find the device in the list of mounted file systems.
 * If this failes, the list is rebuilt from /proc/mounts and the search
 * is repeated.
 *
 * @param dev The device number of the device.
 * @return The pgmount structure for the mounted device or NULL if the
 *     device is currently not mounted.
 */
static struct pgmount *
pgmounts_find(uint64_t dev)
{
	struct pgmount		*mnt = pgmounts_find_once(dev);

	if (mnt)
		return mnt;
	pgmounts_read();
	return pgmounts_find_once(dev);
}

/**
 * Convert a whiteout file name to the corresponding file name
 * that is affected by the whiteout. Both file names include the
 * ".plgr.<ID>." prefix.
 *
 * @param The whiteout file name.
 * @return The file name that is affected by the whiteout. The name
 *     is malloced. If the input file is not a whiteout, NULL is
 *     returned.
 */
static char *
pgfile_whiteout_to_file(const char *whiteout)
{
	int	 off;
	char	*ret, ch;

	if (whiteout == NULL || *whiteout == 0)
		return NULL;
	off = strlen(whiteout)-1;
	/* Find the first non-slash starting at the end of the string. */
	while (off > 0 && whiteout[off] == '/')
		off--;
	/* Find the next slash starting at the current offset. */
	while (off > 0 && whiteout[off] != '/')
		off--;
	/*
	 * Verify that we have a real path component and skip the leading
	 * slash if present.
	 */
	if (whiteout[off] == '/')
		off++;
	if (whiteout[off] == '/' || whiteout[off] == 0)
		return NULL;
	/* Check if the file name really is a whiteout. */
	if (sscanf(whiteout+off, ".plgrD%*x%c", &ch) != 1 || ch != '.')
		return NULL;
	/* Copy the whiteout string and replace the 'D' with a dot ('.') */
	ret = strdup(whiteout);
	if (ret)
		ret[off + 5] = '.';
	return ret;
}

/**
 * Validate a single name in a name list. The name must start with
 * a slash (followed by something that is not a slash) and it must
 * reference the given inode on the given device.
 *
 * @param dev The device number of the file.
 * @param ino The inode number of the file.
 * @param dirfd The directory where the device is mounted. The name
 *     is interpreted relative to this directory.
 * @param name The name of the file relative to the directory dirfd.
 * @return The number of links to the file as returned by stat. This
 *     value is zero if the name is invalid or if the name references
 *     a different file.
 */
static int
pgfile_validate_one_name(uint64_t dev, uint64_t ino, int dirfd,
    const char *name)
{
	const char		*p = name;
	char			*q;
	struct stat		 statbuf;
	int			 ret;

	if (*name != '/')
		return 0;
	while (*p && *p == '/')
		p++;
	if (*p == 0)
		return 0;
	if (fstatat(dirfd, p, &statbuf, AT_SYMLINK_NOFOLLOW) < 0)
		return 0;
	if (expand_dev(statbuf.st_dev) != dev || statbuf.st_ino != ino)
		return 0;
	ret = statbuf.st_nlink;

	/*
	 * Now check if this is a whiteout marker and suppress it, if
	 * the corresponding file exists.
	 */
	q = pgfile_whiteout_to_file(p);
	if (q) {
		if (fstatat(dirfd, q, &statbuf, AT_SYMLINK_NOFOLLOW) == 0)
			ret = 0;
		free(q);
	}
	return ret;
}

/**
 * Check if the list of links to a file is complete.
 * This function goes through all the names in the list of names an
 * checks if the name references the file given by the dev/inode pair.
 * It returns zero if all links are found and a negative error code
 * in case of errors.
 *
 * @param dev The device where the file resides.
 * @param ino The inode number of the file.
 * @param names a list of path names relative to the root of the given
 *     device. These name are searched for links the file with the given
 *     inode number.
 * @return Zero if all links to the file could be found. A negative
 *     error code if something went wrong. In particular:
 *     EXDEV: The given device is not mounted.
 *     ENOENT: None of the file names references the given inode.
 *     EBUSY: Some but not all links to the file were found in the
 *         list of file names.
 */
int
pgfile_check(uint64_t dev, uint64_t ino, const char *names[])
{
	struct pgmount		*mnt = pgmounts_find(dev);
	int			 totallinks = 0, foundlinks = 0;
	int			 i;

	/* Device is currently not mounted. */
	if (!mnt)
		return -EXDEV;
	for (i=0; names[i]; ++i) {
		int		nlinks;

		nlinks = pgfile_validate_one_name(dev, ino, mnt->dirfd,
		    names[i]);
		if (nlinks == 0)
			continue;
		totallinks = nlinks;
		foundlinks++;
	}
	if (totallinks == 0)
		return -ENOENT;
	if (totallinks > foundlinks)
		return -EBUSY;
	return 0;
}

/**
 * This file turns all references to a particular playground file into
 * appropriate production system files. This means that real playground
 * files are renamed and whiteout playground files cause the original file
 * (and the whiteout) to be deleted.
 *
 * @param dev The device number of the file.
 * @param ino The inode number of the playground file.
 * @names A list of all names (relative to the root of the device)
 *     that might reference the file. Only names that actually reference
 *     the correct file (as identified by device and inode number) are
 *     processed. All other names are skipped.
 * @return Zero if all links to the file were found in the file list and
 *     could be renamed. Negative return values indicate an error. In
 *     particular:
 *     EXDEV: The device with the given  device number is not mounted.
 *     ENOENT: None of the file names reference a file with the given
 *         inode number.
 *     EBUSY: The file was found but one or more links to the file could
 *         not be renamed/deleted. Possible cause are permission errors
 *         or missing names in the name list.
 */
int
pgfile_process(uint64_t dev, uint64_t ino, const char *names[])
{
	struct pgmount		*mnt = pgmounts_find(dev);
	int			 totallinks = 0, renamedok = 0;
	int			 i;

	if (!mnt)
		return -EXDEV;
	for (i=0; names[i]; ++i) {
		int	 dir2, nlink;
		int	 off;
		char	*p2, *dir, *oldname, *newname, ch1, ch2;

		nlink = pgfile_validate_one_name(dev, ino, mnt->dirfd,
		    names[i]);
		if (nlink == 0)
			continue;
		totallinks = nlink;

		/*
		 * If pgfile_validate_one_name returns non-zero, the path name
		 * contains a leading slash and at least one character that
		 * is neither NUL nor a slash after the first slash. This
		 * avoid a lot of useless error handling code below.
		 */
		p2 = strdup(names[i]);
		if (!p2)
			continue;
		dir = p2;
		while (*dir == '/')
			dir++;
		/* The loop handles trailing slashes. */
		do {
			oldname = strrchr(p2, '/');
			oldname[0] = 0;
		} while (oldname[1] == 0);
		oldname++;
		if (dir == oldname) {
			dir2 = dup(mnt->dirfd);
		} else {
			dir2 = openat(mnt->dirfd, dir, O_RDONLY | O_DIRECTORY);
		}
		if (dir2 < 0) {
			free(p2);
			continue;
		}
		/*
		 * File names that do not allow a rename have probably been
		 * renamed earlier. Count them as succesfully renamed
		 * instances of the file.
		 */
		renamedok++;
		if (sscanf(oldname, ".plgr%c%*x%c%n", &ch1, &ch2, &off) < 2)
			goto next;
		if (ch2 != '.' || (ch1 != '.' && ch1 != 'D'))
			goto next;
		newname = oldname + off;
		/*
		 * The file is a whiteout. First remove the original file
		 * and if this succeeds remove the whiteout, too.
		 */
		if (ch1 == 'D') {
			int		ret;

			ret = unlinkat(dir2, newname, 0);
			if (ret < 0 && errno == EISDIR)
				ret = unlinkat(dir2, newname, AT_REMOVEDIR);
			if ((ret != 0 && errno != ENOENT)
			    || unlinkat(dir2, oldname, 0) != 0)
				renamedok--;
			goto next;
		}
		if (renameat(dir2, oldname, dir2, newname) < 0)
			renamedok--;
		/* Remove a whiteout if it exists. */
		oldname[5] = 'D';
		unlinkat(dir2, oldname, 0);
next:
		close(dir2);
		free(p2);
	}
	if (totallinks == 0)
		return -ENOENT;
	if (renamedok < totallinks)
		return -EBUSY;
	return 0;
}

/**
 * This function constructs an absolute path for the file given by
 * the device/inode pair. This is done be appending the given name to
 * the mount point of the file system. This function makes sure that
 * the resulting path name in fact points to a file with the same inode
 * number on the correct device. Memory for the return value is allocated
 * by this function an must be freed by the caller.
 *
 * @param pathp The path name of the file is malloced and a pointer to the
 *     malloced path name is returned in this location.
 * @param dev The device number of the device.
 * @param ino The inode number of the file.
 * @param The path of the file relative to the root of the file system
 *     on the given device.
 * @return Zero if the path name was found, a negative error code if the
 *     file was not found. In particular:
 *     -EXDEV: The given device is currently not mounted.
 *     -EBUSY: The file name does not exist or references a different file.
 *     -ENOMEM: Out of memory.
 */
int
pgfile_composename(char **pathp, uint64_t dev, uint64_t ino, const char *name)
{
	struct pgmount		*mnt = pgmounts_find(dev);

	if (!mnt)
		return -EXDEV;
	if (!pgfile_validate_one_name(dev, ino, mnt->dirfd, name))
		return -EBUSY;
	if (asprintf(pathp, "%s%s", mnt->prefix, name) < 0)
		return -ENOMEM;
	return 0;
}

#endif
