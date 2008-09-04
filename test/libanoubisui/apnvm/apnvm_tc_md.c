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

#include <check.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <apnmd.h>

START_TEST(md_tc_parse)
{
	const char *s = "<apnvm-metadata>\n\
key1:=value1\n\
	key2:=value2   \n\
 key3 := value3\n\
key4 += value4\n\
key1 += value1.1\n\
</apnvm-metadata>";
	apnmd	*md;
	char	*value;
	int	result;

	md = apnmd_parse(s);
	fail_if(md == NULL, "Failed to parse input-string");

	result = apnmd_count(md);
	fail_if(result != 4, "Unexpected number of keys: %i", result);

	value = apnmd_get(md, "key1");
	fail_if(value == NULL, "No value for \"key1\"");
	result = strcmp(value, "value1\nvalue1.1");
	fail_if(result != 0, "Unexpected value for \"key1\": %s", value);

	value = apnmd_get(md, "key2");
	fail_if(value == NULL, "No value for \"key2\"");
	result = strcmp(value, "value2");
	fail_if(result != 0, "Unexpected value for \"key2\": %s", value);

	value = apnmd_get(md, "key3");
	fail_if(value == NULL, "No value for \"key3\"");
	result = strcmp(value, "value3");
	fail_if(result != 0, "Unexpected value for \"key3\": %s", value);

	value = apnmd_get(md, "key4");
	fail_if(value == NULL, "No value for \"key4\"");
	result = strcmp(value, "value4");
	fail_if(result != 0, "Unexpected value for \"key4\": %s", value);

	value = apnmd_get(md, "key5");
	fail_if(value != NULL, "Value for \"key5\" found!");

	apnmd_destroy(md);
}
END_TEST

START_TEST(md_tc_parse_empty)
{
	apnmd	*md;
	int	result;

	md = apnmd_parse("");
	fail_if(md == NULL, "Failed to parse input-string");

	result = apnmd_count(md);
	fail_if(result != 0, "Unexpected number of keys: %i", result);

	apnmd_destroy(md);
}
END_TEST

START_TEST(md_tc_parse_null)
{
	apnmd	*md;
	int	result;

	md = apnmd_parse(NULL);
	fail_if(md == NULL, "Failed to parse input-string");

	result = apnmd_count(md);
	fail_if(result != 0, "Unexpected number of keys: %i", result);

	apnmd_destroy(md);
}
END_TEST

START_TEST(md_tc_parse_garbage)
{
	apnmd	*md;
	int	result;

	md = apnmd_parse("wefninfalknlkklkasddssadb  <s<yd");
	fail_if(md == NULL, "Failed to parse input-string");

	result = apnmd_count(md);
	fail_if(result != 0, "Unexpected number of keys: %i", result);

	apnmd_destroy(md);
}
END_TEST

START_TEST(md_tc_serialize)
{
	apnmd	*md;
	char	*result;
	const char *exp_result = "<apnvm-metadata>\n\
key2 := value2.1\n\
key2 += value2.2\n\
key2 += value2.3\n\
key1 := value1\n\
</apnvm-metadata>";

	md = apnmd_parse(NULL);
	fail_if(md == NULL, "Failed to parse input-string");

	apnmd_put(md, "key1", "value1");
	apnmd_put(md, "key2", "value2.1");
	apnmd_add(md, "key2", "value2.2");
	apnmd_add(md, "key2", "value2.3");

	result = apnmd_serialize(md);

	fail_if(strcmp(result, exp_result) != 0,
	    "Wrong serialization result:->%s<-", result);

	free(result);

	apnmd_destroy(md);
}
END_TEST

START_TEST(md_tc_get_int)
{
	const char *s = "<apnvm-metadata>\n\
key1 := 4711\n\
key2 := value2\n\
</apnvm-metadata>";
	apnmd	*md;
	int	result;

	md = apnmd_parse(s);
	fail_if(md == NULL, "Failed to parse input-string");

	result = apnmd_count(md);
	fail_if(result != 2, "Unexpected number of keys: %i", result);

	result = apnmd_get_int(md, "key1");
	fail_if(result != 4711, "Unexpected value for \"key1\"", result);

	result = apnmd_get_int(md, "key2");
	fail_if(result != 0, "Unexpected value for \"key2\"", result);

	result = apnmd_get_int(md, "key3");
	fail_if(result != 0, "Unexpected value for \"key3\"", result);

	apnmd_destroy(md);
}
END_TEST

START_TEST(md_tc_put)
{
	apnmd	*md;
	char	*value;
	int	result;

	md = apnmd_parse(NULL);

	apnmd_put(md, "key1", "value1");
	apnmd_put(md, "key2", "value2");
	apnmd_put(md, "key1", "value1.1");

	value = apnmd_get(md, "key1");
	fail_if(value == NULL, "No value for \"key1\"");
	result = strcmp(value, "value1.1");
	fail_if(result != 0, "Unexpected value for \"key1\": %s", value);

	value = apnmd_get(md, "key2");
	fail_if(value == NULL, "No value for \"key2\"");
	result = strcmp(value, "value2");
	fail_if(result != 0, "Unexpected value for \"key2\": %s", value);

	value = apnmd_get(md, "key3");
	fail_if(value != NULL, "Value for \"key3\" found");

	apnmd_destroy(md);
}
END_TEST

START_TEST(md_tc_put_int)
{
	apnmd	*md;
	char	*sval;
	int	result, ival;

	md = apnmd_parse(NULL);

	apnmd_put_int(md, "key1", 4711);
	apnmd_put_int(md, "key2", 4712);
	apnmd_put_int(md, "key1", 4713);

	sval = apnmd_get(md, "key1");
	fail_if(sval == NULL, "No value for \"key1\"");
	ival = apnmd_get_int(md, "key1");
	result = strcmp(sval, "4713");
	fail_if(result != 0, "Unexpected value for \"key1\": %s", sval);
	fail_if(ival != 4713, "Unexpected value for \"key1\": %s", ival);

	sval = apnmd_get(md, "key2");
	fail_if(sval == NULL, "No value for \"key2\"");
	ival = apnmd_get_int(md, "key2");
	result = strcmp(sval, "4712");
	fail_if(result != 0, "Unexpected value for \"key1\": %s", sval);
	fail_if(ival != 4712, "Unexpected value for \"key1\": %s", ival);

	apnmd_destroy(md);
}
END_TEST

START_TEST(md_tc_add)
{
	apnmd	*md;
	char	*value;
	int	result;

	md = apnmd_parse(NULL);

	apnmd_add(md, "key1", "value1");
	apnmd_put(md, "key2", "value2");
	apnmd_add(md, "key2", "value2.2");

	value = apnmd_get(md, "key1");
	fail_if(value == NULL, "No value for \"key1\"");
	result = strcmp(value, "value1");
	fail_if(result != 0, "Unexpected value for \"key1\": %s", value);

	value = apnmd_get(md, "key2");
	fail_if(value == NULL, "No value for \"key2\"");

	result = strcmp(value, "value2\nvalue2.2");
	fail_if(result != 0, "Unexpected value for \"key2\": %s", value);

	apnmd_destroy(md);
}
END_TEST

START_TEST(md_tc_add_int)
{
	apnmd	*md;
	int	result;
	char	*sval;
	int	ival;

	md = apnmd_parse(NULL);

	apnmd_add_int(md, "key1", 4711);
	apnmd_put(md, "key2", "value2");
	apnmd_add_int(md, "key2", 4712);

	sval = apnmd_get(md, "key1");
	ival = apnmd_get_int(md, "key1");
	fail_if(sval == NULL, "No value for \"key1\"");
	result = strcmp(sval, "4711");
	fail_if(result != 0, "Unexpected value for \"key1\": %s", sval);
	fail_if(ival != 4711, "Unexpected value for \"key1\": %s", ival);

	sval = apnmd_get(md, "key2");
	fail_if(sval == NULL, "No value for \"key2\"");

	result = strcmp(sval, "value2\n4712");
	fail_if(result != 0, "Unexpected value for \"key2\": %s", sval);

	apnmd_destroy(md);
}
END_TEST

TCase *
apnvm_tc_md(void)
{
	TCase *tc = tcase_create("MD");

	tcase_add_test(tc, md_tc_parse);
	tcase_add_test(tc, md_tc_parse_empty);
	tcase_add_test(tc, md_tc_parse_null);
	tcase_add_test(tc, md_tc_parse_garbage);
	tcase_add_test(tc, md_tc_serialize);
	tcase_add_test(tc, md_tc_get_int);
	tcase_add_test(tc, md_tc_put);
	tcase_add_test(tc, md_tc_put_int);
	tcase_add_test(tc, md_tc_add);
	tcase_add_test(tc, md_tc_add_int);

	return (tc);
}
