/*
 * Copyright (c) 2007 GeNUA mbH <info@genua.de>
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

#include <anoubis_errno.h>
#define _(String) String

#ifndef lint
static const char *a_error_str[] = {
	/*
	 * array should begin with entry '0', not '1024'
	 */
	[A_EPERM_NO_CERTIFICATE - A_ERRORCODE_BASE] = _("Operation not "
	    "permitted: Client certificate is not known to the daemon"),
	[A_EPERM_UID_MISMATCH - A_ERRORCODE_BASE] = _("Operation not "
	    "permitted: User ID and certificate's User ID do not match"),
	[A_KEYLOADFAIL - A_ERRORCODE_BASE ] = _("Failed to load private key"),
	[A_RULESET_NOT_LOADED - A_ERRORCODE_BASE] = _("No Ruleset has been "
	    "loaded"),
	[A_RULESET_WRONG_PROFILE - A_ERRORCODE_BASE] = _("Only export to "
	    "user-profile may be allowed"),
	[A_RULESET_MISSING_RIGHTS - A_ERRORCODE_BASE] = _("Could not open "
	    "for writing"),
	[A_RULESET_NO_BACKUP - A_ERRORCODE_BASE] = _("Could not create backup")
};
#else
/*
 * lint doesn't know how to handle '[A_* - A_*] = _(*)'
 */
static const char *a_error_str[];
#endif

char *
anoubis_strerror(int errnum)
{
	char *reply = NULL;

	if (errnum < 0) {
		reply = dgettext("anoubis", "invalid error code");
	} else {
		if (errnum < 1024) {
			reply = strerror(errnum);
		}
		/*
		 * while a_error_str[] has defined entries
		 */
		else if ( errnum - A_ERRORCODE_BASE <
			    (int) (sizeof(a_error_str) / sizeof(const char *)))
		{
			reply = dgettext("anoubis",
				a_error_str[errnum - A_ERRORCODE_BASE]);
		}
		if (reply == NULL) {
			reply = dgettext("anoubis",
					"undefined anoubis error code");
		}
	}
	return reply;
}
