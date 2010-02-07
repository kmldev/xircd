/* cf.h
 * This file contains structures used for storing the client's configuration -
 * clients, folders, default actions, etc.  Everything that is in the config
 * file is stored in structures found here. 
 *
 * $LICENSE$
 */
#ifndef _CF_H
#define _CF_H

/* an instance of a client in the global configuration */
struct client {
	char *hostname;
	int port;
	struct client *next;
	char *crlf; /* client's detected \r\n mapping */
};

/* the root node of the global configuration */
struct config_s {
	char *cfpath; /* path to configuration file */
	bool_t debug; /* run in debugging mode */
	bool_t logsyslog; /* use syslog? if false log to file. */
	char *logpath; /* if not using syslog */
	struct client *clients; /* list of all clients */
	unsigned int dfltimeout; /* default timeout */
	unsigned int keepalive; /* how long to wait between keepalives */
};

extern struct config_s config;

bool_t read_config(char *);
void free_config(void);

#endif /* !_CF_H */
