/* client.h - contains metadata about a client we're talking to.  This
 * is referenced by runtime.h.
 *
 * $AUTHORS$
 * $LICENSE$
 */
#ifndef _CLIENT_H
#define _CLIENT_H

/* an instance of a client in the global configuration */
struct client_s {
	char *hostname;
	int port;
	struct client *next;
	char *crlf; /* client's detected \r\n mapping */
};

#endif /* _CLIENT_H */
