/* utils.h
 * Prototypes and macros for generic utility functions (mostly for strings)
 *
 * $LICENSE$
 */
#ifndef _UTILS_H
#define _UTILS_H

#define stripnl(x) x[strlen(x) - 1] = '\0';
#define MIN(x,y) x < y ? x : y
#define MAX(x,y) x < y ? y : x

#ifndef DEBUG
char *xstrdup(char *);
void *xmalloc(size_t);
void *xrealloc(void *, size_t);
#endif /* !DEBUG */

/* token handling functions */
void freetokens(char **, int);
char **duptokens(char **);
char **gettokens(char *, int *);
int crlf_convert(bool_t *, char *, char **, char *, unsigned int);
int sep_isspace(char c);
int sep_iscomma(char c);
char **gettokens_sep(char *, int *, int (*)(char));

/* operating system-level functions */
bool_t become_daemon(void);

#endif /* !_UTILS_H */
