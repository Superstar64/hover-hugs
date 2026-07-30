#pragma once

#include <stdbool.h>

// Hover Hugs configuration file

// {Home} gets substituted for the user's home directory
// {Hugs} gets substituted for the `<executable folder>/../lib/hugs`

static const char HUGSPATH[] = ".:{Home}/lib/hugs/packages/*:{Hugs}/packages/*";
static const char HUGSSUFFIXES[] = ".hs:.lhs";

/* Define to true if you want the small startup banner. */
enum { SMALL_BANNER = false };

/* Define to true if you want filenames to be converted to normal form by
   replacing relative pathnames with absolute pathnames and eliminating .. and
   . where possible. */
enum { PATH_CANONICALIZATION = false };

/* If you get really desperate to understand why your Hugs programs keep
   crashing or running out of stack, you might like to set this flag and
   recompile Hugs. When you hit a stack error, it will print out a list of all
   the objects currently under evaluation. The information isn't perfect and
   can be pretty hard to understand but it's better than a poke in the eye
   with a blunt stick. This is a very experimental feature! */
enum { GIMME_STACK_DUMPS = false };


/* Define to true if heap profiler can (and should) automatically invoke hp2ps to
   convert heap profile (in "profile.hp") to PostScript. */
enum { HAVE_HP2PS = false };

/* Define to true if heap profiling should be used. */
enum { PROFILING = false };


// Debug Options

enum { DEBUG_BIGNUMS = false };
enum { VERBOSE_PRINT = false };

/* Define to true if you want to perform runtime tag-checks as an internal
   consistency check. This makes Hugs run very slowly - but is very effective
   at detecting and locating subtle bugs. */
enum { CHECK_TAGS = false };

enum { DEBUG_TYPES = false };
enum { DEBUG_KINDS = false };
enum { DEBUG_DEFAULTS = false };
enum { DEBUG_SELS = false };
enum { DEBUG_DEPENDS = false };
enum { DEBUG_DERIVING = false };
enum { DEBUG_SEARCH = false };
enum { DEBUG_SHOWUSE = false };