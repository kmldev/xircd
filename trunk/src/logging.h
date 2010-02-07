/* log.h
 * Prototypes and #defines for logging functions.
 *
 * $AUTHORS$
 * $LICENSE$
 */
#ifndef _LOGGING_H
#define _LOGGING_H

#include <syslog.h>

void xlog(int, char *, ...);

#endif /* !_LOGGING_H */
