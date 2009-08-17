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

#ifndef NEEDBSDCOMPAT
#include <sys/queue.h>
#else
#include <queue.h>
#endif

#include <anoubisd.h>
#include <sfs.h>
#include <anoubis_protocol.h>

struct sfshash_entry;

#define CSTYPE_NEGATIVE		0x1000
#define CSTYPE_MASK		0xfff
#define CSTYPE_UID		1
#define CSTYPE_KEY		2

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
	u_int8_t			 csum[ANOUBIS_CS_LEN];
};

#define SFSHASH_SHIFT		 (13)
#define SFSHASH_NRENTRY		 (1<<13)
#define SFSHASH_MASK		 (SFSHASH_NRENTRY-1)
#define SFSHASH_MAX		 (4*SFSHASH_NRENTRY)

TAILQ_HEAD(, sfshash_entry)	sfshash_tab[SFSHASH_NRENTRY];
TAILQ_HEAD(, sfshash_entry)	sfshash_lru;
int				sfshash_entries;

void
sfshash_init(void)
{
	int i;

	TAILQ_INIT(&sfshash_lru);
	for (i=0; i<SFSHASH_NRENTRY; ++i)
		TAILQ_INIT(&sfshash_tab[i]);
	sfshash_entries = 0;
}

/*
 * NOTE: Keys should be printed hex strings. As such they must be
 * NOTE: handled case insensitive.
 *
 * XXX CEH: Evaluate hash function
 */
static unsigned long
sfshash_fn(const char *path, const char * key, uid_t uid)
{
	unsigned long ret = uid & SFSHASH_MASK;
	for (; *path; path++) {
		ret <<= 1;
		ret ^= *path;
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
	free(entry);
	sfshash_entries--;
}

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

static void
sfshash_touch(struct sfshash_entry *entry)
{
	int slot = entry->slot;
	TAILQ_REMOVE(&sfshash_tab[slot], entry, hash_link);
	TAILQ_INSERT_HEAD(&sfshash_tab[slot], entry, hash_link);
	TAILQ_REMOVE(&sfshash_lru, entry, lru_link);
	TAILQ_INSERT_TAIL(&sfshash_lru, entry, lru_link);
}

void
sfshash_flush(void)
{
	while (!TAILQ_EMPTY(&sfshash_lru))
		sfshash_remove_entry(TAILQ_FIRST(&sfshash_lru));
}

static int
sfshash_insert_common(const char *path, const u_int8_t csum[ANOUBIS_CS_LEN],
    int cstype, uid_t uid, const char *key)
{
	struct sfshash_entry	*entry;
	int			 slot;

	entry = malloc(sizeof(struct sfshash_entry));
	if (entry == NULL)
		return -ENOMEM;
	entry->path = NULL;
	entry->cstype = cstype;
	switch(cstype & CSTYPE_MASK) {
	case CSTYPE_UID:
		entry->u.uid = uid;
		break;
	case CSTYPE_KEY:
		if (!key) {
			free(entry);
			return -EINVAL;
		}
		entry->u.key = strdup(key);
		if (entry->u.key == NULL)
			goto oom;
		break;
	default:
		free(entry);
		return -EINVAL;
	}
	entry->path = strdup(path);
	if (entry->path == NULL)
		goto oom;
	slot = sfshash_fn(path, key, uid);
	if ((cstype & CSTYPE_NEGATIVE) == 0)
		memcpy(entry->csum, csum, ANOUBIS_CS_LEN);
	sfshash_insert_entry(slot, entry);
	return 0;
oom:
	if (entry->path)
		free(entry->path);
	if ((entry->cstype & CSTYPE_MASK) == CSTYPE_KEY && entry->u.key)
		free(entry->u.key);
	free(entry);
	return -ENOMEM;
}

static int
sfshash_insert_uid(const char *path, uid_t uid,
    const u_int8_t hash[ANOUBIS_CS_LEN])
{
	return sfshash_insert_common(path, hash, CSTYPE_UID, uid, NULL);
}

static int
sfshash_insert_uid_negative(const char *path, uid_t uid)
{
	return sfshash_insert_common(path, NULL, CSTYPE_UID|CSTYPE_NEGATIVE,
	    uid, NULL);
}

static int
sfshash_insert_key(const char *path, const char *key,
    const u_int8_t hash[ANOUBIS_CS_LEN])
{
	return sfshash_insert_common(path, hash, CSTYPE_KEY, (uid_t)-1, key);
}

static int
sfshash_insert_key_negative(const char *path, const char *key)
{
	return sfshash_insert_common(path, NULL, CSTYPE_KEY|CSTYPE_NEGATIVE,
	    (uid_t)-1, key);
}

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

static int
sfshash_readsum(const char *path, int cstype, const char *key, uid_t uid,
    u_int8_t csum[ANOUBIS_CS_LEN])
{
	int			 ret, siglen = 0;
	void			*sigdata = NULL;
	struct sfs_checksumop	 csop;
	struct sfs_data		 tmpdata;
	u_int8_t		*keybuf = NULL;

	csop.path = path;
	csop.siglen = 0;
	csop.sigdata = NULL;
	csop.listflags = 0;
	csop.auth_uid = 0;

	switch(cstype) {
	case CSTYPE_UID:
		csop.uid = uid;
		csop.op = ANOUBIS_CHECKSUM_OP_GET2;
		csop.idlen = 0;
		csop.keyid = NULL;
		break;
	case CSTYPE_KEY: {
		int		 i, j, len = strlen(key);

		csop.uid = 0;
		csop.op = ANOUBIS_CHECKSUM_OP_GETSIG2;
		if (len % 2)
			return -ENOENT;
		csop.idlen = len / 2;
		if ((keybuf = malloc(csop.idlen)) == NULL)
			return -ENOMEM;
		for (i=j=0; i<csop.idlen; ++i) {
			int		c1, c2;
			c1 = chartohex(key[j++]);
			c2 = chartohex(key[j++]);
			if (c1 < 0 || c2 < 0) {
				free(keybuf);
				return -ENOENT;
			}
			keybuf[i] = 16*c1 + c2;
		}
		csop.keyid = keybuf;
		break;
	} default:
		return -EINVAL;
	}
	ret = sfs_checksumop_chroot(&csop, &tmpdata);
	if (keybuf)
		free(keybuf);
	if (ret < 0)
		return ret;
	if (cstype == CSTYPE_UID) {
		siglen = tmpdata.cslen;
		sigdata = tmpdata.csdata;
		tmpdata.csdata = NULL;
	} else {
		/*
		 * XXX CEH: Use upgrade signature if present and upgrade
		 * XXX CEH: in progress?
		 * XXX CEH: What if signature is cached?
		 */
		siglen = tmpdata.siglen;
		sigdata = tmpdata.sigdata;
		tmpdata.sigdata = NULL;
	}
	sfs_freesfsdata(&tmpdata);
	if (siglen < ANOUBIS_CS_LEN) {
		if (sigdata)
			free(sigdata);
		return -ENOENT;
	}
	memcpy(csum, sigdata, ANOUBIS_CS_LEN);
	free(sigdata);

	return 0;
}

int
sfshash_get_uid(const char *path, uid_t uid, u_int8_t csum[ANOUBIS_CS_LEN])
{
	struct sfshash_entry	*entry;
	int			 ret;

	DEBUG(DBG_SFSCACHE, ">sfshash_get_uid: %s %d", path, (int)uid);
	entry = sfshash_find_uid(path, uid);
	if (entry) {
		if (entry->cstype & CSTYPE_NEGATIVE) {
			DEBUG(DBG_SFSCACHE, "<sfshash_get_uid: ok "
			    "%s %d (negative)", path, (int)uid);
			return -ENOENT;
		}
		memcpy(csum, entry->csum, ANOUBIS_CS_LEN);
	} else {
		ret = sfshash_readsum(path, CSTYPE_UID, NULL, uid, csum);
		if (ret < 0) {
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
		sfshash_insert_uid(path, uid, csum);
	}
	DEBUG(DBG_SFSCACHE, "<sfshash_get_uid: ok %s %d "
	    "%02x%02x%02x%02x...", path, (int)uid, csum[0],
	    csum[1], csum[2], csum[3]);
	return 0;
}

int
sfshash_get_key(const char *path, const char *key,
    u_int8_t csum[ANOUBIS_CS_LEN])
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
		memcpy(csum, entry->csum, ANOUBIS_CS_LEN);
	} else {
		ret = sfshash_readsum(path, CSTYPE_KEY, key, (uid_t)-1, csum);
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
		sfshash_insert_key(path, key, csum);
	}
	DEBUG(DBG_SFSCACHE, "<sfshash_get_key: ok %s %s "
	    "%02x%02x%02x%02x...", path, key, csum[0],
	    csum[1], csum[2], csum[3]);
	return 0;
}

void
sfshash_invalidate_uid(const char *path,  uid_t uid)
{
	struct sfshash_entry	*entry;

	if ((entry = sfshash_find_uid(path, uid)))
		sfshash_remove_entry(entry);
}

void
sfshash_invalidate_key(const char *path, const char *key)
{
	struct sfshash_entry	*entry;

	if ((entry = sfshash_find_key(path, key)))
		sfshash_remove_entry(entry);
}
