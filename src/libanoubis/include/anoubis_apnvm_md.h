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

#ifndef _APNMD_H_
#define _APNMD_H_

struct _apnmd;
typedef struct _apnmd apnmd;

/**
 * Parses the specified metadata-string and creates a apnmd-structure. If you
 * specify an empty or NULL-string an empty meta-database is created (and
 * nothing is parsed).
 *
 * Format of metadata:
 *
 * Metadata an enclosed in "<apnvm-metadata>" and "</apnvm-metadata>" tags.
 * Everything outside the tags is ignored.
 *
 * Metadata are organized in key-value-pairs:
 * <pre>
 * &lt;key&gt; := &lt;value&gt;
 * </pre>
 *
 * If you want to append another line to a key, use the "+=" operator:
 * <pre>
 * &lt;key&gt; += &lt;another value&gt;
 * </pre>
 *
 * This results into a value <code>&lt;value&gt;\n&lt;another value&gt;</code>
 * for the metadata-key <code>&lt;key&gt;</code>.
 */
apnmd *apnmd_parse(const char *);

/**
 * Destroys the apnmd-datastructure created by apnmd_parse().
 */
void apnmd_destroy(apnmd *);

/**
 * Serializes the meta-database into a string.
 *
 * Memory of the resulting string is allocated using malloc(3). Use free(3) to
 * release the memory again.
 */
char *apnmd_serialize(apnmd *);

/**
 * Returns the number of items in the meta-database.
 */
int apnmd_count(apnmd *);

/**
 * Returns the value of an item in the meta-database. The item is identified
 * with the key of the second parameter. NULL is returned, if the key does not
 * exist. Memory allocated for the return-value is handled internally. Do not
 * release the memory by yourself!
 */
char *apnmd_get(apnmd *, const char *);

/**
 * Returns the integer-value of an item in the meta-database. The item is
 * identified with the key of the second parameter. If the key does not exist
 * or conversionn failed, 0 is returned.
 */
int apnmd_get_int(apnmd *, const char *);

/**
 * Appends a new item into the meta-database. The second parameter specifies
 * the key, the third the value of the item. If the key already exists, the
 * value is overwritten.
 */
void apnmd_put(apnmd *, const char *, const char *);

/**
 * Appends a new item into the meta-database. The second parameter specifies
 * the key, the third the value of the item. If the key already exists, the
 * value is overwritten.
 */
void apnmd_put_int(apnmd *, const char *, int);

/**
 * Appends another line to an maybe already existing meta-database-entry. The
 * second parameter specifies the key, the third the value to be appended. If
 * the entry does not exist, the function behaves like apnmd_put().
 */
void apnmd_add(apnmd *, const char *, const char *);

/**
 * Appends another line to an maybe already existing meta-database-entry. The
 * second parameter specifies the key, the third the integer-value to be
 * appended. If he entry does not exist, the function behaves like
 * apnmd_put_int().
 */
void apnmd_add_int(apnmd *, const char *, int);

#endif	/* _APNMD_H_ */
