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

#ifndef _KEYGEN_H_
#define _KEYGEN_H_

#include <sys/types.h>
#include <sys/cdefs.h>

/**
 * Information about a subject as used in SSL certificates. It encapsulates
 * information about an entity (person, company, ...). Each field has a
 * shortcut that is used in the subject string that can be generated from
 * this structure. The keys used in subject strings that correspond to these
 * field are given for reference purposes.
 *
 * Memory management: The structure it self, all fields contained within
 * it and the string returned by anoubis_keysubject_tostring are allocated
 * by malloc and must be freed. In particular anoubis_keysubject_destroy
 * will free all non-NULL fields within anoubis_keysubject using free.
 * Please keep that in mind if you change any fields in that structure.
 */
struct anoubis_keysubject {
	/** The country/nation (/C=...) */
	char		*country;
	/** state The state or province (/ST=...) */
	char		*state;
	/** locality The city, street etc. within the state (/L=...) */
	char		*locality;
	/** organization The organization (/O=...) */
	char		*organization;
	/** orgunit The sub-unit with the organization (/ON=...) */
	char		*orgunit;
	/** name The common name of the entity (/CN=...) */
	char		*name;
	/** email The mail-Address of the entity (/emailAddress=...) */
	char		*email;
};


__BEGIN_DECLS
extern int	anoubis_keygen(const char *priv, const char *pub,
		    const char *pass, const char *subject, int bits);
extern struct anoubis_keysubject *anoubis_keysubject_defaults(void);
extern struct anoubis_keysubject *anoubis_keysubject_fromstring(const char *);
extern char *anoubis_keysubject_tostring(const struct anoubis_keysubject *);
extern void anoubis_keysubject_destroy(struct anoubis_keysubject *);
__END_DECLS

#endif	/* _KEYGEN_H_ */
