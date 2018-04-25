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

#include <inttypes.h>
#include <stdlib.h>		/* для EXIT_SUCCESS и EXIT_FAILURE */
#include <locale.h>
#include <stddef.h>

#include "lib/inttypes.in.h"
#include "lib/gettext.h"
/* #include "lib/regex.h" */

#include <errno.h>

/* The official name of this program (e.g., no 'g' prefix).  */
#define PROGRAM_NAME "nl"
# define LC_MESSAGES 1729

#define AUTHORS \
	proper_name ("Scott Bartram"), \
proper_name ("David MacKenzie")

/* --------------------------------- */

extern char *program_name;

typedef enum {false, true} bool;

#ifdef _LIBC
weak_alias (__re_compile_pattern, re_compile_pattern)
#endif

	/* extern const char *re_compile_pattern (const char *__pattern, size_t __length, */
/* 				       struct re_pattern_buffer *__buffer); */
# define re_compile_pattern(pattern, length, bufp) \
	__re_compile_pattern (pattern, length, bufp)



/* If this bit is set, succeed as soon as we match the whole pattern,
   without further backtracking.  */
# define RE_NO_POSIX_BACKTRACKING (RE_UNMATCHED_RIGHT_PAREN_ORD << 1)
/* If this bit is set, do not process the GNU regex operators.
   If not set, then the GNU regex operators are recognized. */
# define RE_NO_GNU_OPS (RE_NO_POSIX_BACKTRACKING << 1)

# define RE_DEBUG (RE_NO_GNU_OPS << 1)
/* If this bit is set, a syntactically invalid interval is treated as
   a string of ordinary characters.  For example, the ERE 'a{1' is
   treated as 'a\{1'.  */
# define RE_INVALID_INTERVAL_ORD (RE_DEBUG << 1)

/* If this bit is set, then ignore case when matching.
   If not set, then case is significant.  */
# define RE_ICASE (RE_INVALID_INTERVAL_ORD << 1)

/* This bit is used internally like RE_CONTEXT_INDEP_ANCHORS but only
   for ^, because it is difficult to scan the regex backwards to find
   whether ^ should be special.  */
# define RE_CARET_ANCHORS_HERE (RE_ICASE << 1)

/* If this bit is set, then \{ cannot be first in a regex or
   immediately after an alternation, open-group or \} operator.  */
# define RE_CONTEXT_INVALID_DUP (RE_CARET_ANCHORS_HERE << 1)
/* If this bit is set, +, ? and | aren't recognized as operators.
   If not set, they are.  */
# define RE_LIMITED_OPS (RE_INTERVALS << 1)
/* If this bit is set, newline is an alternation operator.
   If not set, newline is literal.  */
# define RE_NEWLINE_ALT (RE_LIMITED_OPS << 1)

/* If this bit is set, then '{...}' defines an interval, and \{ and \}
     are literals.
  If not set, then '\{...\}' defines an interval.  */
# define RE_NO_BK_BRACES (RE_NEWLINE_ALT << 1)

/* If this bit is set, (...) defines a group, and \( and \) are literals.
   If not set, \(...\) defines a group, and ( and ) are literals.  */
# define RE_NO_BK_PARENS (RE_NO_BK_BRACES << 1)

/* If this bit is set, then \<digit> matches <digit>.
   If not set, then \<digit> is a back-reference.  */
# define RE_NO_BK_REFS (RE_NO_BK_PARENS << 1)

/* If this bit is set, then | is an alternation operator, and \| is literal.
   If not set, then \| is an alternation operator, and | is literal.  */
# define RE_NO_BK_VBAR (RE_NO_BK_REFS << 1)

/* If this bit is set, then an ending range point collating higher
     than the starting range point, as in [z-a], is invalid.
   If not set, then when ending range point collates higher than the
     starting range point, the range is ignored.  */
# define RE_NO_EMPTY_RANGES (RE_NO_BK_VBAR << 1)

/* If this bit is set, then an unmatched ) is ordinary.
   If not set, then an unmatched ) is invalid.  */
# define RE_UNMATCHED_RIGHT_PAREN_ORD (RE_NO_EMPTY_RANGES << 1)
# define RE_HAT_LISTS_NOT_NEWLINE (RE_DOT_NOT_NULL << 1)

/* If this bit is set, either \{...\} or {...} defines an
     interval, depending on RE_NO_BK_BRACES.
   If not set, \{, \}, {, and } are literals.  */
# define RE_INTERVALS (RE_HAT_LISTS_NOT_NEWLINE << 1)
# define RE_CONTEXT_INDEP_ANCHORS (RE_CHAR_CLASSES << 1)

/* If this bit is set, then special characters are always special
     regardless of where they are in the pattern.
   If this bit is not set, then special characters are special only in
     some contexts; otherwise they are ordinary.  Specifically,
     * + ? and intervals are only special when not after the beginning,
     open-group, or alternation operator.  */
# define RE_CONTEXT_INDEP_OPS (RE_CONTEXT_INDEP_ANCHORS << 1)

/* If this bit is set, then *, +, ?, and { cannot be first in an re or
     immediately after an alternation or begin-group operator.  */
# define RE_CONTEXT_INVALID_OPS (RE_CONTEXT_INDEP_OPS << 1)

/* If this bit is set, then . matches newline.
   If not set, then it doesn't.  */
# define RE_DOT_NEWLINE (RE_CONTEXT_INVALID_OPS << 1)

/* If this bit is set, then . doesn't match NUL.
   If not set, then it does.  */
# define RE_DOT_NOT_NULL (RE_DOT_NEWLINE << 1)
# define RE_BACKSLASH_ESCAPE_IN_LISTS ((unsigned long int) 1)
# define RE_BK_PLUS_QM (RE_BACKSLASH_ESCAPE_IN_LISTS << 1)
# define RE_CHAR_CLASSES (RE_BK_PLUS_QM << 1)

/* Syntax bits common to both basic and extended POSIX regex syntax.  */
# define _RE_SYNTAX_POSIX_COMMON					\
  (RE_CHAR_CLASSES | RE_DOT_NEWLINE      | RE_DOT_NOT_NULL		\
   | RE_INTERVALS  | RE_NO_EMPTY_RANGES)

# define RE_SYNTAX_POSIX_BASIC						\
  (_RE_SYNTAX_POSIX_COMMON | RE_BK_PLUS_QM | RE_CONTEXT_INVALID_DUP)


#define _(text) gettext (text)

/* Just like strncmp, but the second argument must be a literal string
   and you don't specify the length;  that comes from the literal.  */
#define STRNCMP_LIT(s, literal) \
  strncmp (s, "" literal "", sizeof (literal) - 1)

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

/* # define bindtextdomain(Domainname, Dirname) \ */
/*     ((void) (Domainname), (const char *) (Dirname)) */


extern void error (int __status, int __errnum, const char *__format, ...)
     _GL_ATTRIBUTE_FORMAT ((__printf__, 3, 4));


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

void
fdadvise (int fd, off_t offset, off_t len, fadvice_t advice)
{
#if HAVE_POSIX_FADVISE
  ignore_value (posix_fadvise (fd, offset, len, advice));
#endif
}

void
fadvise (FILE *fp, fadvice_t advice)
{
  if (fp)
    fdadvise (fileno (fp), 0, 0, advice);
}



int volatile exit_failure = EXIT_FAILURE;

void
xalloc_die (void)
{
  error (exit_failure, 0, "%s", _("memory exhausted"));

  /* _Noreturn cannot be given to error, since it may return if
     its first argument is 0.  To help compilers understand the
     xalloc_die does not return, call abort.  Also, the abort is a
     safety feature if exit_failure is 0 (which shouldn't happen).  */
  abort ();
}

/* Change the size of an allocated block of memory P to N bytes,
   with error checking.  */

void *
xrealloc (void *p, size_t n)
{
  if (!n && p)
    {
      /* The GNU and C99 realloc behaviors disagree here.  Act like
         GNU, even if the underlying realloc is C99.  */
      free (p);
      return NULL;
    }

  p = realloc (p, n);
  if (!p && n)
    xalloc_die ();
  return p;
}










/* Return 1 if an array of N objects, each of size S, cannot exist due
   to size arithmetic overflow.  S must be positive and N must be
   nonnegative.  This is a macro, not a function, so that it
   works correctly even when SIZE_MAX < N.

   By gnulib convention, SIZE_MAX represents overflow in size
   calculations, so the conservative dividend to use here is
   SIZE_MAX - 1, since SIZE_MAX might represent an overflowed value.
   However, malloc (SIZE_MAX) fails on all known hosts where
   sizeof (ptrdiff_t) <= sizeof (size_t), so do not bother to test for
   exactly-SIZE_MAX allocations on such hosts; this avoids a test and
   branch when S is known to be 1.  */
# define xalloc_oversized(n, s) \
    ((size_t) (sizeof (ptrdiff_t) <= sizeof (size_t) ? -1 : -2) / (s) < (n))

/* Basic quoting styles.  For each style, an example is given on the
   input strings "simple", "\0 \t\n'\"\033?""?/\\", and "a:b", using
   quotearg_buffer, quotearg_mem, and quotearg_colon_mem with that
   style and the default flags and quoted characters.  Note that the
   examples are shown here as valid C strings rather than what
   displays on a terminal (with "??/" as a trigraph for "\\").  */
enum quoting_style
  {
    /* Output names as-is (ls --quoting-style=literal).  Can result in
       embedded null bytes if QA_ELIDE_NULL_BYTES is not in
       effect.

       quotearg_buffer:
       "simple", "\0 \t\n'\"\033??/\\", "a:b"
       quotearg:
       "simple", " \t\n'\"\033??/\\", "a:b"
       quotearg_colon:
       "simple", " \t\n'\"\033??/\\", "a:b"
    */
    literal_quoting_style,

    /* Quote names for the shell if they contain shell metacharacters
       or would cause ambiguous output (ls --quoting-style=shell).
       Can result in embedded null bytes if QA_ELIDE_NULL_BYTES is not
       in effect.

       quotearg_buffer:
       "simple", "'\0 \t\n'\\''\"\033??/\\'", "a:b"
       quotearg:
       "simple", "' \t\n'\\''\"\033??/\\'", "a:b"
       quotearg_colon:
       "simple", "' \t\n'\\''\"\033??/\\'", "'a:b'"
    */
    shell_quoting_style,

    /* Quote names for the shell, even if they would normally not
       require quoting (ls --quoting-style=shell-always).  Can result
       in embedded null bytes if QA_ELIDE_NULL_BYTES is not in effect.
       Behaves like shell_quoting_style if QA_ELIDE_OUTER_QUOTES is in
       effect.

       quotearg_buffer:
       "'simple'", "'\0 \t\n'\\''\"\033??/\\'", "'a:b'"
       quotearg:
       "'simple'", "' \t\n'\\''\"\033??/\\'", "'a:b'"
       quotearg_colon:
       "'simple'", "' \t\n'\\''\"\033??/\\'", "'a:b'"
    */
    shell_always_quoting_style,

    /* Quote names as for a C language string (ls --quoting-style=c).
       Behaves like c_maybe_quoting_style if QA_ELIDE_OUTER_QUOTES is
       in effect.  Split into consecutive strings if
       QA_SPLIT_TRIGRAPHS.

       quotearg_buffer:
       "\"simple\"", "\"\\0 \\t\\n'\\\"\\033??/\\\\\"", "\"a:b\""
       quotearg:
       "\"simple\"", "\"\\0 \\t\\n'\\\"\\033??/\\\\\"", "\"a:b\""
       quotearg_colon:
       "\"simple\"", "\"\\0 \\t\\n'\\\"\\033??/\\\\\"", "\"a\\:b\""
    */
    c_quoting_style,

    /* Like c_quoting_style except omit the surrounding double-quote
       characters if no quoted characters are encountered.

       quotearg_buffer:
       "simple", "\"\\0 \\t\\n'\\\"\\033??/\\\\\"", "a:b"
       quotearg:
       "simple", "\"\\0 \\t\\n'\\\"\\033??/\\\\\"", "a:b"
       quotearg_colon:
       "simple", "\"\\0 \\t\\n'\\\"\\033??/\\\\\"", "\"a:b\""
    */
    c_maybe_quoting_style,

    /* Like c_quoting_style except always omit the surrounding
       double-quote characters and ignore QA_SPLIT_TRIGRAPHS
       (ls --quoting-style=escape).

       quotearg_buffer:
       "simple", "\\0 \\t\\n'\"\\033??/\\\\", "a:b"
       quotearg:
       "simple", "\\0 \\t\\n'\"\\033??/\\\\", "a:b"
       quotearg_colon:
       "simple", "\\0 \\t\\n'\"\\033??/\\\\", "a\\:b"
    */
    escape_quoting_style,

    /* Like clocale_quoting_style, but use single quotes in the
       default C locale or if the program does not use gettext
       (ls --quoting-style=locale).  For UTF-8 locales, quote
       characters will use Unicode.

       LC_MESSAGES=C
       quotearg_buffer:
       "`simple'", "`\\0 \\t\\n\\'\"\\033??/\\\\'", "`a:b'"
       quotearg:
       "`simple'", "`\\0 \\t\\n\\'\"\\033??/\\\\'", "`a:b'"
       quotearg_colon:
       "`simple'", "`\\0 \\t\\n\\'\"\\033??/\\\\'", "`a\\:b'"

       LC_MESSAGES=pt_PT.utf8
       quotearg_buffer:
       "\302\253simple\302\273",
       "\302\253\\0 \\t\\n'\"\\033??/\\\\\302\253", "\302\253a:b\302\273"
       quotearg:
       "\302\253simple\302\273",
       "\302\253\\0 \\t\\n'\"\\033??/\\\\\302\253", "\302\253a:b\302\273"
       quotearg_colon:
       "\302\253simple\302\273",
       "\302\253\\0 \\t\\n'\"\\033??/\\\\\302\253", "\302\253a\\:b\302\273"
    */
    locale_quoting_style,

    /* Like c_quoting_style except use quotation marks appropriate for
       the locale and ignore QA_SPLIT_TRIGRAPHS
       (ls --quoting-style=clocale).

       LC_MESSAGES=C
       quotearg_buffer:
       "\"simple\"", "\"\\0 \\t\\n'\\\"\\033??/\\\\\"", "\"a:b\""
       quotearg:
       "\"simple\"", "\"\\0 \\t\\n'\\\"\\033??/\\\\\"", "\"a:b\""
       quotearg_colon:
       "\"simple\"", "\"\\0 \\t\\n'\\\"\\033??/\\\\\"", "\"a\\:b\""

       LC_MESSAGES=pt_PT.utf8
       quotearg_buffer:
       "\302\253simple\302\273",
       "\302\253\\0 \\t\\n'\"\\033??/\\\\\302\253", "\302\253a:b\302\273"
       quotearg:
       "\302\253simple\302\273",
       "\302\253\\0 \\t\\n'\"\\033??/\\\\\302\253", "\302\253a:b\302\273"
       quotearg_colon:
       "\302\253simple\302\273",
       "\302\253\\0 \\t\\n'\"\\033??/\\\\\302\253", "\302\253a\\:b\302\273"
    */
    clocale_quoting_style,

    /* Like clocale_quoting_style except use the custom quotation marks
       set by set_custom_quoting.  If custom quotation marks are not
       set, the behavior is undefined.

       left_quote = right_quote = "'"
       quotearg_buffer:
       "'simple'", "'\\0 \\t\\n\\'\"\\033??/\\\\'", "'a:b'"
       quotearg:
       "'simple'", "'\\0 \\t\\n\\'\"\\033??/\\\\'", "'a:b'"
       quotearg_colon:
       "'simple'", "'\\0 \\t\\n\\'\"\\033??/\\\\'", "'a\\:b'"

       left_quote = "(" and right_quote = ")"
       quotearg_buffer:
       "(simple)", "(\\0 \\t\\n'\"\\033??/\\\\)", "(a:b)"
       quotearg:
       "(simple)", "(\\0 \\t\\n'\"\\033??/\\\\)", "(a:b)"
       quotearg_colon:
       "(simple)", "(\\0 \\t\\n'\"\\033??/\\\\)", "(a\\:b)"

       left_quote = ":" and right_quote = " "
       quotearg_buffer:
       ":simple ", ":\\0\\ \\t\\n'\"\\033??/\\\\ ", ":a:b "
       quotearg:
       ":simple ", ":\\0\\ \\t\\n'\"\\033??/\\\\ ", ":a:b "
       quotearg_colon:
       ":simple ", ":\\0\\ \\t\\n'\"\\033??/\\\\ ", ":a\\:b "

       left_quote = "\"'" and right_quote = "'\""
       Notice that this is treated as a single level of quotes or two
       levels where the outer quote need not be escaped within the inner
       quotes.  For two levels where the outer quote must be escaped
       within the inner quotes, you must use separate quotearg
       invocations.
       quotearg_buffer:
       "\"'simple'\"", "\"'\\0 \\t\\n\\'\"\\033??/\\\\'\"", "\"'a:b'\""
       quotearg:
       "\"'simple'\"", "\"'\\0 \\t\\n\\'\"\\033??/\\\\'\"", "\"'a:b'\""
       quotearg_colon:
       "\"'simple'\"", "\"'\\0 \\t\\n\\'\"\\033??/\\\\'\"", "\"'a\\:b'\""
    */
    custom_quoting_style
  };




static size_t
quotearg_buffer_restyled (char *buffer, size_t buffersize,
                          char const *arg, size_t argsize,
                          enum quoting_style quoting_style, int flags,
                          unsigned int const *quote_these_too,
                          char const *left_quote,
                          char const *right_quote)
{
  size_t i;
  size_t len = 0;
  char const *quote_string = 0;
  size_t quote_string_len = 0;
  bool backslash_escapes = false;
  bool unibyte_locale = MB_CUR_MAX == 1;
  bool elide_outer_quotes = (flags & QA_ELIDE_OUTER_QUOTES) != 0;

#define STORE(c) \
    do \
      { \
        if (len < buffersize) \
          buffer[len] = (c); \
        len++; \
      } \
    while (0)

  switch (quoting_style)
    {
    case c_maybe_quoting_style:
      quoting_style = c_quoting_style;
      elide_outer_quotes = true;
      /* Fall through.  */
    case c_quoting_style:
      if (!elide_outer_quotes)
        STORE ('"');
      backslash_escapes = true;
      quote_string = "\"";
      quote_string_len = 1;
      break;

    case escape_quoting_style:
      backslash_escapes = true;
      elide_outer_quotes = false;
      break;

    case locale_quoting_style:
    case clocale_quoting_style:
    case custom_quoting_style:
      {
        if (quoting_style != custom_quoting_style)
          {
            /* TRANSLATORS:
               Get translations for open and closing quotation marks.
               The message catalog should translate "`" to a left
               quotation mark suitable for the locale, and similarly for
               "'".  For example, a French Unicode local should translate
               these to U+00AB (LEFT-POINTING DOUBLE ANGLE
               QUOTATION MARK), and U+00BB (RIGHT-POINTING DOUBLE ANGLE
               QUOTATION MARK), respectively.

               If the catalog has no translation, we will try to
               use Unicode U+2018 (LEFT SINGLE QUOTATION MARK) and
               Unicode U+2019 (RIGHT SINGLE QUOTATION MARK).  If the
               current locale is not Unicode, locale_quoting_style
               will quote 'like this', and clocale_quoting_style will
               quote "like this".  You should always include translations
               for "`" and "'" even if U+2018 and U+2019 are appropriate
               for your locale.

               If you don't know what to put here, please see
               <http://en.wikipedia.org/wiki/Quotation_marks_in_other_languages>
               and use glyphs suitable for your language.  */
            left_quote = gettext_quote (N_("`"), quoting_style);
            right_quote = gettext_quote (N_("'"), quoting_style);
          }
        if (!elide_outer_quotes)
          for (quote_string = left_quote; *quote_string; quote_string++)
            STORE (*quote_string);
        backslash_escapes = true;
        quote_string = right_quote;
        quote_string_len = strlen (quote_string);
      }
      break;

    case shell_quoting_style:
      quoting_style = shell_always_quoting_style;
      elide_outer_quotes = true;
      /* Fall through.  */
    case shell_always_quoting_style:
      if (!elide_outer_quotes)
        STORE ('\'');
      quote_string = "'";
      quote_string_len = 1;
      break;

    case literal_quoting_style:
      elide_outer_quotes = false;
      break;

    default:
      abort ();
    }

  for (i = 0;  ! (argsize == SIZE_MAX ? arg[i] == '\0' : i == argsize);  i++)
    {
      unsigned char c;
      unsigned char esc;
      bool is_right_quote = false;

      if (backslash_escapes
          && quote_string_len
          && (i + quote_string_len
              <= (argsize == SIZE_MAX && 1 < quote_string_len
                  /* Use strlen only if we must: when argsize is SIZE_MAX,
                     and when the quote string is more than 1 byte long.
                     If we do call strlen, save the result.  */
                  ? (argsize = strlen (arg)) : argsize))
          && memcmp (arg + i, quote_string, quote_string_len) == 0)
        {
          if (elide_outer_quotes)
            goto force_outer_quoting_style;
          is_right_quote = true;
        }

      c = arg[i];
      switch (c)
        {
        case '\0':
          if (backslash_escapes)
            {
              if (elide_outer_quotes)
                goto force_outer_quoting_style;
              STORE ('\\');
              /* If quote_string were to begin with digits, we'd need to
                 test for the end of the arg as well.  However, it's
                 hard to imagine any locale that would use digits in
                 quotes, and set_custom_quoting is documented not to
                 accept them.  */
              if (i + 1 < argsize && '0' <= arg[i + 1] && arg[i + 1] <= '9')
                {
                  STORE ('0');
                  STORE ('0');
                }
              c = '0';
              /* We don't have to worry that this last '0' will be
                 backslash-escaped because, again, quote_string should
                 not start with it and because quote_these_too is
                 documented as not accepting it.  */
            }
          else if (flags & QA_ELIDE_NULL_BYTES)
            continue;
          break;

        case '?':
          switch (quoting_style)
            {
            case shell_always_quoting_style:
              if (elide_outer_quotes)
                goto force_outer_quoting_style;
              break;

            case c_quoting_style:
              if ((flags & QA_SPLIT_TRIGRAPHS)
                  && i + 2 < argsize && arg[i + 1] == '?')
                switch (arg[i + 2])
                  {
                  case '!': case '\'':
                  case '(': case ')': case '-': case '/':
                  case '<': case '=': case '>':
                    /* Escape the second '?' in what would otherwise be
                       a trigraph.  */
                    if (elide_outer_quotes)
                      goto force_outer_quoting_style;
                    c = arg[i + 2];
                    i += 2;
                    STORE ('?');
                    STORE ('"');
                    STORE ('"');
                    STORE ('?');
                    break;

                  default:
                    break;
                  }
              break;

            default:
              break;
            }
          break;

        case '\a': esc = 'a'; goto c_escape;
        case '\b': esc = 'b'; goto c_escape;
        case '\f': esc = 'f'; goto c_escape;
        case '\n': esc = 'n'; goto c_and_shell_escape;
        case '\r': esc = 'r'; goto c_and_shell_escape;
        case '\t': esc = 't'; goto c_and_shell_escape;
        case '\v': esc = 'v'; goto c_escape;
        case '\\': esc = c;
          /* No need to escape the escape if we are trying to elide
             outer quotes and nothing else is problematic.  */
          if (backslash_escapes && elide_outer_quotes && quote_string_len)
            goto store_c;

        c_and_shell_escape:
          if (quoting_style == shell_always_quoting_style
              && elide_outer_quotes)
            goto force_outer_quoting_style;
          /* Fall through.  */
        c_escape:
          if (backslash_escapes)
            {
              c = esc;
              goto store_escape;
            }
          break;

        case '{': case '}': /* sometimes special if isolated */
          if (! (argsize == SIZE_MAX ? arg[1] == '\0' : argsize == 1))
            break;
          /* Fall through.  */
        case '#': case '~':
          if (i != 0)
            break;
          /* Fall through.  */
        case ' ':
        case '!': /* special in bash */
        case '"': case '$': case '&':
        case '(': case ')': case '*': case ';':
        case '<':
        case '=': /* sometimes special in 0th or (with "set -k") later args */
        case '>': case '[':
        case '^': /* special in old /bin/sh, e.g. SunOS 4.1.4 */
        case '`': case '|':
          /* A shell special character.  In theory, '$' and '`' could
             be the first bytes of multibyte characters, which means
             we should check them with mbrtowc, but in practice this
             doesn't happen so it's not worth worrying about.  */
          if (quoting_style == shell_always_quoting_style
              && elide_outer_quotes)
            goto force_outer_quoting_style;
          break;

        case '\'':
          if (quoting_style == shell_always_quoting_style)
            {
              if (elide_outer_quotes)
                goto force_outer_quoting_style;
              STORE ('\'');
              STORE ('\\');
              STORE ('\'');
            }
          break;

        case '%': case '+': case ',': case '-': case '.': case '/':
        case '0': case '1': case '2': case '3': case '4': case '5':
        case '6': case '7': case '8': case '9': case ':':
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
        case 'G': case 'H': case 'I': case 'J': case 'K': case 'L':
        case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R':
        case 'S': case 'T': case 'U': case 'V': case 'W': case 'X':
        case 'Y': case 'Z': case ']': case '_': case 'a': case 'b':
        case 'c': case 'd': case 'e': case 'f': case 'g': case 'h':
        case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
        case 'o': case 'p': case 'q': case 'r': case 's': case 't':
        case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
          /* These characters don't cause problems, no matter what the
             quoting style is.  They cannot start multibyte sequences.
             A digit or a special letter would cause trouble if it
             appeared at the beginning of quote_string because we'd then
             escape by prepending a backslash.  However, it's hard to
             imagine any locale that would use digits or letters as
             quotes, and set_custom_quoting is documented not to accept
             them.  Also, a digit or a special letter would cause
             trouble if it appeared in quote_these_too, but that's also
             documented as not accepting them.  */
          break;

        default:
          /* If we have a multibyte sequence, copy it until we reach
             its end, find an error, or come back to the initial shift
             state.  For C-like styles, if the sequence has
             unprintable characters, escape the whole sequence, since
             we can't easily escape single characters within it.  */
          {
            /* Length of multibyte sequence found so far.  */
            size_t m;

            bool printable;

            if (unibyte_locale)
              {
                m = 1;
                printable = isprint (c) != 0;
              }
            else
              {
                mbstate_t mbstate;
                memset (&mbstate, 0, sizeof mbstate);

                m = 0;
                printable = true;
                if (argsize == SIZE_MAX)
                  argsize = strlen (arg);

                do
                  {
                    wchar_t w;
                    size_t bytes = mbrtowc (&w, &arg[i + m],
                                            argsize - (i + m), &mbstate);
                    if (bytes == 0)
                      break;
                    else if (bytes == (size_t) -1)
                      {
                        printable = false;
                        break;
                      }
                    else if (bytes == (size_t) -2)
                      {
                        printable = false;
                        while (i + m < argsize && arg[i + m])
                          m++;
                        break;
                      }
                    else
                      {
                        /* Work around a bug with older shells that "see" a '\'
                           that is really the 2nd byte of a multibyte character.
                           In practice the problem is limited to ASCII
                           chars >= '@' that are shell special chars.  */
                        if ('[' == 0x5b && elide_outer_quotes
                            && quoting_style == shell_always_quoting_style)
                          {
                            size_t j;
                            for (j = 1; j < bytes; j++)
                              switch (arg[i + m + j])
                                {
                                case '[': case '\\': case '^':
                                case '`': case '|':
                                  goto force_outer_quoting_style;

                                default:
                                  break;
                                }
                          }

                        if (! iswprint (w))
                          printable = false;
                        m += bytes;
                      }
                  }
                while (! mbsinit (&mbstate));
              }

            if (1 < m || (backslash_escapes && ! printable))
              {
                /* Output a multibyte sequence, or an escaped
                   unprintable unibyte character.  */
                size_t ilim = i + m;

                for (;;)
                  {
                    if (backslash_escapes && ! printable)
                      {
                        if (elide_outer_quotes)
                          goto force_outer_quoting_style;
                        STORE ('\\');
                        STORE ('0' + (c >> 6));
                        STORE ('0' + ((c >> 3) & 7));
                        c = '0' + (c & 7);
                      }
                    else if (is_right_quote)
                      {
                        STORE ('\\');
                        is_right_quote = false;
                      }
                    if (ilim <= i + 1)
                      break;
                    STORE (c);
                    c = arg[++i];
                  }

                goto store_c;
              }
          }
        }

      if (! ((backslash_escapes || elide_outer_quotes)
             && quote_these_too
             && quote_these_too[c / INT_BITS] >> (c % INT_BITS) & 1)
          && !is_right_quote)
        goto store_c;

    store_escape:
      if (elide_outer_quotes)
        goto force_outer_quoting_style;
      STORE ('\\');

    store_c:
      STORE (c);
    }

  if (len == 0 && quoting_style == shell_always_quoting_style
      && elide_outer_quotes)
    goto force_outer_quoting_style;

  if (quote_string && !elide_outer_quotes)
    for (; *quote_string; quote_string++)
      STORE (*quote_string);

  if (len < buffersize)
    buffer[len] = '\0';
  return len;

 force_outer_quoting_style:
  /* Don't reuse quote_these_too, since the addition of outer quotes
     sufficiently quotes the specified characters.  */
  return quotearg_buffer_restyled (buffer, buffersize, arg, argsize,
                                   quoting_style,
                                   flags & ~QA_ELIDE_OUTER_QUOTES, NULL,
                                   left_quote, right_quote);
}




#define INT_BITS (sizeof (int) * CHAR_BIT)

struct quoting_options
{
  /* Basic quoting style.  */
  enum quoting_style style;

  /* Additional flags.  Bitwise combination of enum quoting_flags.  */
  int flags;

  /* Quote the characters indicated by this bit vector even if the
     quoting style would not normally require them to be quoted.  */
  unsigned int quote_these_too[(UCHAR_MAX / INT_BITS) + 1];

  /* The left quote for custom_quoting_style.  */
  char const *left_quote;

  /* The right quote for custom_quoting_style.  */
  char const *right_quote;
};


/* Correspondences to quoting style names.  */
enum quoting_style const quoting_style_vals[] =
{
  literal_quoting_style,
  shell_quoting_style,
  shell_always_quoting_style,
  c_quoting_style,
  c_maybe_quoting_style,
  escape_quoting_style,
  locale_quoting_style,
  clocale_quoting_style
};

/* The quoting option used by the functions of quote.h.  */
struct quoting_options quote_quoting_options =
  {
    locale_quoting_style,
    0,
    { 0 },
    NULL, NULL
  };

/* A storage slot with size and pointer to a value.  */
struct slotvec
{
  size_t size;
  char *val;
};

/* Flags for use in set_quoting_flags.  */
enum quoting_flags
  {
    /* Always elide null bytes from styles that do not quote them,
       even when the length of the result is available to the
       caller.  */
    QA_ELIDE_NULL_BYTES = 0x01,

    /* Omit the surrounding quote characters if no escaped characters
       are encountered.  Note that if no other character needs
       escaping, then neither does the escape character.  */
    QA_ELIDE_OUTER_QUOTES = 0x02,

    /* In the c_quoting_style and c_maybe_quoting_style, split ANSI
       trigraph sequences into concatenated strings (for example,
       "?""?/" rather than "??/", which could be confused with
       "\\").  */
    QA_SPLIT_TRIGRAPHS = 0x04
  };

#ifndef _GL_INLINE_HEADER_BEGIN
 #error "Please include config.h first."
#endif
_GL_INLINE_HEADER_BEGIN
#ifndef XALLOC_INLINE
# define XALLOC_INLINE _GL_INLINE
#endif

/* Preallocate a slot 0 buffer, so that the caller can always quote
   one small component of a "memory exhausted" message in slot 0.  */
static char slot0[256];
static unsigned int nslots = 1;
static struct slotvec slotvec0 = {sizeof slot0, slot0};
static struct slotvec *slotvec = &slotvec0;

/* Use storage slot N to return a quoted version of argument ARG.
   ARG is of size ARGSIZE, but if that is SIZE_MAX, ARG is a
   null-terminated string.
   OPTIONS specifies the quoting options.
   The returned value points to static storage that can be
   reused by the next call to this function with the same value of N.
   N must be nonnegative.  N is deliberately declared with type "int"
   to allow for future extensions (using negative values).  */
static char *
quotearg_n_options (int n, char const *arg, size_t argsize,
                    struct quoting_options const *options)
{
  int e = errno;

  unsigned int n0 = n;
  struct slotvec *sv = slotvec;

  if (n < 0)
    abort ();

  if (nslots <= n0)
    {
      /* FIXME: technically, the type of n1 should be 'unsigned int',
         but that evokes an unsuppressible warning from gcc-4.0.1 and
         older.  If gcc ever provides an option to suppress that warning,
         revert to the original type, so that the test in xalloc_oversized
         is once again performed only at compile time.  */
      size_t n1 = n0 + 1;
      bool preallocated = (sv == &slotvec0);

      if (xalloc_oversized (n1, sizeof *sv))
        xalloc_die ();

      slotvec = sv = xrealloc (preallocated ? NULL : sv, n1 * sizeof *sv);
      if (preallocated)
        *sv = slotvec0;
      memset (sv + nslots, 0, (n1 - nslots) * sizeof *sv);
      nslots = n1;
    }

  {
    size_t size = sv[n].size;
    char *val = sv[n].val;
    /* Elide embedded null bytes since we don't return a size.  */
    int flags = options->flags | QA_ELIDE_NULL_BYTES;
    size_t qsize = quotearg_buffer_restyled (val, size, arg, argsize,
                                             options->style, flags,
                                             options->quote_these_too,
                                             options->left_quote,
                                             options->right_quote);

    if (size <= qsize)
      {
        sv[n].size = size = qsize + 1;
        if (val != slot0)
          free (val);
        sv[n].val = val = xcharalloc (size);
        quotearg_buffer_restyled (val, size, arg, argsize, options->style,
                                  flags, options->quote_these_too,
                                  options->left_quote,
                                  options->right_quote);
      }

    errno = e;
    return val;
  }
}
char const *
quote_n_mem (int n, char const *arg, size_t argsize)
{
  return quotearg_n_options (n, arg, argsize, &quote_quoting_options);
}

char const *
quote_mem (char const *arg, size_t argsize)
{
  return quote_n_mem (0, arg, argsize);
}

char const *
quote_n (int n, char const *arg)
{
  return quote_n_mem (n, arg, SIZE_MAX);
}

char const *
quote (char const *arg)
{
  return quote_n (0, arg);
}



/* Redirection and wildcarding when done by the utility itself.
   Generally a noop, but used in particular for native VMS. */
#ifndef initialize_main
# define initialize_main(ac, av)
#endif

/* Set program_name, based on argv[0].
   argv0 must be a string allocated with indefinite extent, and must not be
   modified after this call.  */
extern void set_program_name (const char *argv0);
/* #define set_program_name(ARG0) \ */
/*   set_program_name_and_installdir (ARG0, INSTALLPREFIX, INSTALLDIR) */

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
			errmsg = re_compile_pattern (optarg, strlen (optarg), regexp); if (errmsg)
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
	/* setlocale (LC_ALL, ""); */
	/* bindtextdomain (PACKAGE, LOCALEDIR); */
	/* textdomain (PACKAGE); */

	/* atexit (close_stdout); */

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
