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

#include "config.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>

#include <sys/queue.h>

#include <anoubisd.h>
#include <sfs.h>
#include <anoubis_protocol.h>
#include <anoubis_alloc.h>

#define CSTYPE_NEGATIVE		0x1000
#define CSTYPE_MASK		0xfff
#define CSTYPE_UID		1
#define CSTYPE_KEY		2


/**
 * This structure represents a single entry in the sfs checksum cache.
 * It contains a checksum and the path and uid/key.
 */
struct sfshash_entry {
	TAILQ_ENTRY(sfshash_entry)	hash_link;
	TAILQ_ENTRY(sfshash_entry)	lru_link;
	int				cstype;
	int				slot;
	union {
		char			*key;
		uid_t			 uid;
	} u;
	char				*path;
	struct abuf_buffer		 csum;
};

#define SFSHASH_SHIFT		 (13)
#define SFSHASH_NRENTRY		 (1<<13)
#define SFSHASH_MASK		 (SFSHASH_NRENTRY-1)
#define SFSHASH_MAX		 (4*SFSHASH_NRENTRY)

TAILQ_HEAD(, sfshash_entry)	sfshash_tab[SFSHASH_NRENTRY];
TAILQ_HEAD(, sfshash_entry)	sfshash_lru;
int				sfshash_entries;

/**
 * Initialize the sfshash. This must be called at program startup
 * before the hash is used for the first time. This hash itself
 * cannot be destroyed.
 */
void
sfshash_init(void)
{
	int i;

	TAILQ_INIT(&sfshash_lru);
	for (i=0; i<SFSHASH_NRENTRY; ++i)
		TAILQ_INIT(&sfshash_tab[i]);
	sfshash_entries = 0;
}

/**
 * This is the hash function that is used to convert a path and a
 * Key-ID and User-ID to a hash value. The caller should either specify
 * the user-ID as (uid_t)-1 or the Key-ID as NULL.
 *
 * @param path The path name of the file.
 * @param key The Key-ID of the key that stored this checksum. This value
 *     can be NULL. If it is not NULL it should be a string of printable hex
 *     digits. These are treated case insensitive.
 * @param uid The user-ID of the user.
 * @return A value between zero and SFSHASH_MASK (inclusive). If the
 *     function is called with the same parameters, it will return the
 *     same value.
 *
 * @note As of today not only the key-ID but also the path name is hashed
 *       case insensitive. This might be better if a file system does not
 *       use case sensitive path names and will probably not lead to
 *       more collisions.
 */
static unsigned long
sfshash_fn(const char *path, const char * key, uid_t uid)
{
	unsigned long ret = uid & SFSHASH_MASK;
	for (; *path; path++) {
		char	tmp = tolower(*path);
		ret <<= 1;
		ret ^= tmp;
		ret = (ret ^ (ret >> SFSHASH_SHIFT)) & SFSHASH_MASK;
	}
	if (key) {
		for (; *key; key++) {
			char	tmp = tolower(*key);
			ret <<= 1;
			ret ^= tmp;
			ret = (ret ^ (ret >> SFSHASH_SHIFT)) & SFSHASH_MASK;
		}
	}
	return ret & SFSHASH_MASK;
}

/**
 * Remove an entry from the sfs hash and free it.
 *
 * @param entry The entry to remove. It must be in the sfs hash.
 */
static void
sfshash_remove_entry(struct sfshash_entry *entry)
{
	int slot = entry->slot;
	int is_key = (entry->cstype & CSTYPE_MASK) == CSTYPE_KEY;
	int is_uid = (entry->cstype & CSTYPE_MASK) == CSTYPE_UID;

	DEBUG(DBG_SFSCACHE, ">sfshash_remove_entry: %s %d %s %s", entry->path,
	    is_uid ? (int)entry->u.uid : -1, is_key ? entry->u.key : NULL,
	    (entry->cstype & CSTYPE_NEGATIVE) ? "negative" : "positive");
	TAILQ_REMOVE(&sfshash_tab[slot], entry, hash_link);
	TAILQ_REMOVE(&sfshash_lru, entry, lru_link);
	if (entry->path)
		free(entry->path);
	if (is_key)
		free(entry->u.key);
	abuf_free(entry->csum);
	abuf_free_type(entry, struct sfshash_entry);
	sfshash_entries--;
}

/**
 * Insert a new entry into the sfs hash. This function does fill the
 * new entry it simply inserts it into the hash table. If the total number
 * of entries exceeds the limit SFSHASH_MAX some entries are removed and
 * freed as a side effect. Entries that have not been used for a long
 * period of time ar removed first.
 *
 * @param slot The hash slot
 * @param entry The new entry.
 */
static void
sfshash_insert_entry(int slot, struct sfshash_entry *entry)
{
	int is_key = (entry->cstype & CSTYPE_MASK) == CSTYPE_KEY;
	int is_uid = (entry->cstype & CSTYPE_MASK) == CSTYPE_UID;

	DEBUG(DBG_SFSCACHE, ">sfshash_insert_entry: %s %d %s %s", entry->path,
	    is_uid ? (int)entry->u.uid : -1, is_key ? entry->u.key : NULL,
	    (entry->cstype & CSTYPE_NEGATIVE) ? "negative" : "positive");
	entry->slot = slot;
	TAILQ_INSERT_HEAD(&sfshash_tab[slot], entry, hash_link);
	TAILQ_INSERT_TAIL(&sfshash_lru, entry, lru_link);
	sfshash_entries++;
	while(sfshash_entries > SFSHASH_MAX) {
		sfshash_remove_entry(TAILQ_FIRST(&sfshash_lru));
	}
}

/**
 * Move an entry that that has been looked up to the end of the LRU
 * list (LRU == least recently used). Entries start there life at the end
 * of this list and are moved to the end if they are looked up. Entries
 * at the start of the list are removed if some hash entries must be freed.
 * Additionally, the entry is moved to the front of the list for its
 * hash slot. This should improve performance in case of collisions.
 *
 * @param entry The entry.
 */
static void
sfshash_touch(struct sfshash_entry *entry)
{
	int slot = entry->slot;
	TAILQ_REMOVE(&sfshash_tab[slot], entry, hash_link);
	TAILQ_INSERT_HEAD(&sfshash_tab[slot], entry, hash_link);
	TAILQ_REMOVE(&sfshash_lru, entry, lru_link);
	TAILQ_INSERT_TAIL(&sfshash_lru, entry, lru_link);
}

/**
 * Remove all entries from the sfs hash. This can be called after
 * a large scale checksum update e.g. due to an upgrade. If only
 * individual checksums are change, the approriate invalidate funcions
 * should be used.
 */
void
sfshash_flush(void)
{
	while (!TAILQ_EMPTY(&sfshash_lru))
		sfshash_remove_entry(TAILQ_FIRST(&sfshash_lru));
}

/**
 * Common code that inserts a new entry into the sfs hash.
 *
 * @param path The path name of the file.
 * @param csum The checksum. This function takes over ownership for this buffer.
 *     It will be freed once the entry is removed from the hash. It will
 *     be freed in case of an error, too.
 * @param cstype The checksum type: either CSTYPE_UID or CSTYPE_KEY.
 * @param uid For CSTYPE_UID requests this is the user-ID of the user that
 *     stored the checksum. It must be (uid_t)-1 for CSTYPE_KEY requests.
 * @param key For CSTYPE_KEY requests this is the key-ID of the key that
 *     stored the checksum. It must be NULL for CSTYPE_UID requests and
 *     a string of printable hex digits for CSTYPE_KEY requests.
 * @return Zero in case of success, a negative error code in case of an
 *     error.
 *
 * The path name and the key-ID are copied into the sfs entry, i.e. the
 * caller retains ownership of the memory pointed to by path and key.
 * However, ownership of the checksum buffer is taken over by this
 * function and the caller must not free it on its own.
 *
 * This function will _not_ remove a pre-existing entry for the same
 * path/uid or path/key pair. The caller must do a lookup before calling
 * this function.
 */
static int
sfshash_insert_common(const char *path, struct abuf_buffer csum,
    int cstype, uid_t uid, const char *key)
{
	struct sfshash_entry	*entry;
	int			 slot, ret;

	entry = abuf_alloc_type(struct sfshash_entry);
	if (entry == NULL) {
		abuf_free(csum);
		return -ENOMEM;
	}
	ret = -EINVAL;
	entry->path = NULL;
	entry->cstype = cstype;
	entry->csum = csum;
	switch(cstype & CSTYPE_MASK) {
	case CSTYPE_UID:
		entry->u.uid = uid;
		break;
	case CSTYPE_KEY:
		if (!key)
			goto err;
		ret = -ENOMEM;
		entry->u.key = strdup(key);
		if (entry->u.key == NULL)
			goto err;
		break;
	default:
		goto err;
	}
	ret = -ENOMEM;
	entry->path = strdup(path);
	if (entry->path == NULL)
		goto err;
	slot = sfshash_fn(path, key, uid);
	sfshash_insert_entry(slot, entry);
	return 0;
err:
	if (entry->path)
		free(entry->path);
	if ((entry->cstype & CSTYPE_MASK) == CSTYPE_KEY && entry->u.key)
		free(entry->u.key);
	abuf_free(entry->csum);
	abuf_free_type(entry, struct sfshash_entry);
	return ret;
}

/**
 * Insert a new checksum for the given path and user-ID into the
 * hash.
 *
 * @param path The path name.
 * @param uid The user-ID of the user.
 * @param csum The checksum. Ownership of the checksum buffer is taken over
 *     by this function. It must not be freed by the caller.
 */
static int
sfshash_insert_uid(const char *path, uid_t uid, struct abuf_buffer csum)
{
	return sfshash_insert_common(path, csum, CSTYPE_UID, uid, NULL);
}

/**
 * Insert a negative entry into the sfshash for the given path and user-ID.
 * Subsequent lookups will return -ENOENT for this path/uid pair.
 *
 * @param path The path name.
 * @param uid The user-ID.
 */
static int
sfshash_insert_uid_negative(const char *path, uid_t uid)
{
	return sfshash_insert_common(path, ABUF_EMPTY,
	    CSTYPE_UID|CSTYPE_NEGATIVE, uid, NULL);
}

/**
 * Insert a new checksum for the given path and key-ID into the hash.
 *
 * @param path The path name.
 * @param key The key-ID of the key as a string of printable hey digits.
 * @param csum The checksum data. Ownership of the buffer is taken over
 *     by this function, i.e. the buffer must not be freed by the caller.
 */
static int
sfshash_insert_key(const char *path, const char *key, struct abuf_buffer csum)
{
	return sfshash_insert_common(path, csum, CSTYPE_KEY, (uid_t)-1, key);
}

/**
 * Insert a negative entry for the path/key-ID into the hash. Subsequent
 * lookups will return -ENOENT for this path/uid pair.
 *
 * @param path The path name.
 * @param key The key-ID as a string of printable hex digits.
 */
static int
sfshash_insert_key_negative(const char *path, const char *key)
{
	return sfshash_insert_common(path, ABUF_EMPTY,
	    CSTYPE_KEY|CSTYPE_NEGATIVE, (uid_t)-1, key);
}

/**
 * Search for the hash entry for the given path and user-ID.
 *
 * @param path The path name.
 * @param uid The user-ID.
 * @return A pointer to the hash entry or NULL if no matching entry
 *     was found.
 */
static struct sfshash_entry *
sfshash_find_uid(const char *path, uid_t uid)
{
	int			 slot = sfshash_fn(path, NULL, uid);
	struct sfshash_entry	*entry;

	TAILQ_FOREACH(entry, &sfshash_tab[slot], hash_link) {
		if ((entry->cstype & CSTYPE_MASK) != CSTYPE_UID)
			continue;
		if (entry->u.uid != uid)
			continue;
		if (strcmp(path, entry->path) == 0)
			break;
	}
	if (entry)
		sfshash_touch(entry);
	return entry;
}

/**
 * Search for the hash entry for the given path and key-ID.
 *
 * @param path The path name.
 * @param key The key-ID as a string of printable hex digits.
 * @return A pointer to the hash entry or NULL if no matching entry
 *     was found.
 */
static struct sfshash_entry *
sfshash_find_key(const char *path, const char *key)
{
	int			 slot = sfshash_fn(path, key, (uid_t)-1);
	struct sfshash_entry	*entry;

	TAILQ_FOREACH(entry, &sfshash_tab[slot], hash_link) {
		if ((entry->cstype & CSTYPE_MASK) != CSTYPE_KEY)
			continue;
		if (strcasecmp(entry->u.key, key) != 0)
			continue;
		if (strcmp(path, entry->path) == 0)
			break;
	}
	if (entry)
		sfshash_touch(entry);
	return entry;
}

/**
 * Utility function that converts upper and lower case hex digits to their
 * corresponding integer value.
 *
 * @param ch A character that should represent a hexadecimal digit.
 * @return The decimal value of the digit or -1 if the character is not
 *     a hex digit.
 */
static int
chartohex(char ch)
{
#ifndef S_SPLINT_S
	switch(ch) {
	case '0' ... '9':
		return ch - '0';
	case 'A' ... 'F':
		return 10 + ch - 'A';
	case 'a' ... 'f':
		return 10 + ch - 'a';
	}
#endif
	return -1;
}

/**
 * Read the checksum data of the specified type from the on-disk
 * SFS tree. The checksum data (if any) is stored in a new buffer.
 * The memory associated with the buffer must be freed by the caller.
 *
 * @param path The path name of the file.
 * @param cstype This is either CSTYPE_UID or CSTYPE_KEY
 * @param key The Key-ID for CSTYPE_KEY request. The key-ID is given in
 *     human readable form (i.e. a string of printable hex digits). It
 *     should be NULL if the request type is not CSTYPE_KEY.
 * @param uid The user-ID for CSTYPE_UID requests.
 * @param csum This should point to an emtpy abuf buffer. The buffer
 *     will be allocated and filled with the checksum data in case of success.
 *     The caller is responsible for the freeing of the buffer.
 *     The buffer will only contain the checksum without any associated
 *     signature data.
 * @return Zero in case of success, a negative error code in case of an
 *     an error. The buffer is initialized to ABUF_EMPTY if an error
 *     occurs.
 */
static int
sfshash_readsum(const char *path, int cstype, const char *key, uid_t uid,
    struct abuf_buffer *csum)
{
	int			 ret;
	struct abuf_buffer	 sigbuf = ABUF_EMPTY;
	struct sfs_checksumop	 csop;
	struct sfs_data		 tmpdata;
	struct abuf_buffer	 keybuf = ABUF_EMPTY;

	csop.path = path;
	csop.sigbuf = ABUF_EMPTY;
	csop.listflags = 0;
	csop.auth_uid = 0;

	(*csum) = ABUF_EMPTY;
	switch(cstype) {
	case CSTYPE_UID:
		csop.uid = uid;
		csop.op = ANOUBIS_CHECKSUM_OP_GET2;
		csop.keyid = ABUF_EMPTY;
		break;
	case CSTYPE_KEY: {
		int			 i, j, len = strlen(key);
		uint8_t			*keyptr;

		csop.uid = 0;
		csop.op = ANOUBIS_CHECKSUM_OP_GETSIG2;
		if (len % 2)
			return -ENOENT;
		len /= 2;
		keybuf = abuf_alloc(len);
		if (abuf_empty(keybuf))
			return -ENOMEM;
		keyptr = abuf_toptr(keybuf, 0, len);
		for (i=j=0; i<len; ++i) {
			int		c1, c2;
			c1 = chartohex(key[j++]);
			c2 = chartohex(key[j++]);
			if (c1 < 0 || c2 < 0) {
				abuf_free(keybuf);
				return -ENOENT;
			}
			keyptr[i] = 16*c1 + c2;
		}
		csop.keyid = keybuf;
		break;
	} default:
		return -EINVAL;
	}
	ret = sfs_checksumop_chroot(&csop, &tmpdata);
	abuf_free(keybuf);
	if (ret < 0)
		return ret;
	if (cstype == CSTYPE_UID) {
		sigbuf = tmpdata.csdata;
		tmpdata.csdata = ABUF_EMPTY;
	} else {
		/*
		 * XXX CEH: Use upgrade signature if present and upgrade
		 * XXX CEH: in progress?
		 * XXX CEH: What if signature is cached?
		 */
		sigbuf = tmpdata.sigdata;
		tmpdata.sigdata = ABUF_EMPTY;
	}
	sfs_freesfsdata(&tmpdata);
	if (abuf_length(sigbuf) < ANOUBIS_CS_LEN) {
		abuf_free(sigbuf);
		return -ENOENT;
	}
	/*
	 * Clone the first ANOUBIS_CS_LEN bytes of the buffer if the buffer
	 * contains additional data.
	 */
	if (abuf_length(sigbuf) > ANOUBIS_CS_LEN) {
		abuf_limit(&sigbuf, ANOUBIS_CS_LEN);
		(*csum) = abuf_clone(sigbuf);
		abuf_free(sigbuf);
		if (abuf_empty(*csum))
			return -ENOMEM;
	} else {
		(*csum) = sigbuf;
	}

	return 0;
}

/**
 * Return the unsigned checksum associated with a given path. The checksum
 * data (if any) is returned in an abuf_buffer that is allocated by
 * this function and must be freed by the caller.
 *
 * @param path The path of the file.
 * @param uid The user ID of the user that stored the checksum.
 * @param csum The checksum data will be returned in this buffer.
 *     The buffer will be usable but empty if an error is returned.
 *     The caller is responsible for the memory associated with the buffer.
 * @return Zero in case of success, a negative error code in case of an
 *     error.
 */
int
sfshash_get_uid(const char *path, uid_t uid, struct abuf_buffer *csum)
{
	struct sfshash_entry	*entry;
	int			 ret;


	DEBUG(DBG_SFSCACHE, ">sfshash_get_uid: %s %d", path, (int)uid);
	(*csum) = ABUF_EMPTY;
	entry = sfshash_find_uid(path, uid);
	if (entry) {
		if (entry->cstype & CSTYPE_NEGATIVE) {
			DEBUG(DBG_SFSCACHE, "<sfshash_get_uid: ok "
			    "%s %d (negative)", path, (int)uid);
			return -ENOENT;
		}
		(*csum) = abuf_clone(entry->csum);
	} else {
		struct abuf_buffer	 tmpbuf = ABUF_EMPTY;
		ret = sfshash_readsum(path, CSTYPE_UID, NULL, uid, &tmpbuf);
		if (ret < 0) {
			abuf_free(tmpbuf);
			if (ret == -ENOENT) {
				sfshash_insert_uid_negative(path, uid);
				DEBUG(DBG_SFSCACHE, "<sfshash_get_uid: ok "
				    "%s %d (negative)", path, (int)uid);
			} else {
				DEBUG(DBG_SFSCACHE, "<sfshash_get_uid: fail "
				    "%s %d error %d", path, (int)uid, -ret);
			}
			return ret;
		}
		(*csum) = abuf_clone(tmpbuf);
		/* This takes over control of the buffer @tmpbuf. */
		sfshash_insert_uid(path, uid, tmpbuf);
	}
	if (abuf_empty(*csum))
		return -ENOMEM;
	DEBUG(DBG_SFSCACHE, "<sfshash_get_uid: ok %s %d ", path, (int)uid);
	return 0;
}

/**
 * Return the unsigned signed checksum associated with a given path.
 * The checksum data (if any) is returned in an abuf_buffer that is
 * allocated by this function and must be freed by the calle.
 *
 * @param path The path of the file.
 * @param key The Key-ID of the key that stored the checksum. The Key-ID
 *     is given as a human readable string of hex digits.
 * @param csum The checksum data will be returned in this buffer.
 *     The buffer will be usable but empty if an error is returned.
 *     The caller is responsible for the memory associated with the buffer.
 * @return Zero in case of success, a negative error code in case of an
 *     error.
 */
int
sfshash_get_key(const char *path, const char *key, struct abuf_buffer *csum)
{
	struct sfshash_entry	*entry;
	int			 ret;

	DEBUG(DBG_SFSCACHE, ">sfshash_get_key: %s %s", path, key);
	entry = sfshash_find_key(path, key);
	if (entry) {
		if (entry->cstype & CSTYPE_NEGATIVE) {
			DEBUG(DBG_SFSCACHE, "<sfshash_get_key: ok "
			    "%s %s negative", path, key);
			return -ENOENT;
		}
		(*csum) = abuf_clone(entry->csum);
	} else {
		struct abuf_buffer	tmpbuf = ABUF_EMPTY;

		ret = sfshash_readsum(path, CSTYPE_KEY, key, (uid_t)-1,
		    &tmpbuf);
		if (ret < 0) {
			if (ret == -ENOENT) {
				sfshash_insert_key_negative(path, key);
				DEBUG(DBG_SFSCACHE, "<sfshash_get_key: ok "
				    "%s %s negative", path, key);
			} else {
				DEBUG(DBG_SFSCACHE, "<sfshash_get_key: fail "
				    "%s %s error %d", path, key, -ret);
			}
			return ret;
		}
		(*csum) = abuf_clone(tmpbuf);
		sfshash_insert_key(path, key, tmpbuf);
	}
	if (abuf_empty(*csum))
		return -ENOMEM;
	DEBUG(DBG_SFSCACHE, "<sfshash_get_key: ok %s %s", path, key);
	return 0;
}

/**
 * Remove an unsigned checksum from the hash (if it exists). This is
 * used if the checksum data on disk changes.
 *
 * @param path The path of the file.
 * @param uid The user-ID of the user that stored the checksum.
 * @return None.
 */
void
sfshash_invalidate_uid(const char *path,  uid_t uid)
{
	struct sfshash_entry	*entry;

	if ((entry = sfshash_find_uid(path, uid)))
		sfshash_remove_entry(entry);
}

/**
 * Remove a signed checksum from the hash (if it exists). This is used if
 * the checksum data on disk changes.
 *
 * @param path The path of the file.
 * @param key The Key-ID of the key that stored the signature. The Key-ID
 *     is given as a string of printable hex digits.
 * @return None.
 */
void
sfshash_invalidate_key(const char *path, const char *key)
{
	struct sfshash_entry	*entry;

	if ((entry = sfshash_find_key(path, key)))
		sfshash_remove_entry(entry);
}
