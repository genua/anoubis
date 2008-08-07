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

#include <accbuffer.h>
#include <check.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

achat_buffer *buffer;

void tc_buffer_setup()
{
	buffer = malloc(sizeof(struct achat_buffer));
}

void tc_buffer_teardown()
{
	free(buffer);
	buffer = NULL;
}

START_TEST(tc_buffer_initrelease)
{
	achat_rc	rc;

	rc= acc_bufferinit(buffer);
	fail_if(rc != ACHAT_RC_OK,
		"Failed to initialize buffer, rc = %i", rc);
	mark_point();
	acc_bufferclear(buffer);
	rc = acc_bufferfree(buffer);
	fail_if(rc != ACHAT_RC_OK, "Failed to free the buffer, rc = %i", rc);
	mark_point();
}
END_TEST

START_TEST(tc_buffer_add_lt_defaultsz)
{
	const char	append_buf[] = {'1', '2', '3', '4', '5'};
	size_t		bsize;
	void		*buf;
	achat_rc	rc;

	rc = acc_bufferinit(buffer);
	fail_if(rc != ACHAT_RC_OK,
		"Failed to initialize buffer, rc = %i", rc);
	mark_point();

	/* Make sure length of append_buf is really smaller */
	/* than ACHAT_BUFFER_DEFAULTSZ */
	fail_if(sizeof(append_buf) > ACHAT_BUFFER_DEFAULTSZ,
		"len(append_buf) > ACHAT_BUFFER_DEFAULTSZ");
	mark_point();

	rc = acc_bufferappend(buffer, append_buf, sizeof(append_buf));
	fail_if(rc != ACHAT_RC_OK,
		"Failed to append to buffer, rc = %i", rc);
	mark_point();

	bsize = acc_bufferlen(buffer);
	fail_if(bsize != sizeof(append_buf),
		"Length of buffer is wrong, is %i but %i expected",
		bsize, sizeof(append_buf));
	mark_point();

	buf = acc_bufferptr(buffer);
	fail_if(memcmp(buf, append_buf, bsize) != 0,
		"Unexpected content of buffer");
	mark_point();

	rc = acc_bufferfree(buffer);
	fail_if(rc != ACHAT_RC_OK, "Failed to free the buffer, rc = %i", rc);
	mark_point();
}
END_TEST

START_TEST(tc_buffer_add2_lt_defaultsz)
{
	const char	append_buf1[]	= {'1', '2', '3', '4', '5'};
	const char	append_buf2[]	= {'6', '7', '8', '9', '0'};
	const int	sz_buf1		= sizeof(append_buf1);
	const int	sz_buf2		= sizeof(append_buf2);
	size_t		bsize;
	void		*buf;
	achat_rc	rc;

	rc = acc_bufferinit(buffer);
	fail_if(rc != ACHAT_RC_OK,
		"Failed to initialize buffer, rc = %i", rc);
	mark_point();

	/* Make sure length of append_buf1/2 is really smaller */
	/* than ACHAT_BUFFER_DEFAULTSZ */
	fail_if(sz_buf1 + sz_buf2 > ACHAT_BUFFER_DEFAULTSZ,
		"len(append_buf) > ACHAT_BUFFER_DEFAULTSZ");
	mark_point();

	rc = acc_bufferappend(buffer, append_buf1, sz_buf1);
	fail_if(rc != ACHAT_RC_OK,
		"Failed to append to buffer, rc = %i", rc);
	mark_point();

	rc = acc_bufferappend(buffer, append_buf2, sz_buf2);
	fail_if(rc != ACHAT_RC_OK,
		"Failed to append to buffer, rc = %i", rc);
	mark_point();

	bsize = acc_bufferlen(buffer);
	fail_if(bsize != sz_buf1 + sz_buf2,
		"Length of buffer is wrong, is %i but %i expected",
		bsize, sz_buf1 + sz_buf2);
	mark_point();

	buf = acc_bufferptr(buffer);
	fail_if(memcmp(buf, append_buf1, sz_buf1) != 0,
		"Unexpected content of buffer");
	fail_if(memcmp(buf + sz_buf1, append_buf2, sz_buf2) != 0,
		"Unexpected content of buffer");
	mark_point();

	rc = acc_bufferfree(buffer);
	fail_if(rc != ACHAT_RC_OK, "Failed to free the buffer, rc = %i", rc);
	mark_point();
}
END_TEST

START_TEST(tc_buffer_add_gt_defaultsz)
{
	int		*append_buf;
	int		sz_buf, i;
	size_t		bsize;
	void		*buf;
	achat_rc	rc;

	rc = acc_bufferinit(buffer);
	fail_if(rc != ACHAT_RC_OK,
		"Failed to initialize buffer, rc = %i", rc);
	mark_point();

	/* Initialize append_buf with a size > ACHAT_BUFFER_DEFAULTSZ */
	sz_buf = ACHAT_BUFFER_DEFAULTSZ + 1;
	append_buf = calloc(sz_buf, sizeof(int));
	for (i = 0; i < sz_buf; i++)
		append_buf[i] = i;
	mark_point();

	rc = acc_bufferappend(buffer, append_buf, sz_buf);
	fail_if(rc != ACHAT_RC_OK,
		"Failed to append to buffer, rc = %i", rc);
	mark_point();

	bsize = acc_bufferlen(buffer);
	fail_if(bsize != sz_buf,
		"Length of buffer is wrong, is %i but %i expected",
		bsize, sz_buf);
	mark_point();

	buf = acc_bufferptr(buffer);
	fail_if(memcmp(buf, append_buf, bsize) != 0,
		"Unexpected content of buffer");
	mark_point();

	rc = acc_bufferfree(buffer);
	fail_if(rc != ACHAT_RC_OK, "Failed to free the buffer, rc = %i", rc);
	mark_point();
}
END_TEST

START_TEST(tc_buffer_add2_gt_defaultsz)
{
	int		*append_buf1, *append_buf2;
	int		sz_buf1, sz_buf2, i;
	size_t		bsize;
	void		*buf;
	achat_rc	rc;

	rc = acc_bufferinit(buffer);
	fail_if(rc != ACHAT_RC_OK,
		"Failed to initialize buffer, rc = %i", rc);
	mark_point();

	/* Initialize append_buf1/2 with a size > ACHAT_BUFFER_DEFAULTSZ */
	sz_buf1 = ACHAT_BUFFER_DEFAULTSZ + 1;
	sz_buf2 = ACHAT_BUFFER_DEFAULTSZ * 1.1;
	append_buf1 = calloc(sz_buf1, sizeof(int));
	append_buf2 = calloc(sz_buf2, sizeof(int));
	for (i = 0; i < sz_buf1; i++)
		append_buf1[i] = i;
	for (i = 0; i < sz_buf2; i++)
		append_buf2[i] = i;
	mark_point();

	rc = acc_bufferappend(buffer, append_buf1, sz_buf1);
	fail_if(rc != ACHAT_RC_OK,
		"Failed to append to buffer, rc = %i", rc);
	mark_point();

	rc = acc_bufferappend(buffer, append_buf2, sz_buf2);
	fail_if(rc != ACHAT_RC_OK,
		"Failed to append to buffer, rc = %i", rc);
	mark_point();

	bsize = acc_bufferlen(buffer);
	fail_if(bsize != sz_buf1 + sz_buf2,
		"Length of buffer is wrong, is %i but %i expected",
		bsize, sz_buf1 + sz_buf2);
	mark_point();

	buf = acc_bufferptr(buffer);
	fail_if(memcmp(buf, append_buf1, sz_buf1) != 0,
		"Unexpected content of buffer");
	fail_if(memcmp(buf + sz_buf1, append_buf2, sz_buf2) != 0,
		"Unexpected content of buffer");
	mark_point();

	rc = acc_bufferfree(buffer);
	fail_if(rc != ACHAT_RC_OK, "Failed to free the buffer, rc = %i", rc);
	mark_point();
}
END_TEST

START_TEST(tc_buffer_add_lt_gt_defaultsz)
{
	const char	append_buf1[]	= {'1', '2', '3', '4', '5'};
	int		*append_buf2;
	const int	sz_buf1		= sizeof(append_buf1);
	int		sz_buf2;
	int		i;
	size_t		bsize;
	void		*buf;
	achat_rc	rc;

	rc = acc_bufferinit(buffer);
	fail_if(rc != ACHAT_RC_OK,
		"Failed to initialize buffer, rc = %i", rc);
	mark_point();

	/* Make sure length of append_buf1 is really smaller */
	/* than ACHAT_BUFFER_DEFAULTSZ */
	fail_if(sz_buf1 > ACHAT_BUFFER_DEFAULTSZ,
		"len(append_buf) > ACHAT_BUFFER_DEFAULTSZ");
	mark_point();

	/* Initialize append_buf2 with a size > ACHAT_BUFFER_DEFAULTSZ */
	sz_buf2 = ACHAT_BUFFER_DEFAULTSZ + 1;
	append_buf2 = calloc(sz_buf2, sizeof(int));
	for (i = 0; i < sz_buf2; i++)
		append_buf2[i] = i;
	mark_point();

	rc = acc_bufferappend(buffer, append_buf1, sz_buf1);
	fail_if(rc != ACHAT_RC_OK,
		"Failed to append to buffer, rc = %i", rc);
	mark_point();

	rc = acc_bufferappend(buffer, append_buf2, sz_buf2);
	fail_if(rc != ACHAT_RC_OK,
		"Failed to append to buffer, rc = %i", rc);
	mark_point();

	bsize = acc_bufferlen(buffer);
	fail_if(bsize != sz_buf1 + sz_buf2,
		"Length of buffer is wrong, is %i but %i expected",
		bsize, sz_buf1 + sz_buf2);
	mark_point();

	buf = acc_bufferptr(buffer);
	fail_if(memcmp(buf, append_buf1, sz_buf1) != 0,
		"Unexpected content of buffer");
	mark_point();

	fail_if(memcmp(buf + sz_buf1, append_buf2, sz_buf2) != 0,
		"Unexpected content of buffer");
	mark_point();

	rc = acc_bufferfree(buffer);
	fail_if(rc != ACHAT_RC_OK, "Failed to free the buffer, rc = %i", rc);
	mark_point();
}
END_TEST

START_TEST(tc_buffer_clear)
{
	const char	append_buf[] = {'1', '2', '3', '4', '5'};
	size_t		bsize;
	achat_rc	rc;

	rc = acc_bufferinit(buffer);
	fail_if(rc != ACHAT_RC_OK,
		"Failed to initialize buffer, rc = %i", rc);
	mark_point();

	/* Make sure length of append_buf is really smaller */
	/* than ACHAT_BUFFER_DEFAULTSZ */
	fail_if(sizeof(append_buf) > ACHAT_BUFFER_DEFAULTSZ,
		"len(append_buf) > ACHAT_BUFFER_DEFAULTSZ");
	mark_point();

	rc = acc_bufferappend(buffer, append_buf, sizeof(append_buf));
	fail_if(rc != ACHAT_RC_OK,
		"Failed to append to buffer, rc = %i", rc);
	mark_point();

	rc = acc_bufferclear(buffer);
	fail_if(rc != ACHAT_RC_OK,
		"Failed to clear the buffer, rc = %i", rc);

	bsize = acc_bufferlen(buffer);
	fail_if(bsize != 0,
		"Length of buffer is wrong, is %i but 0 expected", bsize);
	mark_point();

	rc = acc_bufferfree(buffer);
	fail_if(rc != ACHAT_RC_OK, "Failed to free the buffer, rc = %i", rc);
	mark_point();
}
END_TEST

START_TEST(tc_buffer_consume_all)
{
	const char	append_buf[] = {'1', '2', '3', '4', '5'};
	size_t		bsize;
	achat_rc	rc;

	rc = acc_bufferinit(buffer);
	fail_if(rc != ACHAT_RC_OK,
		"Failed to initialize buffer, rc = %i", rc);
	mark_point();

	/* Make sure length of append_buf is really smaller */
	/* than ACHAT_BUFFER_DEFAULTSZ */
	fail_if(sizeof(append_buf) > ACHAT_BUFFER_DEFAULTSZ,
		"len(append_buf) > ACHAT_BUFFER_DEFAULTSZ");
	mark_point();

	rc = acc_bufferappend(buffer, append_buf, sizeof(append_buf));
	fail_if(rc != ACHAT_RC_OK,
		"Failed to append to buffer, rc = %i", rc);
	mark_point();

	rc = acc_bufferconsume(buffer, sizeof(append_buf));
	fail_if(rc != ACHAT_RC_OK,
		"Failed to remove from buffer, rc = %i", rc);

	bsize = acc_bufferlen(buffer);
	fail_if(bsize != 0,
		"Length of buffer is wrong, is %i but 0 expected", bsize);
	mark_point();

	rc = acc_bufferfree(buffer);
	fail_if(rc != ACHAT_RC_OK, "Failed to free the buffer, rc = %i", rc);
	mark_point();
}
END_TEST

START_TEST(tc_buffer_consume_partial)
{
	const char	append_buf[] = {'1', '2', '3', '4', '5'};
	size_t		bsize;
	void		*buf;
	achat_rc	rc;

	rc = acc_bufferinit(buffer);
	fail_if(rc != ACHAT_RC_OK,
		"Failed to initialize buffer, rc = %i", rc);
	mark_point();

	/* Make sure length of append_buf is really smaller */
	/* than ACHAT_BUFFER_DEFAULTSZ */
	fail_if(sizeof(append_buf) > ACHAT_BUFFER_DEFAULTSZ,
		"len(append_buf) > ACHAT_BUFFER_DEFAULTSZ");
	mark_point();

	rc = acc_bufferappend(buffer, append_buf, sizeof(append_buf));
	fail_if(rc != ACHAT_RC_OK,
		"Failed to append to buffer, rc = %i", rc);
	mark_point();

	rc = acc_bufferconsume(buffer, 2);
	fail_if(rc != ACHAT_RC_OK,
		"Failed to remove from buffer, rc = %i", rc);

	bsize = acc_bufferlen(buffer);
	fail_if(bsize != sizeof(append_buf) - 2,
		"Length of buffer is wrong, is %i but %i expected",
		bsize, sizeof(append_buf) - 2);
	mark_point();

	buf = acc_bufferptr(buffer);
	fail_if(memcmp(buf, append_buf + 2, bsize) != 0,
		"Unexpected content of buffer");
	mark_point();

	rc = acc_bufferfree(buffer);
	fail_if(rc != ACHAT_RC_OK, "Failed to free the buffer, rc = %i", rc);
	mark_point();
}
END_TEST

START_TEST(tc_buffer_consume_more)
{
	const char	append_buf[] = {'1', '2', '3', '4', '5'};
	size_t		bsize;
	void		*buf;
	achat_rc	rc;

	rc = acc_bufferinit(buffer);
	fail_if(rc != ACHAT_RC_OK,
		"Failed to initialize buffer, rc = %i", rc);
	mark_point();

	/* Make sure length of append_buf is really smaller */
	/* than ACHAT_BUFFER_DEFAULTSZ */
	fail_if(sizeof(append_buf) > ACHAT_BUFFER_DEFAULTSZ,
		"len(append_buf) > ACHAT_BUFFER_DEFAULTSZ");
	mark_point();

	rc = acc_bufferappend(buffer, append_buf, sizeof(append_buf));
	fail_if(rc != ACHAT_RC_OK,
		"Failed to append to buffer, rc = %i", rc);
	mark_point();

	rc = acc_bufferconsume(buffer, sizeof(append_buf) + 1);
	fail_if(rc != ACHAT_RC_ERROR, "An error was expected, rc = %i", rc);

	bsize = acc_bufferlen(buffer);
	fail_if(bsize != sizeof(append_buf),
		"Length of buffer is wrong, is %i but %i expected",
		bsize, sizeof(append_buf));
	mark_point();

	buf = acc_bufferptr(buffer);
	fail_if(memcmp(buf, append_buf, bsize) != 0,
		"Unexpected content of buffer");
	mark_point();

	rc = acc_bufferfree(buffer);
	fail_if(rc != ACHAT_RC_OK, "Failed to free the buffer, rc = %i", rc);
	mark_point();
}
END_TEST

START_TEST(tc_buffer_consume_empty)
{
	size_t		bsize;
	achat_rc	rc;

	rc = acc_bufferinit(buffer);
	fail_if(rc != ACHAT_RC_OK,
		"Failed to initialize buffer, rc = %i", rc);
	mark_point();

	rc = acc_bufferconsume(buffer, 42);
	fail_if(rc != ACHAT_RC_ERROR, "An error was expected, rc = %i", rc);

	bsize = acc_bufferlen(buffer);
	fail_if(bsize != 0,
		"Length of buffer is wrong, is %i but 0 expected", bsize);
	mark_point();

	rc = acc_bufferfree(buffer);
	fail_if(rc != ACHAT_RC_OK, "Failed to free the buffer, rc = %i", rc);
	mark_point();
}
END_TEST

START_TEST(tc_buffer_add_consume_add)
{
	const char	append_buf1[]	= {'1', '2', '3', '4', '5'};
	const char	append_buf2[]	= {'6', '7', '8', '9', '0'};
	const int	sz_buf1		= sizeof(append_buf1);
	const int	sz_buf2		= sizeof(append_buf2);
	size_t		bsize;
	void		*buf;
	achat_rc	rc;

	rc = acc_bufferinit(buffer);
	fail_if(rc != ACHAT_RC_OK,
		"Failed to initialize buffer, rc = %i", rc);
	mark_point();

	/* Make sure length of append_buf1/2 is really smaller */
	/* than ACHAT_BUFFER_DEFAULTSZ */
	fail_if(sz_buf1 + sz_buf2 > ACHAT_BUFFER_DEFAULTSZ,
		"len(append_buf) > ACHAT_BUFFER_DEFAULTSZ");
	mark_point();

	rc = acc_bufferappend(buffer, append_buf1, sz_buf1);
	fail_if(rc != ACHAT_RC_OK,
		"Failed to append to buffer, rc = %i", rc);
	mark_point();

	rc = acc_bufferconsume(buffer, 4);
	fail_if(rc != ACHAT_RC_OK,
		"Failed to remove from buffer, rc = %i", rc);

	rc = acc_bufferappend(buffer, append_buf2, sz_buf2);
	fail_if(rc != ACHAT_RC_OK,
		"Failed to append to buffer, rc = %i", rc);
	mark_point();

	bsize = acc_bufferlen(buffer);
	fail_if(bsize != sz_buf1 + sz_buf2 - 4,
		"Length of buffer is wrong, is %i but %i expected",
		bsize, sz_buf1 + sz_buf2);
	mark_point();

	buf = acc_bufferptr(buffer);
	fail_if(memcmp(buf, append_buf1 + 4, sz_buf1 - 4) != 0,
		"Unexpected content of buffer");
	fail_if(memcmp(buf + sz_buf1 - 4, append_buf2, sz_buf2) != 0,
		"Unexpected content of buffer");
	mark_point();

	rc = acc_bufferfree(buffer);
	fail_if(rc != ACHAT_RC_OK, "Failed to free the buffer, rc = %i", rc);
	mark_point();
}
END_TEST

START_TEST(tc_buffer_make_space_empty)
{
	achat_rc	rc;
	void		*p;
	size_t		bsize;

	rc = acc_bufferinit(buffer);
	fail_if(rc != ACHAT_RC_OK,
		"Failed to initialize buffer, rc = %i", rc);
	mark_point();

	bsize = acc_bufferlen(buffer);
	fail_if(bsize != 0, "Unexpected bufferlen. Is %i but should be 0",
		bsize);

	p = acc_bufferappend_space(buffer, 5);
	fail_if(p == NULL, "Failed to append some space to buffer");

	bsize = acc_bufferlen(buffer);
	fail_if(bsize != 5, "Unexpected bufferlen. Is %i but should be 0",
		bsize);

	rc = acc_bufferfree(buffer);
	fail_if(rc != ACHAT_RC_OK, "Failed to free the buffer, rc = %i", rc);
	mark_point();
}
END_TEST

START_TEST(tc_buffer_make_space_nonempty)
{
	const char	append_buf[] = {'1', '2', '3', '4', '5'};
	achat_rc	rc;
	void		*p;
	size_t		bsize;

	rc = acc_bufferinit(buffer);
	fail_if(rc != ACHAT_RC_OK,
		"Failed to initialize buffer, rc = %i", rc);
	mark_point();

	bsize = acc_bufferlen(buffer);
	fail_if(bsize != 0, "Unexpected bufferlen. Is %i but should be 0",
		bsize);

	rc = acc_bufferappend(buffer, append_buf, sizeof(append_buf));
	fail_if(rc != ACHAT_RC_OK,
		"Failed to append to buffer, rc = %i", rc);
	mark_point();

	bsize = acc_bufferlen(buffer);
	fail_if(bsize != sizeof(append_buf),
		"Length of buffer is wrong, is %i but %i expected",
		bsize, sizeof(append_buf));
	mark_point();

	p = acc_bufferappend_space(buffer, 5);
	fail_if(p == NULL, "Failed to append some space to buffer");

	bsize = acc_bufferlen(buffer);
	fail_if(bsize != sizeof(append_buf) + 5,
		"Unexpected bufferlen. Is %i but should be %i",
		bsize, sizeof(append_buf) + 5);

	rc = acc_bufferfree(buffer);
	fail_if(rc != ACHAT_RC_OK, "Failed to free the buffer, rc = %i", rc);
	mark_point();
}
END_TEST

START_TEST(tc_buffer_trunc_all)
{
	const char	append_buf[] = {'1', '2', '3', '4', '5'};
	size_t		bsize;
	achat_rc	rc;

	rc = acc_bufferinit(buffer);
	fail_if(rc != ACHAT_RC_OK,
		"Failed to initialize buffer, rc = %i", rc);
	mark_point();

	/* Make sure length of append_buf is really smaller */
	/* than ACHAT_BUFFER_DEFAULTSZ */
	fail_if(sizeof(append_buf) > ACHAT_BUFFER_DEFAULTSZ,
		"len(append_buf) > ACHAT_BUFFER_DEFAULTSZ");
	mark_point();

	rc = acc_bufferappend(buffer, append_buf, sizeof(append_buf));
	fail_if(rc != ACHAT_RC_OK,
		"Failed to append to buffer, rc = %i", rc);
	mark_point();

	bsize = acc_bufferlen(buffer);
	fail_if(bsize != sizeof(append_buf),
		"Length of buffer is wrong, is %i but %i expected",
		bsize, sizeof(append_buf));
	mark_point();

	rc = acc_buffertrunc(buffer, sizeof(append_buf));
	fail_if(rc != ACHAT_RC_OK, "Failed to truncate buffer: %i", rc);

	bsize = acc_bufferlen(buffer);
	fail_if(bsize != 0,
		"Length of buffer is wrong, is %i but 0 expected", bsize);

	rc = acc_bufferfree(buffer);
	fail_if(rc != ACHAT_RC_OK, "Failed to free the buffer, rc = %i", rc);
	mark_point();
}
END_TEST

START_TEST(tc_buffer_trunc_part)
{
	const char	append_buf[] = {'1', '2', '3', '4', '5'};
	size_t		bsize;
	char		*buf;
	achat_rc	rc;

	rc = acc_bufferinit(buffer);
	fail_if(rc != ACHAT_RC_OK,
		"Failed to initialize buffer, rc = %i", rc);
	mark_point();

	/* Make sure length of append_buf is really smaller */
	/* than ACHAT_BUFFER_DEFAULTSZ */
	fail_if(sizeof(append_buf) > ACHAT_BUFFER_DEFAULTSZ,
		"len(append_buf) > ACHAT_BUFFER_DEFAULTSZ");
	mark_point();

	rc = acc_bufferappend(buffer, append_buf, sizeof(append_buf));
	fail_if(rc != ACHAT_RC_OK,
		"Failed to append to buffer, rc = %i", rc);
	mark_point();

	bsize = acc_bufferlen(buffer);
	fail_if(bsize != sizeof(append_buf),
		"Length of buffer is wrong, is %i but %i expected",
		bsize, sizeof(append_buf));
	mark_point();

	rc = acc_buffertrunc(buffer, 3);
	fail_if(rc != ACHAT_RC_OK, "Failed to truncate buffer: %i", rc);

	bsize = acc_bufferlen(buffer);
	fail_if(bsize != sizeof(append_buf) - 3,
		"Length of buffer is wrong, is %i but %i expected",
		bsize, sizeof(append_buf) - 3);

	buf = acc_bufferptr(buffer);
	fail_if(memcmp(buf, append_buf, bsize) != 0,
		"Unexpected content of buffer");

	rc = acc_bufferfree(buffer);
	fail_if(rc != ACHAT_RC_OK, "Failed to free the buffer, rc = %i", rc);
	mark_point();
}
END_TEST

START_TEST(tc_buffer_trunc_more)
{
	const char	append_buf[] = {'1', '2', '3', '4', '5'};
	size_t		bsize;
	char		*buf;
	achat_rc	rc;

	rc = acc_bufferinit(buffer);
	fail_if(rc != ACHAT_RC_OK,
		"Failed to initialize buffer, rc = %i", rc);
	mark_point();

	/* Make sure length of append_buf is really smaller */
	/* than ACHAT_BUFFER_DEFAULTSZ */
	fail_if(sizeof(append_buf) > ACHAT_BUFFER_DEFAULTSZ,
		"len(append_buf) > ACHAT_BUFFER_DEFAULTSZ");
	mark_point();

	rc = acc_bufferappend(buffer, append_buf, sizeof(append_buf));
	fail_if(rc != ACHAT_RC_OK,
		"Failed to append to buffer, rc = %i", rc);
	mark_point();

	bsize = acc_bufferlen(buffer);
	fail_if(bsize != sizeof(append_buf),
		"Length of buffer is wrong, is %i but %i expected",
		bsize, sizeof(append_buf));
	mark_point();

	rc = acc_buffertrunc(buffer, sizeof(append_buf) + 1);
	fail_if (rc != ACHAT_RC_ERROR,
		"Unexpected result from truncate: %i", rc);

	bsize = acc_bufferlen(buffer);
	fail_if(bsize != sizeof(append_buf),
		"Length of buffer is wrong, is %i but %i expected",
		bsize, sizeof(append_buf));

	buf = acc_bufferptr(buffer);
	fail_if(memcmp(buf, append_buf, bsize) != 0,
		"Unexpected content of buffer");

	rc = acc_bufferfree(buffer);
	fail_if(rc != ACHAT_RC_OK, "Failed to free the buffer, rc = %i", rc);
	mark_point();
}
END_TEST

START_TEST(tc_buffer_trunc_empty)
{
	size_t		bsize;
	achat_rc	rc;

	rc = acc_bufferinit(buffer);
	fail_if(rc != ACHAT_RC_OK,
		"Failed to initialize buffer, rc = %i", rc);
	mark_point();

	bsize = acc_bufferlen(buffer);
	fail_if(bsize != 0,
		"Length of buffer is wrong, is %i but 0 expected",
		bsize);
	mark_point();

	rc = acc_buffertrunc(buffer, 42);
	fail_if(rc != ACHAT_RC_ERROR,
		"Unexpected result from truncate: %i", rc);

	bsize = acc_bufferlen(buffer);
	fail_if(bsize != 0,
		"Length of buffer is wrong, is %i but 0 expected",
		bsize);
	mark_point();

	rc = acc_bufferfree(buffer);
	fail_if(rc != ACHAT_RC_OK, "Failed to free the buffer, rc = %i", rc);
	mark_point();
}
END_TEST

START_TEST(tc_buffer_add_consume_trunc)
{
	const char	append_buf[] = {'1', '2', '3', '4', '5'};
	size_t		bsize;
	void		*buf;
	achat_rc	rc;

	rc = acc_bufferinit(buffer);
	fail_if(rc != ACHAT_RC_OK,
		"Failed to initialize buffer, rc = %i", rc);
	mark_point();

	/* Make sure length of append_buf is really smaller */
	/* than ACHAT_BUFFER_DEFAULTSZ */
	fail_if(sizeof(append_buf) > ACHAT_BUFFER_DEFAULTSZ,
		"len(append_buf) > ACHAT_BUFFER_DEFAULTSZ");
	mark_point();

	rc = acc_bufferappend(buffer, append_buf, sizeof(append_buf));
	fail_if(rc != ACHAT_RC_OK,
		"Failed to append to buffer, rc = %i", rc);
	mark_point();

	bsize = acc_bufferlen(buffer);
	fail_if(bsize != sizeof(append_buf),
		"Length of buffer is wrong, is %i but %i expected",
		bsize, sizeof(append_buf));
	mark_point();

	buf = acc_bufferptr(buffer);
	fail_if(memcmp(buf, append_buf, bsize) != 0,
		"Unexpected content of buffer");
	mark_point();

	rc = acc_bufferconsume(buffer, 1);
	fail_if(rc != ACHAT_RC_OK, "Failed to consume from buffer: %i", rc);

	rc = acc_buffertrunc(buffer, 2);
	fail_if(rc != ACHAT_RC_OK, "Failed to truncate buffer: %i", rc);

	bsize = acc_bufferlen(buffer);
	fail_if(bsize != sizeof(append_buf) - 3,
		"Length of buffer is wrong, is %i but %i expected",
		bsize, sizeof(append_buf) - 3);

	buf = acc_bufferptr(buffer);
	fail_if(memcmp(buf, append_buf + 1, bsize) != 0,
		"Unexpected content of buffer");
	mark_point();

	rc = acc_bufferfree(buffer);
	fail_if(rc != ACHAT_RC_OK, "Failed to free the buffer, rc = %i", rc);
	mark_point();
}
END_TEST

TCase *
libanoubischat_testcase_buffer(void)
{
	/* Buffer test case */
	TCase *tc_buffer = tcase_create("Buffer");
	tcase_add_checked_fixture(tc_buffer,
		tc_buffer_setup, tc_buffer_teardown);

	tcase_add_test(tc_buffer, tc_buffer_initrelease);
	tcase_add_test(tc_buffer, tc_buffer_add_lt_defaultsz);
	tcase_add_test(tc_buffer, tc_buffer_add2_lt_defaultsz);
	tcase_add_test(tc_buffer, tc_buffer_add_gt_defaultsz);
	tcase_add_test(tc_buffer, tc_buffer_add2_gt_defaultsz);
	tcase_add_test(tc_buffer, tc_buffer_add_lt_gt_defaultsz);
	tcase_add_test(tc_buffer, tc_buffer_clear);
	tcase_add_test(tc_buffer, tc_buffer_consume_all);
	tcase_add_test(tc_buffer, tc_buffer_consume_partial);
	tcase_add_test(tc_buffer, tc_buffer_consume_more);
	tcase_add_test(tc_buffer, tc_buffer_consume_empty);
	tcase_add_test(tc_buffer, tc_buffer_add_consume_add);
	tcase_add_test(tc_buffer, tc_buffer_make_space_empty);
	tcase_add_test(tc_buffer, tc_buffer_make_space_nonempty);
	tcase_add_test(tc_buffer, tc_buffer_trunc_all);
	tcase_add_test(tc_buffer, tc_buffer_trunc_part);
	tcase_add_test(tc_buffer, tc_buffer_trunc_more);
	tcase_add_test(tc_buffer, tc_buffer_trunc_empty);
	tcase_add_test(tc_buffer, tc_buffer_add_consume_trunc);

	return (tc_buffer);
}
