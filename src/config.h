#pragma once

#include <stdbool.h>

// Hover Hugs configuration file

// {Home} gets substituted for the user's home directory
// {Hugs} gets substituted for the `<executable folder>/../lib/hugs`

#define HUGSPATH ".:{Home}/lib/hugs/packages/*:{Hugs}/packages/*"
#define HUGSSUFFIXES ".hs:.lhs"

enum { SMALL_BANNER = false };


// Flags that were preserved from the autoconf script (hopefully at least)

/* Define to 1 if you want to use the primitives which let you examine Hugs
   bytecodes (requires INTERNAL_PRIMS). */
/* #undef BYTECODE_PRIMS */

/* Define if you want to use a low-level printer from within a debugger. */
/* #undef DEBUG_PRINTER */

/* If you get really desperate to understand why your Hugs programs keep
   crashing or running out of stack, you might like to set this flag and
   recompile Hugs. When you hit a stack error, it will print out a list of all
   the objects currently under evaluation. The information isn't perfect and
   can be pretty hard to understand but it's better than a poke in the eye
   with a blunt stick. This is a very experimental feature! */
/* #undef GIMME_STACK_DUMPS */

/* Define to 1 if heap profiler can (and should) automatically invoke hp2ps to
   convert heap profile (in "profile.hp") to PostScript. */
// #define HAVE_HP2PS 1

/* Define to 1 if compiler supports gcc's "labels as values" (aka computed
   goto) feature (which is used to speed up instruction dispatch in the
   interpreter). */
// #define HAVE_LABELS_AS_VALUES 1

/* Define to 1 if jmpbufs can be treated like arrays. */
// #define JMPBUF_ARRAY 1

/* Define to 1 if you want to use the primitives which let you examine Hugs
   internals. */
/* #undef INTERNAL_PRIMS */

/* Define to 1 if you want filenames to be converted to normal form by: (a)
   replacing relative pathnames with absolute pathnames and eliminating .. and
   . where possible. (b) converting to lower case (only in case-insensitive
   filesystems) */
/* #undef PATH_CANONICALIZATION */


/* Define to 1 if heap profiling should be used. */
/* #undef PROFILING */


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