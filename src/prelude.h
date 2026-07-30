/* --------------------------------------------------------------------------
 * Basic data type definitions, prototypes and standard macros including
 * machine dependent variations...
 *
 * The Hugs 98 system is Copyright (c) Mark P Jones, Alastair Reid, the
 * Yale Haskell Group, and the OGI School of Science & Engineering at OHSU,
 * 1994-2003, All rights reserved.  It is distributed as free software under
 * the license in the file "License", which is included in the distribution.
 *
 * $RCSfile: prelude.h,v $
 * $Revision: 1.80 $
 * $Date: 2005/09/11 00:33:00 $
 * ------------------------------------------------------------------------*/
#pragma once

#include "config.h"
#include <stdio.h>

# include <stdlib.h>
# include <sys/types.h>
# include <unistd.h>

#define HUGS_PROPER 1

/*---------------------------------------------------------------------------
 * Most of the configuration code from earlier versions of Hugs has been moved
 * into config.h (which is usually automatically generated).  
 *
 * Most of the configuration code is "feature based".  That is, the 
 * configure script looks to see if a particular feature (or misfeature)
 * is present on the compiler/OS.  
 *
 * A small amount of configuration code is still "system based": it tests
 * flags to determine what kind of compiler/system it's running on - from
 * which it infers what features the compiler/system has.  Use of system
 * based tests generally indicates that we can't remember/figure out
 * what the original problem was and so we can't add an appropriate feature
 * test to the configure script.
 *-------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
 * Platform-dependent settings:
 *-------------------------------------------------------------------------*/



/*---------------------------------------------------------------------------
 * Include stuff required for WinHugs:
 *  mainly redirect get/put console functions
 *  also required API definitions
 *-------------------------------------------------------------------------*/



/*---------------------------------------------------------------------------
 * Macros used in declarations:
 *  function prototypes
 *  local/far declarations
 *  HUGS_noreturn/HUGS_unused (prevent spurious warnings)
 *  result type of main
 *  dynamic linking declarations
 *-------------------------------------------------------------------------*/

/* local = prefix for locally defined functions */
/* far   = prefix for far pointers              */
# define local
# define far

#ifdef __GNUC__     /* Avoid spurious warnings                             */
#if __GNUC__ > 2 || __GNUC__ == 2 && __GNUC_MINOR__ >= 7
#define HUGS_noreturn  __attribute__ ((noreturn))
#define HUGS_unused    __attribute__ ((unused))
#else
#define HUGS_noreturn  
#define HUGS_unused
#endif
#else
#define HUGS_noreturn  
#define HUGS_unused
#endif

/*---------------------------------------------------------------------------
 * Dynamic linking tricks
 *-------------------------------------------------------------------------*/


# define DLLIMPORT(rty) rty
# define DLLEXPORT(rty) rty

#define HUGSAPI(rty) DLLEXPORT(rty)

/*---------------------------------------------------------------------------
 * String operations:
 *-------------------------------------------------------------------------*/

# include <string.h>
#define strCompare strcmp

# include <ctype.h>
#ifndef isascii
#define  isascii(c)	(((unsigned)(c))<128)
#endif

/*---------------------------------------------------------------------------
 * Printf-related operations:
 *-------------------------------------------------------------------------*/

#include <stdarg.h>

/*---------------------------------------------------------------------------
 * Interrupting execution (signals, allowBreak):
 *-------------------------------------------------------------------------*/

# define sigProto(nm)	void nm (int)
# define sigRaise(nm)	nm(1)
# define sigHandler(nm)	void nm(sig_arg) int sig_arg;
# define sigResume	return

/* allowBreak: call to allow user to interrupt computation
 * ctrlbrk:    set control break handler
 */

#  include <signal.h>
#  define ctrlbrk(bh)	{ sigset_t mask; \
			  signal(SIGINT,bh); \
			  sigemptyset(&mask); \
			  sigaddset(&mask, SIGINT); \
			  sigprocmask(SIG_UNBLOCK, &mask, NULL); \
			}
# define allowBreak()   doNothing()

/*---------------------------------------------------------------------------
 * Floating point support
 *-------------------------------------------------------------------------*/

#define FloatImpType	   float
#define FloatPro	   double  /* type to use in prototypes		   */
				   /* strictly ansi (i.e. gcc) conforming  */
				   /* but breaks data hiding :-(	   */
#define FloatFMT	   "%.7g"

#define DoubleImpType	   double
#define DoublePro	   double
#define DoubleFMT          "%.15g"

#include <float.h>

# define HUGS_FLT_RADIX    FLT_RADIX
# define HUGS_FLT_MANT_DIG FLT_MANT_DIG
# define HUGS_FLT_MIN_EXP  FLT_MIN_EXP
# define HUGS_FLT_MAX_EXP  FLT_MAX_EXP

# define HUGS_DBL_MANT_DIG DBL_MANT_DIG
# define HUGS_DBL_MIN_EXP  DBL_MIN_EXP
# define HUGS_DBL_MAX_EXP  DBL_MAX_EXP



/*---------------------------------------------------------------------------
 * Memory allocation
 *-------------------------------------------------------------------------*/

# define farCalloc(n,s)	(Void *)valloc(((unsigned)n)*((unsigned)s))

/* bison-generated parsers like to have alloca - so try to define it */



/*---------------------------------------------------------------------------
 * Assertions
 *-------------------------------------------------------------------------*/

#include <assert.h>

/*---------------------------------------------------------------------------
 * Preprocessor support
 *-------------------------------------------------------------------------*/
 
/*---------------------------------------------------------------------------
 * Environment variables and the registry
 *-------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
 * File operations:
 *-------------------------------------------------------------------------*/

# define filenamecmp(s1,s2) strcmp(s1,s2)

/*---------------------------------------------------------------------------
 * General settings:
 *-------------------------------------------------------------------------*/

#define Void     void   /* older compilers object to: typedef void Void;   */
typedef unsigned Bool;
#define TRUE     1
#define FALSE    0
typedef char    *String;
typedef int      Int;
typedef signed char Int8;
typedef short    Int16;
typedef long     Long;
typedef int      Char;
typedef unsigned Unsigned;
typedef void*    Pointer;

#define doNothing() do { } while (0) /* Null statement */

#ifndef STD_PRELUDE
#define STD_PRELUDE	      "Prelude"
#define STD_PRELUDE_HUGS      "Hugs.Prelude"
#define STD_EMPTY_MODULE      "Hugs"
#endif

#define NUM_HANDLES	   40
#define NUM_TUPLES         100
#define NUM_OFFSETS        1024
#define NUM_LAT1_CHARS     256
#define NUM_EXT		   100

#define DEF_PROFINTDIV	   10		/* hpsize/this cells between samples*/

#define NUM_OBS_TAGS       1000
#define NUM_BRKPTS         200

#define NUM_SCRIPTS        800
#define NUM_FIXUPS         1000
#define NUM_TYCON          2000
#define NUM_NAME           32000
#define NUM_CLASSES        1000
#define NUM_INSTS          4000
#define NUM_TEXT           320000
#define NUM_TEXTH	       10
#define NUM_TYVARS         8000
#define NUM_STACK          160000
#define NUM_ADDRS          1280000
#define MINIMUMHEAP	       19000
#define MAXIMUMHEAP	   	   0
#define DEFAULTHEAP        1000000
#define MAXPOSINT          0x7fffffff
#define MAXHUGSWORD        0xffffffffU
#define NUM_STABLEPTRS	   10000
#define NUM_MALLOCPTRS	   10000
#define NUM_DTUPLES	       5

/* Some infinite computations generate an infinite depth of alternations
 * between eval() and run().  If they don't use the Hugs stack or heap,
 * they will overrun the C stack, crashing the interpreter.  To protect
 * against this, we place a limit on the depth of recursion of eval().
 */
#define MAX_EVAL_DEPTH	   16384

/* Representation of Integer: requires BIGBASE == 10^BIGEXP */
#define BIGBASE		       10000
#define BIGEXP		       4

#define MINNEGINT          (-MAXPOSINT-1)

#define NUM_MODULE         NUM_SCRIPTS


#define DECTABLE(tab)	   tab[]
#define DYNDECTABLE(tab)   far *tab
#define DEFTABLE(tab,sz)   tab[sz]

#define minRecovery	       1000
#define bitsPerWord	       32
#define wordShift	       5
#define wordMask	       31

#define bitArraySize(n)    ((n)/bitsPerWord + 1)
#define placeInSet(n)      ((-(n)-1)>>wordShift)
#define maskInSet(n)       (1<<((-(n)-1)&wordMask))

/*-------------------------------------------------------------------------*/
