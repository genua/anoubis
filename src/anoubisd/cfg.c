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
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "anoubisd.h"
#include "cfg.h"

#ifdef LINUX
#include <bsdcompat.h>
#endif

typedef enum {
	key_bad,
	key_nooption,
	key_unixsocket,
	key_upgrade_mode,
	key_upgrade_trigger
} cfg_key;

static struct {
	const char *name;
	cfg_key key;
} keywords[] = {
	{ "unixsocket", key_unixsocket },
	{ "upgrade_mode", key_upgrade_mode },
	{ "upgrade_trigger", key_upgrade_trigger },
	{ NULL, key_bad }
};

struct cfg_param
{
	cfg_key key;
	char *value;
	size_t value_size;
};

struct cfg_data
{
	int			 cmdlineopts;
	char			*conffile;
	char			*optsocket;
};
static struct cfg_data data_store = {
	0,
	ANOUBISD_CONF,
	NULL,
};

/*@noreturn@*/
static void	 usage(void) __dead;
static char	*strip_whitespaces(char *);
static cfg_key	 parse_key(const char *);

static void cfg_param_initialize(struct cfg_param *);
static void cfg_param_clear(struct cfg_param *);
static void cfg_param_reset(struct cfg_param *);
static int  cfg_param_addvalue(struct cfg_param *, const char *, int);

static int cfg_parse_file(FILE *);
static int cfg_parse_upgrade_mode(const char *, int *, int);
static int cfg_parse_upgrade_trigger(const char *, int);
static int cfg_param_process(struct cfg_param *, int);

static int cfg_read_cmdline(int, char * const *);
static int cfg_read_file(const char *);

/*
 * Say how to use and exits.
 */
__dead static void
usage(void)
{
	extern char *__progname;

	fprintf(stderr, "usage: %s [-D <flags>] [-dnv] [-f <conffile>]"
	    " [-s <socket> ]\n", __progname);
	exit(1);
}

/*
 * Removes any leading and trailing whitespaces from the given string.
 * Note, that the string can be modified! A pointer to the modified
 * string is returned.
 */
static char *
strip_whitespaces(char *string)
{
	unsigned int i;
	char *s;

	if ((string == NULL) || (strlen(string) == 0)) {
		/* Nothing to do */
		return (string);
	}

	/* Strip trailing whitespace */
	for (i = strlen(string) - 1; i > 0; i--) {
		if (!isspace(string[i]))
			break;
		string[i] = '\0';
	}

	/* Strip leading whitespaces */
	s = string;
	while (*s != '\0') {
		if (isspace(*s))
			s++;
		else
			break;
	}

	return (s);
}

/*
 * Searches the keywords array for the given string. The assigned cfg_key is
 * returned. If the name is not registered, key_bad is returned.
 */
static cfg_key
parse_key(const char *s)
{
	unsigned int i;

	for (i = 0; keywords[i].name; i++) {
		if (strcasecmp(s, keywords[i].name) == 0)
			return keywords[i].key;
	}

	return (key_bad);
}

/* Initializes the given cfg_param structure */
static void
cfg_param_initialize(struct cfg_param *param)
{
	param->key = key_nooption;
	param->value = NULL;
	param->value_size = 0;
}

/* Destroys the given cfg_param structure */
static void
cfg_param_clear(struct cfg_param *param)
{
	if (param->value != NULL)
		free(param->value);
}

/* Resets (empty) the given cfg_param structure. */
static void
cfg_param_reset(struct cfg_param *param)
{
	param->key = key_nooption;

	if (param->value != NULL)
		*param->value = '\0';
}

/* Appends a value to the given cfg_param structure. */
static int
cfg_param_addvalue(struct cfg_param *param, const char *value, int lineno)
{
	if (value == NULL) {
		/* Nothing to append */
		log_warnx("%s: line %d: %s", __FUNCTION__, lineno,
		    strerror(EINVAL));

		return (-EINVAL);
	}

	if (param->value == NULL) {
		size_t size = strlen(value) + 1;

		param->value = malloc(size);

		if (param->value == NULL) {
			log_warnx("%s: line %d: Out of memory",
			    __FUNCTION__, lineno);

			param->value_size = 0;
			return (-ENOMEM);
		}

		param->value_size = size;

		strlcpy(param->value, value, size);
	} else {
		size_t size_needed = strlen(param->value) + strlen(value) + 1;

		if (param->value_size < size_needed) {
			/* Not enough memory to hold the complete value */
			char *tmp = realloc(param->value, size_needed);

			if (tmp == NULL) {
				log_warnx("%s: line %d: Out of memory",
				    __FUNCTION__, lineno);

				free(param->value);

				param->value = NULL;
				param->value_size = 0;

				return (-ENOMEM);
			}

			param->value = tmp;
			param->value_size = size_needed;
		}

		strncat(param->value, value, strlen(value));
	}

	return (0);
}

/**
 * Reads out the configuration from the given file handle. The results are
 * written into the anoubisd_config structure.
 *
 * On success 0 is return (no errors). A return value greater than 0 specifies
 * the number of detected errors. If one of the arguments are NULL, -1 is
 * returned.
 *
 * ** Format of configuration-file **
 *
 * In general:
 *
 * <key> = <value>
 *
 * If a line end with a backslash, the value is continued in the next line.
 *
 * A complete line can be marked as a comment, when it starts with the
 * hash-key.
 *
 * Only keys specified in the keywords array are accepted. Otherwise an error
 * is raised. The format of the value highly depends on the key.
 */
static int
cfg_parse_file(FILE *f)
{
	struct cfg_param param;
	char buf[1024], *line;
	unsigned int lineno;
	int key_expected;
	int err_counter;
	int result;

	if (f == NULL)
		return (-1);

	lineno = 0;
	key_expected = 1;
	err_counter = 0;
	cfg_param_initialize(&param);

	while (fgets(buf, sizeof(buf), f)) {
		lineno++;
		line = buf;

		if (strlen(buf) >= (sizeof(buf) - 1)) {
			/* Max. size of line reached, but this is too much */
			log_warnx("%s: line %d: Line too long",
			    __FUNCTION__, lineno);

			err_counter++;
			cfg_param_reset(&param);

			continue;
		}

		/* Strip leading and trailing whitespaces */
		line = strip_whitespaces(line);

		if (strlen(line) == 0) {
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

			if (pos != NULL) {
				*pos = '\0'; /* Split key and value */

				/* Extract key, ends at =-character */
				key = strip_whitespaces(line);

				/* Rest of line */
				line = strip_whitespaces(pos + 1);
			} else {
				/*
				 * No =-character found.
				 * Assume that the whole line is a key
				 * (with an empty value)
				 */
				key = line;
				line += strlen(line);
			}

			/* Parse the key */
			param.key = parse_key(key);

			if (param.key == key_bad) {
				/*
				 * No mapping for the requested key, continue
				 * with next paramater
				 */
				log_warnx("%s: line %d: "
				    "Bad configuration option: %s",
				    __FUNCTION__, lineno, key);

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

			result = cfg_param_addvalue(&param, line, lineno);
			if (result != 0) {
				err_counter++;

				if (result == -ENOMEM) {
					cfg_param_clear(&param);
					return (err_counter);
				} else {
					cfg_param_reset(&param);
					continue;
				}
			}

			key_expected = 0; /* Continued, next line has no key */
		} else {
			/*
			 * No backslash at the end of the line,
			 * Key-value-pair completed
			 */
			result = cfg_param_addvalue(&param, line, lineno);
			if (result != 0) {
				err_counter++;

				if (result == -ENOMEM) {
					cfg_param_clear(&param);
					return (err_counter);
				} else {
					cfg_param_reset(&param);
					continue;
				}
			}

			result = cfg_param_process(&param, lineno);
			if (result != 0) {
				/*
				 * Failed to process the value, continue with
				 * next paramater
				 */
				err_counter++;
				key_expected = 1;

				if (result == -ENOMEM) {
					cfg_param_clear(&param);
					return (err_counter);
				} else {
					cfg_param_reset(&param);
					continue;
				}
			}

			cfg_param_reset(&param);
			key_expected = 1; /* Next line starts with a key */
		}
	}

	if (param.key != key_nooption) {
		/*
		 * There's still an unprocessed parameter! This can happen, if
		 * the last value is escapted with an backslash at the end of
		 * the line. Do not forget this parameter and process it now!
		 */
		if (cfg_param_process(&param, lineno) != 0)
			err_counter++;
	}

	cfg_param_clear(&param);

	return (err_counter);
}

/*
 * Transforms the given string into a anoubisd_upgrade_mode value.
 * On success, the related anoubisd_upgrade_mode is returned and *ok is set
 * to 1. On failure, *ok is set to 0.
 */
static int
cfg_parse_upgrade_mode(const char *value, int *ok, int lineno)
{
	if ((value == NULL) || (ok == NULL)) {
		log_warnx("%s: line %d: Invalid argument",
		    __FUNCTION__, lineno);

		*ok = 0;
		return (-1);
	}

	if (strcasecmp(value, "off") == 0) {
		*ok = 1;
		return (ANOUBISD_UPGRADE_MODE_OFF);
	} else if (strcasecmp(value, "strictlock") == 0) {
		*ok = 1;
		return (ANOUBISD_UPGRADE_MODE_STRICT_LOCK);
	} else if (strcasecmp(value, "looselock") == 0) {
		*ok = 1;
		return (ANOUBISD_UPGRADE_MODE_LOOSE_LOCK);
	} else if (strcasecmp(value, "process") == 0) {
		*ok = 1;
		return (ANOUBISD_UPGRADE_MODE_PROCESS);
	} else {
		log_warnx("%s: line %d: Unexpected value (%s)",
		    __FUNCTION__, lineno, value);

		*ok = 0;
		return (-1);
	}
}

/*
 * Filles the anoubisd_config.upgrade_trigger list with triggers
 * enumerated in the given value.
 */
static int
cfg_parse_upgrade_trigger(const char *value, int lineno) {

	struct anoubisd_upgrade_trigger	*trigger;
	char				*s;

	if (value == NULL) {
		log_warnx("%s: line %d: Invalid argument",
		    __FUNCTION__, lineno);

		return (-EINVAL);
	}

	if ((s = strdup(value)) == NULL) {
		log_warnx("%s: line %d: Out of memory",
		    __FUNCTION__, lineno);

		return (-ENOMEM);
	}

	while (1) {
		/*
		 * Also split at whitespaces to detect whitespaces around the
		 * comma
		 */
		char *token = strsep(&s, ", \t");

		if (token == NULL) /* End of string reached */
			break;

		if (strlen(token) > 0) { /* Ignore empty strings */
			trigger = malloc(
			    sizeof(struct anoubisd_upgrade_trigger));

			if (trigger == NULL) {
				log_warnx("%s: line %d: Out of memory",
				    __FUNCTION__, lineno);

				free(s);

				return (-ENOMEM);
			}

			trigger->arg = strdup(token);
			if (trigger->arg == NULL) {
				log_warnx("%s: line %d: Out of memory",
				    __FUNCTION__, lineno);

				free(trigger);
				free(s);

				return (-ENOMEM);
			}

			LIST_INSERT_HEAD(&anoubisd_config.upgrade_trigger,
			    trigger, entries);
		}
	}

	free(s);

	return (0);
}

/*
 * Processes the given cfg_param structure, The anoubisd_config structure is
 * filled.
 */
static int
cfg_param_process(struct cfg_param *param, int lineno)
{
	char *s;
	anoubisd_upgrade_mode upgrade_mode;
	int parse_result = 0;

	switch (param->key) {
		case key_unixsocket:
			s = strdup(param->value);

			if (s == NULL) {
				log_warnx("%s: line %d: Out of memory",
				    __FUNCTION__, lineno);
				return (-ENOMEM);
			}

			anoubisd_config.unixsocket = s;
			break;
		case key_upgrade_mode:
			upgrade_mode = cfg_parse_upgrade_mode(
			    param->value, &parse_result, lineno);

			if (parse_result) {
				anoubisd_config.upgrade_mode = upgrade_mode;
				break;
			} else
				return (1);
		case key_upgrade_trigger:
			parse_result = cfg_parse_upgrade_trigger(
			    param->value, lineno);

			if (parse_result != 0)
				return (parse_result);

			break;
		default:
			break;
	}

	return (0);
}

/**
 * Work on the arguments given by the command line.
 */
static int
cfg_read_cmdline(int argc, char * const *argv)
{
	int	 ch;
	char	*endptr;

	ch = 0;
	endptr = NULL;

	while (ch != -1) {
		ch = getopt(argc, argv, "dD:nvs:f:");
		switch (ch) {
		case -1:
			/* No arguments left. */
			break;
		case 'd':
			debug_stderr = 1;
			break;
		case 'D':
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
			data_store.cmdlineopts |= ANOUBISD_OPT_NOACTION;
			break;
		case 'v':
			if (data_store.cmdlineopts & ANOUBISD_OPT_VERBOSE)
				data_store.cmdlineopts |= ANOUBISD_OPT_VERBOSE2;
			data_store.cmdlineopts |= ANOUBISD_OPT_VERBOSE;
			break;
		case 's':
			if (data_store.optsocket) {
				usage();
			}
			data_store.optsocket = optarg;
			break;
		case 'f':
			data_store.conffile = optarg;
			break;
		default:
			usage();
			/*NOTREACHED*/
		}
	}

	return (0);
}

/**
 * Reads out the configuration from the given file. The results are written
 * into the anoubisd_config structure.
 *
 * On success 0 is return (no errors). A return value greater than 0 specifies
 * the number of detected errors. If one of the arguments are NULL, -1 is
 * returned.
 */
int
cfg_read_file(const char *file)
{
	FILE *f;
	int result;

	if (file == NULL)
		return (-1);

	f = fopen(file, "r");
	if (f == NULL) {
		/*
		 * Failed to open config file. But a feasible default
		 * configuration is already loaded. That's why you can says
		 * success here.
		 */
		return (0);
	}

	result = cfg_parse_file(f);
	fclose(f);

	return (result);
}

/**
 * Load the default values.
 */
int
cfg_defaults(void)
{
	char *s;

	anoubisd_config.opts = 0;

	if (anoubisd_config.unixsocket == NULL) {
		s = strdup(PACKAGE_SOCKET);
		if (s == NULL) {
			log_warnx("%s: Out of memory", __FUNCTION__);
			return (-1);
		} else {
			anoubisd_config.unixsocket = s;
		}
	}

	anoubisd_config.upgrade_mode = ANOUBISD_UPGRADE_MODE_OFF;

	return (0);
}

/**
 * Initializes the given anoubisd_config structure.
 *
 * The fields of the structure are filled with default-values (if specified).
 */
int
cfg_initialize(int argc, char * const *argv)
{
	memset(&anoubisd_config, 0, sizeof(struct anoubisd_config));
	cfg_defaults();
	LIST_INIT(&anoubisd_config.upgrade_trigger);

	return (cfg_read_cmdline(argc, argv));
}

/**
 * Releases memory allocated for the given anoubisd_config structure.
 */
void
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
}

int
cfg_read(void)
{
	int rc;

	rc = cfg_read_file(data_store.conffile);

	/* Command line options are more important than config file. */
	if (rc == 0) {
		/* Transfer cmdline socket. */
		if (data_store.optsocket != NULL) {
			free(anoubisd_config.unixsocket);
			anoubisd_config.unixsocket = strdup(
			    data_store.optsocket);
		}

		/* Transfer cmdline options. */
		anoubisd_config.opts = data_store.cmdlineopts;
	}

	return (rc);
}

/**
 * Clears the configuration and read it again.
 */
int
cfg_reread(void)
{
	int rc = 0;

	cfg_clear();
	rc += cfg_defaults();
	rc += cfg_read();

	return (rc);
}

/**
 * Dumps the configuration into the given file handle.
 */
void
cfg_dump(FILE *f)
{
	struct anoubisd_upgrade_trigger *trigger;

	fprintf(f, "conffile: %s\n", data_store.conffile);
	fprintf(f, "unixsocket: %s\n", anoubisd_config.unixsocket);

	/* upgrade_mode */
	switch (anoubisd_config.upgrade_mode) {
	case ANOUBISD_UPGRADE_MODE_OFF:
		fprintf(f, "upgrade_mode: off\n");
		break;
	case ANOUBISD_UPGRADE_MODE_STRICT_LOCK:
		fprintf(f, "upgrade_mode: strictlock\n");
		break;
	case ANOUBISD_UPGRADE_MODE_LOOSE_LOCK:
		fprintf(f, "upgrade_mode: looselock\n");
		break;
	case ANOUBISD_UPGRADE_MODE_PROCESS:
		fprintf(f, "upgrade_mode: process\n");
		break;
	}

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
}

/**
 * Create a message transporting the current config.
 */
anoubisd_msg_t *
cfg_msg_create(void)
{
	int	msgsize;
	int	count;
	int	currlen;
	int	len;

	anoubisd_msg_t			*msg;
	anoubisd_msg_config_t		*confmsg;
	struct anoubisd_upgrade_trigger *trigger;

	/* Calculate needed message size. */
	count = 0;
	msgsize  = sizeof(anoubisd_msg_config_t);
	msgsize += strlen(anoubisd_config.unixsocket) + 1;
	LIST_FOREACH(trigger, &anoubisd_config.upgrade_trigger, entries) {
		msgsize += strlen(trigger->arg) + 1;
		count++;
	}

	/* Create message. */
	msg = msg_factory(ANOUBISD_MSG_CONFIG, msgsize);
	if (msg == NULL) {
		log_warnx("cfg_transmit: Out of memory");
		master_terminate(ENOMEM);
		return (NULL);
	}
	confmsg = (anoubisd_msg_config_t *)msg->msg;

	/* Fill message: options. */
	confmsg->opts = anoubisd_config.opts;

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
		if (currlen + len > msgsize) {
			/* This should never happen! Extemely odd! */
			free(msg);
			return (NULL);
		}
		memcpy(confmsg->chunk + currlen, trigger->arg, len);
		currlen += len;
	}

	return (msg);
}

/**
 * Fill config with message content.
 */
int
cfg_msg_parse(anoubisd_msg_t *msg)
{
	int	count;
	int	offset;

	anoubisd_msg_config_t		*confmsg;
	struct anoubisd_upgrade_trigger *trigger;

	confmsg = (anoubisd_msg_config_t *)msg->msg;

	cfg_clear();

	/* Extract options. */
	anoubisd_config.opts = confmsg->opts;

	/* Extract upgrade mode. */
	anoubisd_config.upgrade_mode =
	    (anoubisd_upgrade_mode)confmsg->upgrade_mode;

	/* Extract unixsocket. */
	offset = strlen(confmsg->chunk) + 1;
	anoubisd_config.unixsocket = (char *)malloc(offset);
	if (anoubisd_config.unixsocket == NULL) {
		log_warnx("%s:%d: Out of memory", __FUNCTION__, __LINE__);
		return (-ENOMEM);
	}
	memcpy(anoubisd_config.unixsocket, confmsg->chunk, offset);

	/* Extract trigger list. */
	count = confmsg->triggercount;
	while (count > 0) {
		trigger = malloc(sizeof(struct anoubisd_upgrade_trigger));
		if (trigger == NULL) {
			log_warnx("%s:%d: Out of memory", __FUNCTION__,
			    __LINE__);
			return (-ENOMEM);
		}

		trigger->arg = strdup(confmsg->chunk + offset);
		if (trigger->arg == NULL) {
			log_warnx("%s:%d: Out of memory", __FUNCTION__,
			    __LINE__);
			free(trigger);
			return (-ENOMEM);
		}

		LIST_INSERT_HEAD(&anoubisd_config.upgrade_trigger,
		    trigger, entries);

		offset += strlen(trigger->arg) + 1;
		count--;
	}

	return (0);
}
