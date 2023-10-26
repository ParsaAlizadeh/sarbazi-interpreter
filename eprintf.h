#ifndef EPRINTF_H
#define EPRINTF_H

#include <stdarg.h>
#include <stdlib.h>

void setprogname(const char *);
const char *getprogname(void);

/*
 * print into stderr. eprintf runs exit(2) at the end. if last character of
 * fmt is ':', strerror(errno) will also be printed. always prints \n at
 * the end.
 */
void vweprintf(const char *fmt, va_list);
void weprintf(const char *fmt, ...);
void eprintf(const char *fmt, ...);

/*
 * on error, run eprintf and stop the program.
 */
void *emalloc(size_t);
void *erealloc(void *, size_t);
char *estrdup(const char *);

#endif
