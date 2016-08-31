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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../config.h"
#include "ngetopt.h"
#include "options.h"
#include "utils.h"
#include "err.h"
#include "uz80as.h"

static const char *s_version[] =
{
PACKAGE_STRING,
"",
"Copyright (C) 2016 Jorge Giner Cordero",
"License MIT: <http://opensource.org/licenses/MIT>",
"This is free software: you are free to change and redistribute it.",
"There is NO WARRANTY, to the extent permitted by law."
};

static void print_version(FILE *f)
{
	int i;

	for (i = 0; i < NELEMS(s_version); i++)
		fprintf(f, "%s\n", s_version[i]);
}

static void print_author(void)
{
	printf("\nWritten by Jorge Giner Codero.\n");
}

static void print_help(const char *argv0)
{
	static const char *help =
"uz80as is an assembler for the Zilog Z80 microprocessor.\n"
"\n"
"Usage: %s [OPTION]... ASM_FILE [OBJ_FILE [LST_FILE]]\n"
"\n"
"Options:\n"
"  -h, --help\t\tdisplay this help and exit\n"
"  -v, --version\t\toutput version information and exit\n"
"  -dmacro, --define MACRO\n"
"\t\t\tdefine a macro\n"
"  -f n, --fill n\tfill memory with value n\n"
"  -q, --quiet\t\tdisable the listing file\n"
"  -x, --extended\tenable extended instruction set\n"
"\n"
"Examples:\n"
"  " PACKAGE " p.asm\t\tassemble p.asm into p.obj\n"
"  " PACKAGE " p.asm p.bin\tassemble p.asm into p.bin\n"
"  " PACKAGE " -d\"MUL(a,b) (a*b)\" p.asm\n"
"\t\t\t define the macro MUL and assemle p.asm\n"
"\n"
"Report bugs to: <" PACKAGE_BUGREPORT ">.\n"
"Home page: <" PACKAGE_URL ">.\n";

	printf(help, argv0);
}

/*
 * Allocate a new string which is 'fname' with its extension substituted
 * by 'ext'.
 * If 'fname' has no extension, a dot and 'ext' are appended.
 */
static char *mkfname(const char *fname, const char *ext)
{
	size_t alen, elen;
	const char *q;
	char *s;

	alen = strlen(fname);
	elen = strlen(ext);
	q = fname + alen;
	while (q > fname && *q != '.')
		q--;

	if (q != fname)
		alen = q - fname;

	s = emalloc(alen + 1 + elen + 1);
	strcpy(s, fname);
	s[alen] = '.';
	strcpy(s + alen + 1, ext);
	return s;
}

static void parse_fill_byte(const char *optarg)
{
	int hi, lo;

	if (strlen(optarg) != 2)
		goto error;

	if ((hi = hexval(optarg[0])) < 0)
		goto error;
	if ((lo = hexval(optarg[1])) < 0)
		goto error;

	s_mem_fillval = hi * 16 + lo;
	return;

error:	eprogname();
	fprintf(stderr, _("invalid command line fill value (%s)\n"), optarg);
	eprogname();
	fprintf(stderr, " ");
	fprintf(stderr, _("Please, use two hexadecimal digits.\n"));
	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
	int c;
	struct ngetopt ngo;

	static struct ngetopt_opt ops[] = {
		{ "version", 0, 'v' },
		{ "help", 0, 'h' },
		{ "define", 1, 'd' },
		{ "extended", 0, 'x' },
		{ "fill", 1, 'f' },
		{ "quiet", 0, 'q' },
		{ NULL, 0, 0 },
	};

	ngetopt_init(&ngo, argc, argv, ops);
	do {
		c = ngetopt_next(&ngo);
		switch (c) {
		case 'v':
			print_version(stdout);
			print_author();
			exit(EXIT_SUCCESS);
		case 'h':
			print_help(argv[0]);
			exit(EXIT_SUCCESS);
		case 'd':
			predefine(ngo.optarg);
			break;
		case 'f':
			parse_fill_byte(ngo.optarg);
			break;
		case 'q':
			s_listing = 0;
			break;
		case 'x':
			s_extended_iset = 1;
			break;
		case '?':
			eprint(_("unrecognized option %s\n"),
				ngo.optarg);
			exit(EXIT_FAILURE);
		case ':':
			eprint(_("the -%c option needs an argument\n"),
				(char) ngo.optopt);
			exit(EXIT_FAILURE);
		}
	} while (c != -1);

	if (argc == ngo.optind) {
		eprint(_("wrong number of arguments\n"));
		exit(EXIT_FAILURE);
	}

	s_asmfname = argv[ngo.optind];

	if (argc - ngo.optind > 1)
		s_objfname = argv[ngo.optind + 1];
	else
		s_objfname = mkfname(s_asmfname, "obj");

	if (argc - ngo.optind > 2)
		s_lstfname = argv[ngo.optind + 2];
	else
		s_lstfname = mkfname(s_asmfname, "lst");

	uz80as();
	return 0;
}
