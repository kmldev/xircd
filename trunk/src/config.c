/* cf.c
 * This file contains all the code used for parsing and managing configuration
 * information. 
 *
 * $AUTHORS$
 * $LICENSE$
 */
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <limits.h>
#include "xircd.h"
#include "config.h"
#include "logging.h"
#include "utils.h"

struct config_s config;

/* prototypes for the various keyword handlers */
static bool_t cf_setuint(char **, int, void *);
static bool_t cf_setbool(char **, int, void *);
static bool_t cf_setlogfile(char **, int, void *);
static bool_t cf_setstr(char **, int, void *);

/* cfhandlers are sets of keyword/function to pointer pairs.  a given
 * function is called when a certain keyword is encountered.  by 
 * swapping around the active cfhandler we can enter and exit different
 * scopes where different commandsets are available. */
struct cfhandler {
	char *keyword;
	int numargs;
	/* tokens, tokencount, linenum, arg */
	bool_t (*kfn)(char **, int, void *);
	void *arg;
};

/* cfhandler block for global keywords */
static struct cfhandler gblkeywords[] = 
	{ { "debug", 1, cf_setbool, &config.debug },
	  { "timeout", 2, cf_setuint, &config.dfltimeout },
	  { "log-syslog", 1, cf_setbool, &config.logsyslog },
	  { "log-file", 2, cf_setlogfile, NULL },
	  { "timeout", 2, cf_setuint, &config.dfltimeout },
	  { NULL, 0, NULL, NULL } };

/* cfhandler block for server keywords */
/*
static struct cfhandler serverkeywords[] = 
	{ { "endserver", 1, cf_endserver, NULL },
	  { "password", 2, cf_setstr, &newserver.pw }, 
	  { NULL, 0, NULL } }; 
*/

/* we start out in the global keyword handler context */
static struct cfhandler *curhandler = gblkeywords;

/* global config handlers */
/* set an unsigned int passed in arg to the value of the second argument */
static bool_t cf_setuint(char **tokens, int lc, void *arg) {
	unsigned int *p = (unsigned int *)arg;

	*p = (unsigned int) strtoul(tokens[1], NULL, 10);

	/* make sure the number is valid.  error checking from strtoul seems
	 * iffy */
	if((*p == ULONG_MAX && errno == ERANGE) ||
	   (*p == 0 && errno == EINVAL)) {
		xlog(LOG_ERR, "%s:%d: invalid number specified in argument", 
		     config.cfpath, lc);
		return(false);
	}

	return(true);
}

/* set a boolean variable passed in arg to true */
static bool_t cf_setbool(char **tokens, int lc, void *arg) {
	bool_t *p = (bool_t *)arg;

	*p = true;
	return(true);
}

static bool_t cf_setlogfile(char **tokens, int lc, void *arg) {
	config.logpath = xstrdup(tokens[1]);

	return(true);
}


/* this function moves us into server context.  This function is a lot more 
 * tolerant of errors than the rest because if there is an error on the 
 * 'server' line, we still want to end up in the right context so the user 
 * doesn't get a ton of incorrect error messages because of one typo. */
/* XXX commented out to show how context switches work, there was a lot
 * more code here before */
/*
static bool_t cf_startserver(char **tokens, int lc, void *arg) {
	.. flip into server context ..
	curhandler = serverkeywords;
	return(isok);
}
*/

/* exit server context */
/* XXX commented out to show how context switches work, there was a lot
 * more code here before. */
/*
static bool_t cf_endserver(char **tokens, int lc, void *arg) {
	.. attempt to continue parsing even after we've seen an error ..
	if(isok == false) {
		free_static_server(&newserver);
		curhandler = gblkeywords;
		return(true);
	}

	.. back to global context ..
	curhandler = gblkeywords;
	return(true);
}
*/

static bool_t cf_setstr(char **tokens, int lc, void *arg) {
	char **p = (char **)arg;

	*p = xstrdup(tokens[1]);

	return(true);
}

/* remove comments from a line */
static void killcomments(char *s) {
	char *p;

	if(s && (p = strchr(s, '#')))
		*p = '\0';
}

/* read in a given configuration. */
bool_t read_config(char *cfp) {
	FILE *cf;
	char inbuf[BUFSIZ], **tokens;
	int lc = 0, tokenc;
	bool_t validkey; /* is the current keyword okay? */
	bool_t validfile = true; /* does the file not contain syntax errors? */
	struct cfhandler *cfhptr;

	config.cfpath = cfp;

	if(!(cf = fopen(config.cfpath, "r"))) {
		xlog(LOG_ERR, "unable to read config file '%s': %s", 
                     config.cfpath, strerror(errno));
		return(false);
	}

	while(fgets(inbuf, BUFSIZ, cf)) {
		lc++;

		stripnl(inbuf);
		killcomments(inbuf); /* replace the first # with \0 */
		tokens = gettokens(inbuf, &tokenc);
		/* empty line? */
		if(!tokenc)
			continue;
		if(!strlen(tokens[0])) {
			freetokens(tokens, tokenc);
			continue;
		}
		/* find the appropriate handler function for this keyword */
		validkey = false;
		for(cfhptr = curhandler; cfhptr->keyword; cfhptr++) {
			if(!strcasecmp(cfhptr->keyword, tokens[0])) {
				if(tokenc < cfhptr->numargs) {
					xlog(LOG_ERR, "%s:%d: insufficient arguments to keyword '%s'",
							config.cfpath, lc, cfhptr->keyword);
					validfile = false;
					break;
				} else if(tokenc > cfhptr->numargs) {
					xlog(LOG_ERR, "%s:%d: extra argument(s) to keyword '%s'",
							config.cfpath, lc, cfhptr->keyword);
					validfile = false;
					break;
				}
				validkey = true;
				if(cfhptr->kfn(tokens, lc, cfhptr->arg) == false) 
					validfile = false;
				break;
			}
		}

		/* was the right handler found? */
		if(validkey == false) {
			/* no */
			validfile = false;
			xlog(LOG_ERR, "%s:%d: unknown or invalid keyword '%s'", 
				config.cfpath, lc, tokens[0]);
			/* XXX should we look in other cfhandler blocks to see if they 
			 * are specifying a keyword in the wrong scope? it would lead to better
			 * error reporting. */
		}

		freetokens(tokens, tokenc);
	}

	fclose(cf);

	return(validfile);
}

void free_config(void) {
	struct client_s *cptr;

	/* clean up everything except config.cfpath. */
	if(config.logpath)
		free(config.logpath);
	/* XXX iterate through runtime.clients and free them all... */
}
