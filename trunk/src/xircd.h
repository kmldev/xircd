/* xircd.h - defines the most generic types and structures needed throughout
 * xircd, as well as some housekeeping things like version strings.
 * 
 * $AUTHORS$
 * $LICENSE$
 */
#ifndef _XIRCD_H
#define _XIRCD_H
#include "logging.h"

#define XIRCD_VERSION_STR "0.1"
#define XIRCD_VERSION_MAJOR 0
#define XIRCD_VERSION_MINOR 1

typedef enum { false = 0, true } bool_t;

extern bool_t quit;
extern bool_t graceful_quit;

#endif /* !_XIRCD_H */
