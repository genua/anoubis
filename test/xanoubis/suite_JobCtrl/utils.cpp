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
#include <fcntl.h>
#include <unistd.h>

#include <openssl/sha.h>

#include <config.h>
#ifdef NEEDBSDCOMPAT
#include <linux/anoubis.h>
#include <bsdcompat.h>
#else
#include <sha2.h>
#include <dev/anoubis.h>
#endif

#include <wx/ffile.h>
#include <wx/filename.h>

#include "utils.h"

bool
JobCtrl_execute(const char *format, ...)
{
	char cmd[PATH_MAX];
	va_list ap;

	va_start(ap, format);
	vsnprintf(cmd, sizeof(cmd), format, ap);
	va_end(ap);

	int rc = system(cmd);
	return (WEXITSTATUS(rc) == 0);
}

bool
JobCtrl_calculate_checksum(const wxString &file, u_int8_t *cs, int cslen)
{
	SHA256_CTX	shaCtx;
	int		fd, nread;
	unsigned char	buf[1024];

	if ((cs == NULL) || (cslen < ANOUBIS_CS_LEN)) {
		return (false);
	}

	fd = open(file.To8BitData(), O_RDONLY);
	if (fd == -1)
		return (false);

	SHA256_Init(&shaCtx);

	/* Read file chunk by chunk and put it into SHA256_CTX */
	while ((nread = read(fd, buf, sizeof(buf))) > 0) {
		SHA256_Update(&shaCtx, buf, nread);
	}

	SHA256_Final(cs, &shaCtx);

	close(fd);

	if (nread == -1) {
		/* read operation failed */
		return (false);
	}

	return (true);
}

bool
JobCtrl_calculate_checksum_buffer(const char *buffer, int buf_len,
    u_int8_t *cs, int cslen)
{
	SHA256_CTX shaCtx;

	if ((buffer == NULL) || (cs == NULL) || (cslen < ANOUBIS_CS_LEN)) {
		return (false);
	}

	SHA256_Init(&shaCtx);
	SHA256_Update(&shaCtx, buffer, buf_len);
	SHA256_Final(cs, &shaCtx);

	return (true);
}

wxString
JobCtrl_tempfile(void)
{
	wxFFile file;
	wxString fileName = wxFileName::CreateTempFileName(
	    wxT("JobCtrl"), &file);

	if (!file.Write(wxT("Hello World!\n"))) {
		return (wxEmptyString);
	}

	if (!file.Flush()) {
		return (wxEmptyString);
	}

	file.Close();

	return (fileName);
}
