/* main.c - the main entrypoint to xircd.
 * 
 * $AUTHORS$
 * $LICENSE$
 */
#include <stdio.h>
#include <string.h>
#include <sysexits.h>
#include <time.h>
#include <unistd.h>

#include "xircd.h"
#include "utils.h"
#include "config.h"
#include "runtime.h"

bool_t quit = false;
bool_t graceful_quit = true;

int main(int argc, char **argv) {
	char optch;
	extern char *optarg;
	bool_t show_usage = false;
	bool_t show_version = false;
	bool_t invalid_usage = false;

	set_cf_defaults();

	while((optch = getopt(argc, argv, "c:vdh")) != EOF) {
		switch(optch) {
			case 'c':
				config.cfpath = xstrdup(optarg);
				break;
			case 'v':
				show_version = true;
				break;
			case 'd':
				config.debug = true;
			default:
				invalid_usage = true; /* FALLTHROUGH */
			case 'h':
				show_usage = true;
				break;
		}
	}

	if(show_version) {
		printf("xircd version %s\n", XIRCD_VERSION_STR);
		exit(EX_OK);
	}

	if(show_usage) {
		display_usage();
		exit(invalid_usage ? EX_USAGE : EX_OK);
	}

	if(!logging_start()) 
		exit(EX_TEMPFAIL);

	if(!freadable(config.cfpath)) {
		xlog(LOG_ERR, "unable to open configuration file %s: %s",
			config.cfpath, strerror(errno));
		exit(EX_NOINPUT);
	}

	if(!read_config(config.cfpath)) 
		exit(EX_CONFIG);

	xlog(LOG_INFO, "xircd %s starting.");

	if(!begin_sockets())
		exit(EX_OSERR);

	if(!config.debug)
		become_daemon();

	xlog(LOG_INFO, "ready to accept connections.");

	while(!quit) {
		/* Eat any pending incoming connections. */
		accept_incoming_connections();
		/* Read any buffered data waiting for us. */
		slurp_all_sockets();
		/* Flush any outbound data we have buffered. */
		flush_all_buffers();
		/* See if we received any complete IRC messages from sockets.
		 * If yes, turn them into event objects and put them in queues. */
		check_for_events();
		/* Run through the event queues and do whatever needs to be done 
		 * with each event. */
		dispatch_event_queues();
		/* And then run through any housekeeping. */
		run_periodic_tasks();
	}

	xlog(LOG_INFO, "shutdown request received.");

	if(graceful_quit)
		time_t start = time(NULL); /* my first ever line of C99 code! */

		issue_all_kills(); /* issue KILL messages to everyone */

		while((time(NULL) - start < config.graceful_shutdown_time) &&
			  runtime.connection_count) {
			flush_all_buffers();
			sleep(1);
		}
	}

	close_all_connections();
	free_config();
	logging_end();

	exit(EX_OK);
}
