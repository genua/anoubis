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
#include "config.h"
#endif

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <anoubis_errno.h>
#include "anoubisd.h"
#include "amsg.h"
#include "cfg.h"
#include "anoubis_alloc.h"

#ifdef LINUX
#include <bsdcompat.h>
#endif

/**
 * \file
 * This file handles both command line and config file parsing.
 * The command line is parsed once at program startup and errors
 * cause the program to exit.
 *
 * The configuration file can be re-read (e.g. after a HUP signal is
 * received). However, only the master process has access to the
 * modified configuration file in this case. Thus it must parse the
 * config and sent the result to the other daemon processes. Functions
 * to help with this task are included in this file, too.
 */

/**
 * This enum list the numeric values of the keys that are allowed
 * in the configuration file.
 */
typedef enum {
	key_bad,			/** Invalid key */
	key_unixsocket,
	key_upgrade_mode,
	key_upgrade_trigger,
	key_rootkey,
	key_rootkey_required,
	key_auth_mode,
	key_coredumps,
	key_policysize,
	key_commit,
	key_scantimeout,
} cfg_key;


/**
 * This structure is ued to build lists of string valued keys that
 * are associated with integer values. The function @see name_to_value
 * can be used to find the string value of a key in an array of keys.
 */
struct stringkey {
	/** The string name of the key */
	const char	*name;

	/** The numeric value of the key */
	long		 value;
};

/**
 * This array associates each key that is allowed in the configuration file
 * with its corresponding numeric value. Only key_bad is not associated with
 * a key string.
 *
 * The array is terminated with an entry that uses NULL for the key string.
 */
static const struct stringkey		keywords[] = {
	{ "unixsocket", key_unixsocket },
	{ "upgrade_mode", key_upgrade_mode },
	{ "upgrade_trigger", key_upgrade_trigger },
	{ "rootkey", key_rootkey },
	{ "rootkey_required", key_rootkey_required },
	{ "auth_mode", key_auth_mode } ,
	{ "allow_coredumps", key_coredumps } ,
	{ "policysize", key_policysize },
	{ "commit", key_commit },
	{ "scanner_timeout", key_scantimeout },
	{ NULL, key_bad }
};

/**
 * This array associates string values of upgrade modes with their
 * respective numeric values. The value -1 indicates an invalid string.
 */
static const struct stringkey		upgrademodes[] = {
	{ "off", ANOUBISD_UPGRADE_MODE_OFF },
	{ "strictlock", ANOUBISD_UPGRADE_MODE_STRICT_LOCK },
	{ "looselock", ANOUBISD_UPGRADE_MODE_LOOSE_LOCK },
	{ "process", ANOUBISD_UPGRADE_MODE_PROCESS },
	{ NULL, -1 },
};

/**
 * This array associates string value for auth modes with their
 * respective numeric values. The value -1 indicates an invalid string.
 */
static const struct stringkey		authmodes[] = {
	{ "enabled", ANOUBISD_AUTH_MODE_ENABLED },
	{ "optional", ANOUBISD_AUTH_MODE_OPTIONAL },
	{ "off", ANOUBISD_AUTH_MODE_OFF },
	{ NULL, -1 },
};

/**
 * This array associates string value for boolean values with their
 * respective numeric values. The value -1 indicates an invalid string.
 */
static const struct stringkey		boolvalues[] = {
	{ "yes", 1 },
	{ "true", 1 },
	{ "1", 1 },
	{ "no", 0 },
	{ "false", 0 },
	{ "0", 0 },
	{ "off", 0 },
	{ NULL, -1 },
};

/**
 * This structure represents a single uninterpreted configuration
 * parameter.
 */
struct cfg_param {
	/** The numeric value of the configuration key. */
	cfg_key			key;
	/** The string value associated with the key. */
	struct abuf_buffer	value;
};

/**
 * This variable stores global configuratioon options that are
 * read from the command line. This values always take precedence over
 * values in the configuration file, even if the configuration file
 * is re-read after a HUP signal.
 *
 * Fields:
 * conffile: The location of the configuration file.
 * optsocket: The path to the daemon socket.
 * opts: The global anoubis daemon options.
 */
static struct {
	char			*conffile;
	char			*optsocket;
} data_store = {
	.conffile = ANOUBISD_CONF,
};

/**
 * Print a usage message for anoubisd and exit.
 *
 * @return This function does exits with exit code 1.
 */
__dead static void
usage(void)
{
	extern char *__progname;

	fprintf(stderr, "usage: %s [-D <flags>] [-dn] [-f <conffile>]"
	    " [-s <socket> ]\n", __progname);
	exit(1);
}

/**
 * Remove leading and trailing whitespaces from a string. The caller must
 * allocate the string string might be modified by this function.
 *
 * @param string The original string.
 * @return A pointer to the same string with leading and trailing
 *     blanks removed.
 *
 * @note If the original string is allocated from the heap, the caller
 *     must save a pointer to the start of the memory area and free
 *     the memory using this pointer.
 */
static char *
strip_whitespaces(char *string)
{
	unsigned int i;
	char *s;

	if (string == NULL || *string == '\0')
		return string;

	/* Strip trailing whitespace */
	for (i = strlen(string) - 1; i > 0; i--) {
		if (!isspace(string[i]))
			break;
		string[i] = '\0';
	}

	/* Strip leading whitespaces */
	s = string;
	while (*s != 0 && isspace(*s))
		s++;

	return s;
}

/**
 * Search for the given string in the stringkey array and return the
 * numeric value associated with the key. The array of keys must be
 * terminated by an element with a NULL name. Its key value will be
 * returned if no matching keyword was found.
 *
 * @param keys The string key array.
 * @param keybuf The string key in an abuf_buffer (must be a NUL-terminated
 *     string).
 * @param lineno The line number in the configuration file (for error message)
 *     Use zero to inhibit printing of error messages.
 * @return The numeric value of the key.
 *
 * NOTE: There is no canonical return value for an invalid key. The value
 * NOTE: associated with the terminating NULL pointer is used instead and
 * NOTE: this value can be different for different key arrays.
 * NOTE: Most arrays use -1 for the error case, however, the keyword array
 * NOTE: uses key_bad instead.
 */
static long
name_to_value(const struct stringkey *keys, const struct abuf_buffer keybuf,
    int lineno)
{
	unsigned int	 i;
	const char	*key = abuf_tostr(keybuf, 0);

	for (i = 0; keys[i].name; i++) {
		if (key && strcasecmp(key, keys[i].name) == 0)
			return keys[i].value;
	}
	if (lineno > 0)
		log_warnx("line %d: Invalid keyword %s found in "
		    "configuration file", lineno, key);
	return keys[i].value;
}

/**
 * Search for the first name in the string list that is associated with
 * the given value and return a pointer to the string.
 *
 * @param keys The string key array.
 * @param value The value to search for.
 * @return The first name associated with the given value.
 *     NULL if no name with the given value was found.
 */
static const char *
value_to_name(const struct stringkey *keys, long value)
{
	unsigned int	 i;

	for (i=0; keys[i].name; i++) {
		if (keys[i].value == value)
			return keys[i].name;
	}
	return NULL;
}

/**
 * Initializes a cfg_param structure.
 *
 * @param param A pointer to the cfg_param structure.
 * @return None.
 */
static void
cfg_param_initialize(struct cfg_param *param)
{
	param->key = key_bad;
	param->value = ABUF_EMPTY;
}

/**
 * Resets the given cfg_param structure. Any memory associated with
 * the structure is freed.
 *
 * @param param A pointer to the cfg_param structure.
 * @return None.
 */
static void
cfg_param_reset(struct cfg_param *param)
{
	param->key = key_bad;
	abuf_free(param->value);
	param->value = ABUF_EMPTY;
}

/**
 * Append the string <code>value</code> to the parameter <code>param</code>.
 *
 * @param param An initalized cfg_param structure that contains a value that
 *     is a NULL terminated string.
 * @param value Another NULL terminated string that is appended to the value
 *     buffer by this function.
 * @return Zero in case of success, a negative error code in case of
 *     an error. In case of errors the memory associated with the
 *     parameter value is freed. The only possible error is ENOMEM.
 */
static int
cfg_param_addvalue(struct cfg_param *param, char *value)
{
	if (abuf_empty(param->value)) {
		size_t size = strlen(value) + 1;

		param->value = abuf_alloc(size);
		if (abuf_length(param->value) == 0)
			goto nomem;
		abuf_copy_tobuf(param->value, value, size);
	} else {
		size_t			oldlen = abuf_length(param->value);
		size_t			newlen = strlen(value);
		struct abuf_buffer	tmp;

		/* Include the NUL byte in the buffer! */
		tmp = abuf_open_frommem(value, newlen+1);
		param->value = abuf_realloc(param->value, oldlen + newlen);
		if (abuf_empty(param->value))
			goto nomem;
		/*
		 * Offset oldlen-1 will make sure that we override the
		 * terminating NUL byte. The NUL byte from tmp is copied
		 * to the param buffer.
		 */
		abuf_copy_part(param->value, oldlen-1, tmp, 0,  newlen+1);
	}
	return 0;
nomem:
	log_warnx("Error: Out of memory while parsing configuration file");
	return -ENOMEM;
}

/**
 * Fills the anoubisd_config.upgrade_trigger list with triggers
 * enumerated in the given value.
 *
 * @param valuebuf The buffer that contains the string with the upgrade
 *     triggers.
 * @param lineno The line number for error message.
 * @return True in case of success, false if an error occured. An
 *     error message is logged in case of an error.
 */
static int
cfg_parse_upgrade_trigger(struct abuf_buffer valuebuf, int lineno)
{
	struct anoubisd_upgrade_trigger	*trigger;
	char				*s = NULL, *orig_s;
	const char			*value;

	value = abuf_tostr(valuebuf, 0);
	if (value)
		s = strdup(value);
	if (s == NULL) {
		log_warnx("line %d: Out of memory", lineno);
		return 0;
	}

	/* The original pointer is need for free(3c) below. */
	orig_s = s;
	while (1) {
		/*
		 * Also split at whitespaces to detect whitespaces around the
		 * comma
		 */
		char *token = strsep(&s, ", \t");

		if (token == NULL) /* End of string reached */
			break;
		/* Ignore empty strings */
		if (*token == '\0')
			continue;

		trigger = malloc(sizeof(struct anoubisd_upgrade_trigger));
		if (trigger == NULL) {
			log_warnx("line %d: Out of memory", lineno);
			free(orig_s);
			return 0;
		}

		trigger->arg = strdup(token);
		if (trigger->arg == NULL) {
			log_warnx("line %d: Out of memory", lineno);
			free(trigger);
			free(orig_s);
			return -ENOMEM;
		}

		LIST_INSERT_HEAD(&anoubisd_config.upgrade_trigger,
		    trigger, entries);
	}

	free(orig_s);
	return 1;
}

/**
 * Adds the specified playground content scanner to the
 * anoubisd_config.pg_scanner.
 *
 * @param valuebuf The buffer that contains the string with the config value.
 * @param lineno The line number for error message.
 * @return True in case of success, false if an error occured. An
 *     error message is logged in case of an error.
 */
static int
cfg_parse_commit(struct abuf_buffer valuebuf, int lineno)
{
	struct anoubisd_pg_scanner  *scanner;
	const char		    *v;
	char			    *value = NULL;
	char			    *cur = NULL, *next;
	int			     param;
	int			    ret = 1;
	struct stat		    sbuf;
	const char		    *err_msg = NULL;
	static const char	    *err_oom = "Out of memory";
	static const char	    *err_syntax = "Syntax error, expected: "
	    "(required|recommended) <path_to_scanner> <description>";

	/* get working copy of value */
	v = abuf_tostr(valuebuf, 0);
	if (v) {
		value = strdup(v);
	}
	if (value == NULL) {
		log_warnx("line %d: Out of memory", lineno);
		return 0;
	}

	/* now process the value */
	scanner = malloc(sizeof(struct anoubisd_pg_scanner));
	bzero(scanner, sizeof(struct anoubisd_pg_scanner));

	/* parse three paras: required|recommended <path> <description> */
	next = value;
	param = 0;
	while (param<3) {
		if (next == NULL) {
			err_msg = err_syntax;
			ret = 0;
			break;
		}

		/* Note: 3rd param is the remaining string */
		if (param != 2) {
			/* terminate current parameter, find next parameter */
			cur = strsep(&next, " \t");

			if (*cur == 0) {
				/* empty token, ignore */
				continue;
			}
		}

		if (param == 0) {
			if (strcmp(cur, "required") == 0) {
				scanner->required = 1;
			} else if (strcmp(cur, "recommended") == 0) {
				scanner->required = 0;
			} else {
				err_msg = err_syntax;
				ret = 0;
				break;
			}
			param++;
		} else if (param == 1) {
			scanner->path = strdup(cur);
			if (!scanner->path) {
				err_msg = err_oom;
				ret = 0;
				break;
			}
			if (strcasecmp(scanner->path, "allow") != 0
			    && strcasecmp(scanner->path, "deny") != 0
			    && stat(scanner->path, &sbuf) != 0) {
				log_warn("invalid scanner '%s' (line %d)",
					scanner->path, lineno);
			}
			param++;
		} else if (param == 2) {
			scanner->description = strdup(strip_whitespaces(next));
			if (!scanner->description) {
				err_msg = err_oom;
				ret = 0;
				break;
			}
			if (*(scanner->description) == 0) {
				err_msg = err_syntax;
				ret = 0;
				break;
			}
			param++;
		}
	}

	if (ret) {
		/* success */
		CIRCLEQ_INSERT_TAIL(&anoubisd_config.pg_scanner,
		    scanner, link);
	} else {
		log_warnx("line %d: %s", lineno, err_msg);

		if (scanner->path)
			free(scanner->path);
		if (scanner->description)
			free(scanner->description);
		free(scanner);
	}

	free(value);
	return ret;
}

/**
 * Convert the string in the argument buffer to an integer value.
 * The value must be in the range between min and max (inclusive).
 *
 * @param value The argument buffer.
 * @param lineno The line number for error message.
 * @param min The minimal value allowed for the result.
 * @param max The maximal value allowed for the result.
 * @param intptr The result of the conversion is stored in the integer
 *     pointed to by this pointer.
 * @return True if parsing is successful, false in case of an error.
 *     If an error occurs, an error message is logged.
 */
static int
cfg_parse_int(struct abuf_buffer value, int lineno, int min, int max,
    int *intptr)
{
	const char	*str;
	char		*endptr;
	long		 result;

	str = abuf_tostr(value, 0);
	if (str == NULL) {
		log_warnx("line %d: Out of memory!", lineno);
		return 0;
	}
	result = strtol(str, &endptr, 10);

	if (endptr == str) {
		/* No digits found */
		log_warnx("line %d: no digits found", lineno);
		return 0;
	}

	if (*endptr != '\0') {
		/* Further characters found after number */
		log_warnx("line %d: further characters found after number",
				lineno);
		return 0;
	}

	/* Range-checking */
	if (result < min || result > max) {
		log_warnx("line %d: Number out of range, min: %d, max: %d",
				lineno, min, max);
		return 0;
	}

	*intptr = result;
	return 1;
}

/**
 * Interpret the buffer as a string and return a newly allocated duplicate
 * version of the string.
 *
 * @param value The string value.
 * @param lineno The line number for error messages.
 * @return The duplicated string or NULL if an error occred (out of memory).
 *     If an error occured, an error message is logged.
 */
static char *
cfg_parse_string(struct abuf_buffer value, int lineno)
{
	const char	*str = abuf_tostr(value, 0);
	char		*ret = NULL;

	if (str)
		ret = strdup(str);
	if (ret == NULL)
		log_warnx("line %d: Out of memory", lineno);
	return ret;
}

/**
 * Process a single configuration parameter. The current line number
 * in the configuration file is given by <code>lineno</code>.
 *
 * @param param The configuration parameter.
 * @param lineno The current line number in the configuration file
 *     (for error messages).
 * @return True if the parameter could be processed, false if an
 *     error occured. In case of an error, an appropriate error message
 *     is logged.
 */
static int
cfg_param_process(struct cfg_param *param, int lineno)
{
	long		tmp;

	switch (param->key) {
		case key_unixsocket:
			anoubisd_config.unixsocket = cfg_parse_string(
					param->value, lineno);
			if (anoubisd_config.unixsocket == NULL)
				return 0;
			break;
		case key_upgrade_mode:
			tmp = name_to_value(upgrademodes, param->value, lineno);
			if (tmp == -1)
				return 0;
			anoubisd_config.upgrade_mode = tmp;
			break;
		case key_upgrade_trigger:
			if (!cfg_parse_upgrade_trigger(param->value, lineno))
				return 0;
			break;
		case key_rootkey:
			anoubisd_config.rootkey = cfg_parse_string(
					param->value, lineno);
			if (anoubisd_config.rootkey == NULL)
				return 1;
			break;
		case key_rootkey_required:
			tmp = name_to_value(boolvalues, param->value, lineno);
			if (tmp == -1)
				return 0;
			anoubisd_config.rootkey_required = tmp;
			break;
		case key_auth_mode:
			tmp = name_to_value(authmodes, param->value, lineno);
			if (tmp == -1)
				return 0;
			anoubisd_config.auth_mode = tmp;
			break;
		case key_coredumps:
			tmp = name_to_value(boolvalues, param->value, lineno);
			if (tmp == -1)
				return 0;
			anoubisd_config.allow_coredumps = tmp;
			break;
		case key_policysize:
			if (!cfg_parse_int(param->value, lineno,
			    0, INT_MAX, &anoubisd_config.policysize))
				return 0;
			break;
		case key_commit:
			if (!cfg_parse_commit(param->value, lineno))
				return 0;
			break;

		case key_scantimeout:
			if (!cfg_parse_int(param->value, lineno, 0, INT_MAX,
			    &anoubisd_config.scanner_timeout))
				return 0;
			break;
		default:
			log_warnx("line %d: Internal error: "
			    "Bad key value %d", lineno, param->key);
			return 0;
	}
	return 1;
}

/**
 * Reads out the configuration from the given file handle. The results are
 * written into the anoubisd_config structure.
 *
 * ** Format of configuration-file **
 *
 * In general:
 *
 * <pre>
 * &lt;key&gt; = &lt;value&gt;
 * </pre>
 *
 * If a line end with a backslash, the value is continued in the next line.
 *
 * A complete line can be marked as a comment, when it starts with the
 * hash-key.
 *
 * Only keys specified in the keywords array are accepted. Otherwise an error
 * is raised. The format of the value highly depends on the key.
 *
 * @param f The file handle.
 * @return True in case of success, False if a parse error occured.
 */
static int
cfg_parse_file(FILE *f)
{
	struct cfg_param param;
	char buf[1024], *line;
	unsigned int lineno = 0;
	int key_expected = 1;
	int err_counter = 0;
	int result;

	cfg_param_initialize(&param);

	while (fgets(buf, sizeof(buf), f)) {
		lineno++;
		line = buf;

		if (strlen(buf) >= (sizeof(buf) - 1)) {
			/* Max. size of line reached, but this is too much */
			log_warnx("line %d: Line too long", lineno);

			err_counter++;
			cfg_param_reset(&param);

			continue;
		}

		/* Strip leading and trailing whitespaces */
		line = strip_whitespaces(line);

		if (*line == '\0') {
			/* Nothing to do, continue with next line */
			continue;
		}

		if (*line == '#') {
			/* This is a comment, skip the line */
			continue;
		}

		if (key_expected) {
			/*
			 * Extract key from line.
			 * Sepatated with = from rest of line
			 */
			char *key, *pos;

			pos = strchr(line, '=');
			if (pos == NULL) {
				log_warnx("No key found in config file "
				    "line %d", lineno);
				err_counter++;
				cfg_param_reset(&param);
				continue;
			}
			/* Split key and value. */
			*pos = '\0';
			key = strip_whitespaces(line);
			line = strip_whitespaces(pos + 1);

			/* Parse the key */
			param.key = name_to_value(keywords,
			    abuf_open_frommem(key, strlen(key)+1), lineno);
			if (param.key == key_bad) {
				err_counter++;
				cfg_param_reset(&param);
				continue;
			}
		}

		if (line[strlen(line) - 1] == '\\') {
			/*
			 * Current line ends with an backslash, the value
			 * is continued in next line
			 */
			line[strlen(line) - 1] = '\0'; /* Remove backslash */

			result = cfg_param_addvalue(&param, line);
			if (result < 0)
				return 0;
			key_expected = 0; /* Continued, next line has no key */
		} else {
			/*
			 * No backslash at the end of the line,
			 * Key-value-pair completed
			 */
			result = cfg_param_addvalue(&param, line);
			if (result < 0)
				return 0;

			if (!cfg_param_process(&param, lineno))
				err_counter++;
			cfg_param_reset(&param);
			key_expected = 1; /* Next line starts with a key */
		}
	}

	if (param.key != key_bad) {
		/*
		 * There's still an unprocessed parameter! This can happen, if
		 * the last value is escaped with an backslash at the end of
		 * the line. Do not forget this parameter and process it now!
		 */
		if (!cfg_param_process(&param, lineno))
			err_counter++;
	}

	cfg_param_reset(&param);
	if (err_counter)
		return 0;
	return 1;
}

/**
 * Parse the anoubis Daemon command line and store the result in
 * the global data_store variable. The command line values are kept
 * there and copied to the anoubisd_conf structure only afer the
 * configuration file was parsed. This ensures that command line
 * options take precedence over settings in the configuration file
 * even if the configuration is reloaded.
 *
 * @param argc The number of elements in the anoubis argument vector.
 * @param argv The argument vector.
 * @return None. A usage error is logged and the process terminates if
 *     the command line options are invalid. Additional non-option argument
 *     are an error.
 */
static void
cfg_read_cmdline(int argc, char * const *argv)
{
	int	 ch;
	char	*endptr;

	ch = 0;
	endptr = NULL;

	while (ch != -1) {
		ch = getopt(argc, argv, "dD:ns:f:");
		switch (ch) {
		case -1:
			/* No arguments left. */
			break;
		case 'd':
			debug_stderr = 1;
			break;
		case 'D':
			if (!optarg) {
				/* make statical analyzers happy */
				early_errx("getopt error");
			}
			errno = 0;    /* To distinguish success/failure */
			debug_flags = strtol(optarg, &endptr, 0);
			if (errno) {
				perror("strtol");
				usage();
			}
			if (endptr == optarg) {
				fprintf(stderr, "No digits were found\n");
				usage();
			}
			break;
		case 'n':
			anoubisd_noaction = 1;
			break;
		case 's':
			if (!optarg) {
				/* make statical analyzers happy */
				early_errx("getopt error");
			}
			if (data_store.optsocket) {
				usage();
			}
			data_store.optsocket = optarg;
			break;
		case 'f':
			if (!optarg) {
				/* make statical analyzers happy */
				early_errx("getopt error");
			}
			data_store.conffile = optarg;
			break;
		default:
			usage();
			/*NOTREACHED*/
		}
	}
	if (optind != argc)
		usage();
}

/**
 * Reads out the configuration from the given file. The results are written
 * into the anoubisd_config structure.
 *
 * @param file The name of the file to read from.
 * @return True in case of success, false in case of an error.
 */
static int
cfg_read_file(const char *file)
{
	FILE *f;
	int result;

	if (file == NULL)
		return 0;

	f = fopen(file, "r");
	if (f == NULL) {
		log_warn("Cannot open config file");
		/* Report success (default configuration is used) */
		return 1;
	}

	result = cfg_parse_file(f);
	fclose(f);

	return result;
}

/**
 * Load default configuration values.
 *
 * @return True in case of success, false if an error (out of memory)
 *     occured. This function logs an error message in case of errors.
 */
static int
cfg_defaults(void)
{
	char *s;

	if (anoubisd_config.unixsocket == NULL) {
		s = strdup(PACKAGE_SOCKET);
		if (s == NULL) {
			log_warnx("Out of memory");
			return 0;
		} else {
			anoubisd_config.unixsocket = s;
		}
	}

	anoubisd_config.upgrade_mode = ANOUBISD_UPGRADE_MODE_OFF;
	anoubisd_config.auth_mode = ANOUBISD_AUTH_MODE_OPTIONAL;
	anoubisd_config.policysize = ANOUBISD_MAX_POLICYSIZE;
	anoubisd_config.scanner_timeout = 5*60; /* Five minutes */

	return 1;
}

/**
 * Initializes the global anoubisd_config structure and parses the
 * command line arguments. The command line arguments are stored in the
 * global variable data_store.
 *
 * @param argc The number of arguments in the argument vector.
 * @param argv The argument vector.
 * @return True in case of success, false if an error occred. In case
 *      of an error in the command line parsing a usage message is printed.
 *      Other errors will only log an error message and return false.
 */
int
cfg_initialize(int argc, char * const *argv)
{
	memset(&anoubisd_config, 0, sizeof(struct anoubisd_config));
	if (!cfg_defaults())
		return 0;
	LIST_INIT(&anoubisd_config.upgrade_trigger);
	CIRCLEQ_INIT(&anoubisd_config.pg_scanner);
	cfg_read_cmdline(argc, argv);

	return 1;
}

/**
 * Releases memory allocated for the given anoubisd_config structure.
 */
static void
cfg_clear(void)
{
	free(anoubisd_config.unixsocket);
	anoubisd_config.unixsocket = NULL;

	while (!LIST_EMPTY(&anoubisd_config.upgrade_trigger)) {
		struct anoubisd_upgrade_trigger *trigger;

		trigger = LIST_FIRST(&anoubisd_config.upgrade_trigger);
		LIST_REMOVE(trigger, entries);

		free(trigger->arg);
		free(trigger);
	}
	anoubisd_config.rootkey = NULL;
	anoubisd_config.rootkey_required = 0;
	anoubisd_config.allow_coredumps = 0;

	while (!CIRCLEQ_EMPTY(&anoubisd_config.pg_scanner)) {
		struct anoubisd_pg_scanner *scanner;

		scanner = CIRCLEQ_FIRST(&anoubisd_config.pg_scanner);
		CIRCLEQ_REMOVE(&anoubisd_config.pg_scanner, scanner, link);

		free(scanner->path);
		free(scanner->description);
		free(scanner);
	}
}

/**
 * Perform consistency checks on the configuration and warn if any
 * inconsistencies are found.
 *
 * @return True if the verification is successful, false if an inconsistency
 *     was found.
 */
static int
cfg_verify(void)
{
	if (anoubisd_config.rootkey_required) {
		if (!anoubisd_config.rootkey) {
			log_warnx("Configuration error: rootkey_required but "
			    "no rootkey");
			return 0;
		}
		if (anoubisd_config.upgrade_mode
		    == ANOUBISD_UPGRADE_MODE_PROCESS) {
			log_warnx("Configuration error: upgrade_mode "
			    "'process' is not support with rootkey_required");
			return 0;
		}
	}
	return 1;
}

/**
 * Read the configuration from the appropriate configuration file.
 * The origin of the configuration file may be the default value
 * a custom value provided on the command line.
 *
 * @return True if the configuration file could be read successfully,
 *     false if an error occured.
 */
int
cfg_read(void)
{
	if (!cfg_read_file(data_store.conffile))
		return 0;
	if (!cfg_verify())
		return 0;
	/* Command line options are more important than config file. */
	/* Transfer cmdline socket. */
	if (data_store.optsocket != NULL) {
		free(anoubisd_config.unixsocket);
		anoubisd_config.unixsocket = strdup(data_store.optsocket);
	}
	return 1;
}

/**
 * Clears the configuration and reads it again. This should be called
 * if a HUP signal is received in the master process.
 *
 * @return True in case of success, false if an error occured. An error
 *     message is logged in case of errors.
 */
int
cfg_reread(void)
{
	cfg_clear();
	if (!cfg_defaults())
		return 0;
	if (!cfg_read())
		return 0;

	return 1;
}

/**
 * Write the current configuration to the file handle.
 *
 * @param f The file handle.
 */
void
cfg_dump(FILE *f)
{
	struct anoubisd_upgrade_trigger *trigger;
	struct anoubisd_pg_scanner *scanner;

	fprintf(f, "conffile: %s\n", data_store.conffile);
	fprintf(f, "unixsocket: %s\n", anoubisd_config.unixsocket);
	fprintf(f, "upgrade_mode: %s\n",
	    value_to_name(upgrademodes, anoubisd_config.upgrade_mode));
	/* upgrade_trigger */
	if (!LIST_EMPTY(&anoubisd_config.upgrade_trigger)) {
		fprintf(f, "upgrade_trigger: ");
		LIST_FOREACH(trigger, &anoubisd_config.upgrade_trigger,
		    entries) {
			if (trigger->entries.le_next != NULL)
				fprintf(f, "%s, ", trigger->arg);
			else
				fprintf(f, "%s\n", trigger->arg);
		}
	} else
		fprintf(f, "upgrade_trigger:\n");
	if (anoubisd_config.rootkey) {
		fprintf(f, "rootkey: %s\n", anoubisd_config.rootkey);
		fprintf(f, "rootkey_required: %s\n",
		    anoubisd_config.rootkey_required?"true":"false");
	}
	fprintf(f, "auth_mode: %s\n",
	    value_to_name(authmodes, anoubisd_config.auth_mode));
	fprintf(f, "policysize: %i\n", anoubisd_config.policysize);

	/* playground scanners */
	CIRCLEQ_FOREACH(scanner, &anoubisd_config.pg_scanner, link) {
		char * required_str = NULL;
		if (scanner->required) {
			required_str = "required";
		} else {
			required_str = "recommended";
		}
		fprintf(f, "commit = %s %s %s\n", required_str,
		    scanner->path, scanner->description);
	}
}

/**
 * Create a message to send the current config to other anoubisd processes.
 * The configuration message consists of the following elements:
 * - An anoubisd_msg_config structure that contains the configuration data
 *   required by the policy engine that can be passed as integers and the
 *   number of upgrade triggers in the payload.
 * - The payload consists of count+1 NUL-terminated strings. The first
 *   string is the path to the daemon socket.
 * - The rest of the payload contains count NUL-terminated strings. Each
 *   of these strings is an upgrade trigger.
 *
 * @return A pointer to the message structure or NULL in case of an error
 *     (out of memory).
 */
struct anoubisd_msg *
cfg_msg_create(void)
{
	int	msgsize;
	int	count;
	int	currlen;
	int	len;

	struct anoubisd_msg			*msg;
	struct anoubisd_msg_config		*confmsg;
	struct anoubisd_upgrade_trigger		*trigger;

	/* Calculate needed message size. */
	count = 0;
	msgsize  = sizeof(struct anoubisd_msg_config);
	msgsize += strlen(anoubisd_config.unixsocket) + 1;
	LIST_FOREACH(trigger, &anoubisd_config.upgrade_trigger, entries) {
		msgsize += strlen(trigger->arg) + 1;
		count++;
	}

	/* Create message. */
	msg = msg_factory(ANOUBISD_MSG_CONFIG, msgsize);
	if (msg == NULL) {
		log_warnx("cfg_transmit: Out of memory");
		master_terminate();
		return (NULL);
	}
	confmsg = (struct anoubisd_msg_config *)msg->msg;

	confmsg->policysize = anoubisd_config.policysize;
	/* Fill message: upgrade mode. */
	confmsg->upgrade_mode = anoubisd_config.upgrade_mode;

	/* Fill message: unixsocket. */
	currlen = 0;
	len = strlen(anoubisd_config.unixsocket) + 1;
	memcpy(confmsg->chunk, anoubisd_config.unixsocket, len);
	currlen += len;

	/* Fill message: trigger list. */
	confmsg->triggercount = count;
	LIST_FOREACH(trigger, &anoubisd_config.upgrade_trigger, entries) {
		len = strlen(trigger->arg) + 1;
		memcpy(confmsg->chunk + currlen, trigger->arg, len);
		currlen += len;
	}

	/* Note: playground scanners are not distributed */

	return (msg);
}

/**
 * Handle a configuration message that was received from the anoubis
 * daemon master process. The configuration options stored in the
 * message replace the current daemon configuration. Any other action
 * that might be neccessary as a result of the modified configuration
 * are the responsibility of the caller.
 * @see cfg_msg_create for a description of the message format.
 *
 * @param msg The configuration message.
 * @return Zero in case of success, a negative error code in case of an
 *     error.
 */
int
cfg_msg_parse(struct anoubisd_msg *msg)
{
	int	count;
	int	offset;

	struct anoubisd_msg_config		*confmsg;
	struct anoubisd_upgrade_trigger		*trigger;

	confmsg = (struct anoubisd_msg_config *)msg->msg;

	cfg_clear();

	/* Extract upgrade mode. */
	anoubisd_config.upgrade_mode =
	    (anoubisd_upgrade_mode)confmsg->upgrade_mode;

	/* Extract unixsocket. */
	offset = strlen(confmsg->chunk) + 1;
	anoubisd_config.unixsocket = (char *)malloc(offset);
	if (anoubisd_config.unixsocket == NULL) {
		log_warnx("line %d: Out of memory", __LINE__);
		return (-ENOMEM);
	}
	memcpy(anoubisd_config.unixsocket, confmsg->chunk, offset);
	anoubisd_config.policysize = confmsg->policysize;

	/* Extract trigger list. */
	count = confmsg->triggercount;
	while (count > 0) {
		trigger = malloc(sizeof(struct anoubisd_upgrade_trigger));
		if (trigger == NULL) {
			log_warnx("%d: Out of memory", __LINE__);
			return (-ENOMEM);
		}

		trigger->arg = strdup(confmsg->chunk + offset);
		if (trigger->arg == NULL) {
			log_warnx("%d: Out of memory", __LINE__);
			free(trigger);
			return (-ENOMEM);
		}

		LIST_INSERT_HEAD(&anoubisd_config.upgrade_trigger,
		    trigger, entries);

		offset += strlen(trigger->arg) + 1;
		count--;
	}

	/* Note: playground scanners are not distributed */

	return (0);
}
