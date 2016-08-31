/*
Copyright (c) 2016 Jorge Giner Cordero

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../config.h"
#include "incl.h"
#include "err.h"

/* Max number of errors before halt. */
#define MAXERR		64

int s_nerrors;

static void eprfl(void)
{
	fprintf(stderr, "%s:%d: ", curfile()->name, curfile()->linenum);
}

static void eprwarn(void)
{
	fputs(_("warning:"), stderr);
	fputc(' ', stderr);
}

/* Print the characters in [p, q[ to stderr. */
void echars(const char *p, const char *q)
{
	while (*p != '\0' && p != q) {
		fputc(*p, stderr);
		p++;
	}
}

/*
 * Print a space, an opening parenthesis, the characters in [p, q[,
 * and a closing parenthesis to stderr.
 */
void epchars(const char *p, const char *q)
{
	fputs(" (", stderr);
	echars(p, q);
	fputs(")", stderr);
}

/*
 * Increments the number of errors, and exit with failure if
 * maximum number of errors allowed is reached.
 */
void newerr(void)
{
	s_nerrors++;
	if (s_nerrors >= MAXERR) {
		eprogname();
		fprintf(stderr, _("exiting: too many errors"));
		exit(EXIT_FAILURE);
	}
}

static void evprint(int warn, const char *ecode, va_list args)
{
	if (nfiles() > 0)
		eprfl();
	else
		eprogname();

	if (warn)
		eprwarn();

	assert(ecode != NULL);
	vfprintf(stderr, ecode, args);
}

/* Prints only the printable characters, the rst as space. */
static void eprint_printable(const char *p)
{
	for (; *p != '\0'; p++) {
		if (isprint(*p))
			putc(*p, stderr);
		else
			putc(' ', stderr);
	}
}

/* Prints the line and a marker pointing to the charcater q inside line. */
void eprcol(const char *line, const char *q)
{
	putc(' ', stderr);
	eprint_printable(line);
	fputs("\n ", stderr);
	while (line != q) {
		putc(' ', stderr);
		line++;
	}
	fputs("^\n", stderr);
}

/*
 * Like fprintf but prints to stderr.
 * If we are parsing any file (incl.c), print first the file and the line.
 * If not, print first the program name.
 */
void eprint(const char *ecode, ...)
{
	va_list args;

	va_start(args, ecode);
	evprint(0, ecode, args);
	va_end(args);
}

/* Same as eprint, but print "warning: " before ecode str.  */
void wprint(const char *ecode, ...)
{
	va_list args;

	va_start(args, ecode);
	evprint(1, ecode, args);
	va_end(args);
}

/* Print \n on stderr. */
void enl(void)
{
	fputc('\n', stderr);
}

/* Print the program name on stderr. */
void eprogname(void)
{
	fprintf(stderr, PACKAGE": ");
}

/* Call malloc, but if no memory, print that error and exit with failure. */
void *emalloc(size_t n)
{
	void *p;

	if ((p = malloc(n)) == NULL) {
		eprint(_("not enough memory\n"));
		exit(EXIT_FAILURE);
	}
	return p;
}

/* Call realloc, but if no memory, print that error and exit with failure. */
void *erealloc(void *p, size_t n)
{
	if ((p = realloc(p, n)) == NULL) {
		eprint(_("not enough memory\n"));
		exit(EXIT_FAILURE);
	}
	return p;
}

/* Call fopen, but if any error, print it and exit with failure. */
FILE *efopen(const char *fname, const char *ops)
{
	FILE *fp;

	if ((fp = fopen(fname, ops)) == NULL) {
		eprint(_("cannot open file %s\n"), fname);
		exit(EXIT_FAILURE);
	}
	return fp;
}
