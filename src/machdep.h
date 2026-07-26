/* --------------------------------------------------------------------------
 * Machine dependent code
 *
 * The Hugs 98 system is Copyright (c) Mark P Jones, Alastair Reid, the
 * Yale Haskell Group, and the OGI School of Science & Engineering at OHSU,
 * 1994-2003, All rights reserved.  It is distributed as free software under
 * the license in the file "License", which is included in the distribution.
 *
 * ------------------------------------------------------------------------*/
#ifndef __MACHDEP_H__
#define __MACHDEP_H__
#include "prelude.h"

# include <time.h>

#if USE_REGISTRY
# if HAVE_WINDOWS_H
#include <windows.h>
# endif
#endif

/* --------------------------------------------------------------------------
 * Find information about a file:
 * ------------------------------------------------------------------------*/

#if RISCOS
typedef struct { unsigned hi, lo; } Time;
#define timeChanged(now,thn)    (now.hi!=thn.hi || now.lo!=thn.lo)
#define timeSet(var,tm)         var.hi = tm.hi; var.lo = tm.lo
#else
typedef time_t Time;
#define timeChanged(now,thn)    (now!=thn)
#define timeSet(var,tm)         var = tm
#endif

extern Void getFileInfo (String, Time *, Long *);

/* --------------------------------------------------------------------------
 * Prototypes for registry reading
 * ------------------------------------------------------------------------*/

#if USE_REGISTRY

#if HUGS_FOR_WINDOWS
extern Int    readRegInt     (String,Int);
extern Bool   writeRegInt    (String,Int);
#endif

extern String readRegString       (HKEY, String, String, String);
extern Bool   writeRegString      (String,String);
extern String readRegChildStrings (HKEY, String, String, String);
#endif /* USE_REGISTRY */

/* --------------------------------------------------------------------------
 * Search for script files on the HUGS path:
 * ------------------------------------------------------------------------*/

extern String RealPath  (String);
extern String substPath (String,String);
extern String uniqPath  (String);
#if !HUGS_SERVER
extern Bool startEdit   (Int,String);
#endif

extern  Int    shellEsc		(String,Bool,Bool);
extern  Int    getTerminalWidth (Void);
extern  Void   normalTerminal	(Void);
extern  Bool   getEchoTerminal	(Int);
extern  Void   setEchoTerminal	(Int,Bool);
extern  Bool   getBuffTerminal	(Int);
extern  Void   setBuffTerminal	(Int,Bool);
extern  Void   gcStarted	(Void);
extern  Void   gcScanning	(Void);
extern  Void   gcRecovered	(Int);
extern  Void   gcCStack		(Void);
extern  Void   needPrims        (Int,void*); 

extern  String fromEnv		(String,String);
extern  Bool   initSystem       (Void);

extern	String dirname		(String);

#endif /* __MACHDEP_H__ */
