/* nl -- number lines of files
   Copyright (C) 1989-2014 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* Written by Scott Bartram (nancy!scott@uunet.uu.net)
   Revised by David MacKenzie (djm@gnu.ai.mit.edu) */

/* #include <config.h> */

#include <stdio.h>
#include <sys/types.h>
#include <getopt.h>

#include "lib/inttypes.in.h"
#include <inttypes.h>
#include <stdlib.h>		/* для EXIT_SUCCESS и EXIT_FAILURE */
/* #include <gettext.h> */

/* #include "system.h" */
#include "lib/gettext.h"

#include "lib/regex.h"

#include <errno.h>
/* #include <inttypes.h> */
/* #include "fadvise.h" */
/* #include "linebuffer.h" */
/* #include "quote.h" */
/* #include "xstrtol.h" */

/* The official name of this program (e.g., no 'g' prefix).  */
#define PROGRAM_NAME "nl"
# define LC_MESSAGES 1729

#define AUTHORS \
	proper_name ("Scott Bartram"), \
proper_name ("David MacKenzie")

/* --------------------------------- */

extern char *program_name;

typedef enum {false, true} bool;

#define HELP_OPTION_DESCRIPTION \
  _("      --help     display this help and exit\n")
#define VERSION_OPTION_DESCRIPTION \
  _("      --version  output version information and exit\n")


#define GETOPT_HELP_OPTION_DECL \
  "help", no_argument, NULL, GETOPT_HELP_CHAR
#define GETOPT_VERSION_OPTION_DECL \
  "version", no_argument, NULL, GETOPT_VERSION_CHAR
/* #define GETOPT_SELINUX_CONTEXT_OPTION_DECL \ */
/*   "context", optional_argument, NULL, 'Z' */

static inline void
emit_try_help (void)
{
  fprintf (stderr, _("Try '%s --help' for more information.\n"), program_name);
}


static inline void
emit_mandatory_arg_note (void)
{
  fputs (_("\n\
Mandatory arguments to long options are mandatory for short options too.\n\
"), stdout);
}

static inline void
emit_ancillary_info (void)
{
  printf (_("\n%s online help: <%s>\n"), "GNU coreutils", "url_!!");
  /* Don't output this redundant message for English locales.
     Note we still output for 'C' so that it gets included in the man page.  */
  const char *lc_messages = setlocale (LC_MESSAGES, NULL);
  if (lc_messages && STRNCMP_LIT (lc_messages, "en_"))
    {
      /* TRANSLATORS: Replace LANG_CODE in this URL with your language code
         <http://translationproject.org/team/LANG_CODE.html> to form one of
         the URLs at http://translationproject.org/team/.  Otherwise, replace
         the entire URL with your translation team's email address.  */
      printf (_("Report %s translation bugs to "
                "<http://translationproject.org/team/>\n"),
                last_component (program_name));
    }
  printf (_("For complete documentation, run: "
            "info coreutils '%s invocation'\n"), last_component (program_name));
}

/* The __attribute__ feature is available in gcc versions 2.5 and later.
   The __-protected variants of the attributes 'format' and 'printf' are
   accepted by gcc versions 2.6.4 (effectively 2.7) and later.
   We enable _GL_ATTRIBUTE_FORMAT only if these are supported too, because
   gnulib and libintl do '#define printf __printf__' when they override
   the 'printf' function.  */
#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 7)
# define _GL_ATTRIBUTE_FORMAT(spec) __attribute__ ((__format__ spec))
#else
# define _GL_ATTRIBUTE_FORMAT(spec) /* empty */
#endif

extern void error (int __status, int __errnum, const char *__format, ...)
     _GL_ATTRIBUTE_FORMAT ((__printf__, 3, 4));

#define _(text) gettext (text)

/* These enum values cannot possibly conflict with the option values
   ordinarily used by commands, including CHAR_MAX + 1, etc.  Avoid
   CHAR_MIN - 1, as it may equal -1, the getopt end-of-options value.  */
enum
{
  GETOPT_HELP_CHAR = (CHAR_MIN - 2),
  GETOPT_VERSION_CHAR = (CHAR_MIN - 3)
};

#if HAVE_POSIX_FADVISE
typedef enum {
  FADVISE_NORMAL =     POSIX_FADV_NORMAL,
  FADVISE_SEQUENTIAL = POSIX_FADV_SEQUENTIAL,
  FADVISE_NOREUSE =    POSIX_FADV_NOREUSE,
  FADVISE_DONTNEED =   POSIX_FADV_DONTNEED,
  FADVISE_WILLNEED =   POSIX_FADV_WILLNEED,
  FADVISE_RANDOM =     POSIX_FADV_RANDOM
} fadvice_t;
#else
typedef enum {
  FADVISE_NORMAL,
  FADVISE_SEQUENTIAL,
  FADVISE_NOREUSE,
  FADVISE_DONTNEED,
  FADVISE_WILLNEED,
  FADVISE_RANDOM
} fadvice_t;
#endif

/* We ignore any errors as these hints are only advisory.
   There is the chance one can pass invalid ADVICE, which will
   not be indicated, but given the simplicity of the interface
   this is unlikely.  Also not returning errors allows the
   unconditional passing of descriptors to non standard files,
   which will just be ignored if unsupported.  */

void fdadvise (int fd, off_t offset, off_t len, fadvice_t advice);
void fadvise (FILE *fp, fadvice_t advice);

/* Redirection and wildcarding when done by the utility itself.
   Generally a noop, but used in particular for native VMS. */
#ifndef initialize_main
# define initialize_main(ac, av)
#endif

/* Set program_name, based on argv[0].
   argv0 must be a string allocated with indefinite extent, and must not be
   modified after this call.  */
extern void set_program_name (const char *argv0);

# define bindtextdomain(Domainname, Dirname) \
    ((void) (Domainname), (const char *) (Dirname))




/* Line-number formats.  They are given an int width, an intmax_t
   value, and a string separator.  */

/* Right justified, no leading zeroes.  */
static char const FORMAT_RIGHT_NOLZ[] = "%*" PRIdMAX "%s";

/* Right justified, leading zeroes.  */
static char const FORMAT_RIGHT_LZ[] = "%0*" PRIdMAX "%s";

/* Left justified, no leading zeroes.  */
static char const FORMAT_LEFT[] = "%-*" PRIdMAX "%s";

/* Default section delimiter characters.  */
static char const DEFAULT_SECTION_DELIMITERS[] = "\\:";

/* Types of input lines: either one of the section delimiters,
   or text to output. */
enum section
{
	Header, Body, Footer, Text
};

/* Format of body lines (-b).  */
static char const *body_type = "t";

/* Format of header lines (-h).  */
static char const *header_type = "n";

/* Format of footer lines (-f).  */
static char const *footer_type = "n";

/* Format currently being used (body, header, or footer).  */
static char const *current_type;

/* Regex for body lines to number (-bp).  */
static struct re_pattern_buffer body_regex;

/* Regex for header lines to number (-hp).  */
static struct re_pattern_buffer header_regex;

/* Regex for footer lines to number (-fp).  */
static struct re_pattern_buffer footer_regex;

/* Fastmaps for the above.  */
static char body_fastmap[UCHAR_MAX + 1];
static char header_fastmap[UCHAR_MAX + 1];
static char footer_fastmap[UCHAR_MAX + 1];

/* Pointer to current regex, if any.  */
static struct re_pattern_buffer *current_regex = NULL;

/* Separator string to print after line number (-s).  */
static char const *separator_str = "\t";

/* Input section delimiter string (-d).  */
static char const *section_del = DEFAULT_SECTION_DELIMITERS;

/* Header delimiter string.  */
static char *header_del = NULL;

/* Header section delimiter length.  */
static size_t header_del_len;

/* Body delimiter string.  */
static char *body_del = NULL;

/* Body section delimiter length.  */
static size_t body_del_len;

/* Footer delimiter string.  */
static char *footer_del = NULL;

/* Footer section delimiter length.  */
static size_t footer_del_len;

/* Input buffer.  */
static struct linebuffer line_buf;

/* printf format string for unnumbered lines.  */
static char *print_no_line_fmt = NULL;

/* Starting line number on each page (-v).  */
static intmax_t starting_line_number = 1;

/* Line number increment (-i).  */
static intmax_t page_incr = 1;

/* If true, reset line number at start of each page (-p).  */
static bool reset_numbers = true;

/* Number of blank lines to consider to be one line for numbering (-l).  */
static intmax_t blank_join = 1;

/* Width of line numbers (-w).  */
static int lineno_width = 6;

/* Line number format (-n).  */
static char const *lineno_format = FORMAT_RIGHT_NOLZ;

/* Current print line number.  */
static intmax_t line_no;

/* True if we have ever read standard input.  */
static bool have_read_stdin;

static struct option const longopts[] =
{
	{"header-numbering", required_argument, NULL, 'h'},
	{"body-numbering", required_argument, NULL, 'b'},
	{"footer-numbering", required_argument, NULL, 'f'},
	{"starting-line-number", required_argument, NULL, 'v'},
	{"line-increment", required_argument, NULL, 'i'},
	{"no-renumber", no_argument, NULL, 'p'},
	{"join-blank-lines", required_argument, NULL, 'l'},
	{"number-separator", required_argument, NULL, 's'},
	{"number-width", required_argument, NULL, 'w'},
	{"number-format", required_argument, NULL, 'n'},
	{"section-delimiter", required_argument, NULL, 'd'},
	{GETOPT_HELP_OPTION_DECL},
	{GETOPT_VERSION_OPTION_DECL},
	{NULL, 0, NULL, 0}
};

/* Print a usage message and quit. */

void usage (int status)
{
	if (status != EXIT_SUCCESS)
		emit_try_help ();
	else
	{
		printf (_("\
					Usage: %s [OPTION]... [FILE]...\n\
					"),
				program_name);
		fputs (_("\
					Write each FILE to standard output, with line numbers added.\n\
					With no FILE, or when FILE is -, read standard input.\n\
					"), stdout);

		emit_mandatory_arg_note ();

		fputs (_("\
					-b, --body-numbering=STYLE      use STYLE for numbering body lines\n\
					-d, --section-delimiter=CC      use CC for separating logical pages\n\
					-f, --footer-numbering=STYLE    use STYLE for numbering footer lines\n\
					"), stdout);
		fputs (_("\
					-h, --header-numbering=STYLE    use STYLE for numbering header lines\n\
					-i, --line-increment=NUMBER     line number increment at each line\n\
					-l, --join-blank-lines=NUMBER   group of NUMBER empty lines counted as one\n\
					-n, --number-format=FORMAT      insert line numbers according to FORMAT\n\
					-p, --no-renumber               do not reset line numbers at logical pages\n\
					-s, --number-separator=STRING   add STRING after (possible) line number\n\
					"), stdout);
		fputs (_("\
					-v, --starting-line-number=NUMBER  first line number on each logical page\n\
					-w, --number-width=NUMBER       use NUMBER columns for line numbers\n\
					"), stdout);
		fputs (HELP_OPTION_DESCRIPTION, stdout);
		fputs (VERSION_OPTION_DESCRIPTION, stdout);
		fputs (_("\
					\n\
					By default, selects -v1 -i1 -l1 -sTAB -w6 -nrn -hn -bt -fn.  CC are\n\
					two delimiter characters for separating logical pages, a missing\n\
					second character implies :.  Type \\\\ for \\.  STYLE is one of:\n\
					"), stdout);
		fputs (_("\
					\n\
					a         number all lines\n\
					t         number only nonempty lines\n\
					n         number no lines\n\
					pBRE      number only lines that contain a match for the basic regular\n\
					expression, BRE\n\
					\n\
					FORMAT is one of:\n\
					\n\
					ln   left justified, no leading zeros\n\
					rn   right justified, no leading zeros\n\
					rz   right justified, leading zeros\n\
					\n\
					"), stdout);
		emit_ancillary_info ();
	}
	exit (status);
}

/* Set the command line flag TYPEP and possibly the regex pointer REGEXP,
   according to 'optarg'.  */

	static bool
build_type_arg (char const **typep,
		struct re_pattern_buffer *regexp, char *fastmap)
{
	char const *errmsg;
	bool rval = true;

	switch (*optarg)
	{
		case 'a':
		case 't':
		case 'n':
			*typep = optarg;
			break;
		case 'p':
			*typep = optarg++;
			regexp->buffer = NULL;
			regexp->allocated = 0;
			regexp->fastmap = fastmap;
			regexp->translate = NULL;
			re_syntax_options =
				RE_SYNTAX_POSIX_BASIC & ~RE_CONTEXT_INVALID_DUP & ~RE_NO_EMPTY_RANGES;
			errmsg = re_compile_pattern (optarg, strlen (optarg), regexp);
			if (errmsg)
				error (EXIT_FAILURE, 0, "%s", errmsg);
			break;
		default:
			rval = false;
			break;
	}
	return rval;
}

/* Print the line number and separator; increment the line number. */

	static void
print_lineno (void)
{
	intmax_t next_line_no;

	printf (lineno_format, lineno_width, line_no, separator_str);

	next_line_no = line_no + page_incr;
	if (next_line_no < line_no)
		error (EXIT_FAILURE, 0, _("line number overflow"));
	line_no = next_line_no;
}

/* Switch to a header section. */

	static void
proc_header (void)
{
	current_type = header_type;
	current_regex = &header_regex;
	if (reset_numbers)
		line_no = starting_line_number;
	putchar ('\n');
}

/* Switch to a body section. */

	static void
proc_body (void)
{
	current_type = body_type;
	current_regex = &body_regex;
	putchar ('\n');
}

/* Switch to a footer section. */

	static void
proc_footer (void)
{
	current_type = footer_type;
	current_regex = &footer_regex;
	putchar ('\n');
}

/* Process a regular text line in 'line_buf'. */

	static void
proc_text (void)
{
	static intmax_t blank_lines = 0;	/* Consecutive blank lines so far. */

	switch (*current_type)
	{
		case 'a':
			if (blank_join > 1)
			{
				if (1 < line_buf.length || ++blank_lines == blank_join)
				{
					print_lineno ();
					blank_lines = 0;
				}
				else
					fputs (print_no_line_fmt, stdout);
			}
			else
				print_lineno ();
			break;
		case 't':
			if (1 < line_buf.length)
				print_lineno ();
			else
				fputs (print_no_line_fmt, stdout);
			break;
		case 'n':
			fputs (print_no_line_fmt, stdout);
			break;
		case 'p':
			switch (re_search (current_regex, line_buf.buffer, line_buf.length - 1,
						0, line_buf.length - 1, NULL))
			{
				case -2:
					error (EXIT_FAILURE, errno, _("error in regular expression search"));

				case -1:
					fputs (print_no_line_fmt, stdout);
					break;

				default:
					print_lineno ();
					break;
			}
	}
	fwrite (line_buf.buffer, sizeof (char), line_buf.length, stdout);
}

/* Return the type of line in 'line_buf'. */

	static enum section
check_section (void)
{
	size_t len = line_buf.length - 1;

	if (len < 2 || memcmp (line_buf.buffer, section_del, 2))
		return Text;
	if (len == header_del_len
			&& !memcmp (line_buf.buffer, header_del, header_del_len))
		return Header;
	if (len == body_del_len
			&& !memcmp (line_buf.buffer, body_del, body_del_len))
		return Body;
	if (len == footer_del_len
			&& !memcmp (line_buf.buffer, footer_del, footer_del_len))
		return Footer;
	return Text;
}

/* Read and process the file pointed to by FP. */

	static void
process_file (FILE *fp)
{
	while (readlinebuffer (&line_buf, fp))
	{
		switch (check_section ())
		{
			case Header:
				proc_header ();
				break;
			case Body:
				proc_body ();
				break;
			case Footer:
				proc_footer ();
				break;
			case Text:
				proc_text ();
				break;
		}
	}
}

/* Process file FILE to standard output.
   Return true if successful.  */

	static bool
nl_file (char const *file)
{
	FILE *stream;

	if (STREQ (file, "-"))
	{
		have_read_stdin = true;
		stream = stdin;
	}
	else
	{
		stream = fopen (file, "r");
		if (stream == NULL)
		{
			error (0, errno, "%s", file);
			return false;
		}
	}

	fadvise (stream, FADVISE_SEQUENTIAL);

	process_file (stream);

	if (ferror (stream))
	{
		error (0, errno, "%s", file);
		return false;
	}
	if (STREQ (file, "-"))
		clearerr (stream);		/* Also clear EOF. */
	else if (fclose (stream) == EOF)
	{
		error (0, errno, "%s", file);
		return false;
	}
	return true;
}

	int
main (int argc, char **argv)
{
	int c;
	size_t len;
	bool ok = true;

	initialize_main (&argc, &argv);
	set_program_name (argv[0]);
	setlocale (LC_ALL, "");
	bindtextdomain (PACKAGE, LOCALEDIR);
	textdomain (PACKAGE);

	atexit (close_stdout);

	have_read_stdin = false;

	while ((c = getopt_long (argc, argv, "h:b:f:v:i:pl:s:w:n:d:", longopts,
					NULL)) != -1)
	{
		switch (c)
		{
			case 'h':
				if (! build_type_arg (&header_type, &header_regex, header_fastmap))
				{
					error (0, 0, _("invalid header numbering style: %s"),
							quote (optarg));
					ok = false;
				}
				break;
			case 'b':
				if (! build_type_arg (&body_type, &body_regex, body_fastmap))
				{
					error (0, 0, _("invalid body numbering style: %s"),
							quote (optarg));
					ok = false;
				}
				break;
			case 'f':
				if (! build_type_arg (&footer_type, &footer_regex, footer_fastmap))
				{
					error (0, 0, _("invalid footer numbering style: %s"),
							quote (optarg));
					ok = false;
				}
				break;
			case 'v':
				if (xstrtoimax (optarg, NULL, 10, &starting_line_number, "")
						!= LONGINT_OK)
				{
					error (0, 0, _("invalid starting line number: %s"),
							quote (optarg));
					ok = false;
				}
				break;
			case 'i':
				if (! (xstrtoimax (optarg, NULL, 10, &page_incr, "") == LONGINT_OK
							&& 0 < page_incr))
				{
					error (0, 0, _("invalid line number increment: %s"),
							quote (optarg));
					ok = false;
				}
				break;
			case 'p':
				reset_numbers = false;
				break;
			case 'l':
				if (! (xstrtoimax (optarg, NULL, 10, &blank_join, "") == LONGINT_OK
							&& 0 < blank_join))
				{
					error (0, 0, _("invalid number of blank lines: %s"),
							quote (optarg));
					ok = false;
				}
				break;
			case 's':
				separator_str = optarg;
				break;
			case 'w':
				{
					long int tmp_long;
					if (xstrtol (optarg, NULL, 10, &tmp_long, "") != LONGINT_OK
							|| tmp_long <= 0 || tmp_long > INT_MAX)
					{
						error (0, 0, _("invalid line number field width: %s"),
								quote (optarg));
						ok = false;
					}
					else
					{
						lineno_width = tmp_long;
					}
				}
				break;
			case 'n':
				if (STREQ (optarg, "ln"))
					lineno_format = FORMAT_LEFT;
				else if (STREQ (optarg, "rn"))
					lineno_format = FORMAT_RIGHT_NOLZ;
				else if (STREQ (optarg, "rz"))
					lineno_format = FORMAT_RIGHT_LZ;
				else
				{
					error (0, 0, _("invalid line numbering format: %s"),
							quote (optarg));
					ok = false;
				}
				break;
			case 'd':
				section_del = optarg;
				break;
				case_GETOPT_HELP_CHAR;
				case_GETOPT_VERSION_CHAR (PROGRAM_NAME, AUTHORS);
			default:
				ok = false;
				break;
		}
	}

	if (!ok)
		usage (EXIT_FAILURE);

	/* Initialize the section delimiters.  */
	len = strlen (section_del);

	header_del_len = len * 3;
	header_del = xmalloc (header_del_len + 1);
	stpcpy (stpcpy (stpcpy (header_del, section_del), section_del), section_del);

	body_del_len = len * 2;
	body_del = xmalloc (body_del_len + 1);
	stpcpy (stpcpy (body_del, section_del), section_del);

	footer_del_len = len;
	footer_del = xmalloc (footer_del_len + 1);
	stpcpy (footer_del, section_del);

	/* Initialize the input buffer.  */
	initbuffer (&line_buf);

	/* Initialize the printf format for unnumbered lines. */
	len = strlen (separator_str);
	print_no_line_fmt = xmalloc (lineno_width + len + 1);
	memset (print_no_line_fmt, ' ', lineno_width + len);
	print_no_line_fmt[lineno_width + len] = '\0';

	line_no = starting_line_number;
	current_type = body_type;
	current_regex = &body_regex;

	/* Main processing. */

	if (optind == argc)
		ok = nl_file ("-");
	else
		for (; optind < argc; optind++)
			ok &= nl_file (argv[optind]);

	if (have_read_stdin && fclose (stdin) == EOF)
		error (EXIT_FAILURE, errno, "-");

	exit (ok ? EXIT_SUCCESS : EXIT_FAILURE);
}
