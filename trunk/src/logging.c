/* logging.c 
 * Functions used for error logging. 
 * XXX loglevels are kind of messed up right now, need a properly defined
 * mapping between verbose levels and log levels. 
 *
 * $LICENSE$
 */
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include "xircd.h"
#include "config.h"
#include "logging.h"
#include "utils.h"

static FILE *logf;
static bool_t initialized = false;

bool_t logging_start(void) {
	if(!config.debug) {
		if(config.logsyslog) {
			openlog("xircd", LOG_PID, LOG_DAEMON);
		} else {
			logf = fopen(config.logpath, "a");
			if(!logf) {
				xlog(LOG_ERR, "unable to open logpath '%s': %s",
		  		     config.logpath, strerror(errno));
				return(false);
			}
		}
	} else {
		logf = stderr;
	}

	initialized = true;
	return(true);
}

void logging_end(void) {
	if(initialized == false)
		return;
	if(config.logsyslog == true)
		closelog();
	else
		fclose(logf);
}

char *getlogdate(void) {
	char *tp;
	time_t tm = time(NULL);

	tp = ctime(&tm);
	stripnl(tp);
	return(tp);
}

void xlog(int prio, char *fmt, ...) {
	va_list ap;
	FILE *targetf;

	if(prio == LOG_DEBUG && !config.debug)
		return;

	va_start(ap, fmt);
	if(initialized == false)
		targetf = stderr;
	else
		targetf = logf;

	if(config.logsyslog == true) {
		vsyslog(prio, fmt, ap);
	} else {
		fprintf(targetf, "%s [%d]: xircd: ", getlogdate(), getpid());
		vfprintf(targetf, fmt, ap);
		fprintf(targetf, "\n");
		fflush(targetf);
	}
	va_end(ap);
}
