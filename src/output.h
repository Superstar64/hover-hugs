/* --------------------------------------------------------------------------
 * Unparse expressions and types - for use in error messages, type checker
 * and for debugging.
 *
 * The Hugs 98 system is Copyright (c) Mark P Jones, Alastair Reid, the
 * Yale Haskell Group, and the OGI School of Science & Engineering at OHSU,
 * 1994-2003, All rights reserved.  It is distributed as free software under
 * the license in the file "License", which is included in the distribution.
 *
 * ------------------------------------------------------------------------*/
#ifndef __OUTPUT_H__
#define __OUTPUT_H__

#include "prelude.h"
#include "storage.h"

extern Void printExp     (FILE *,Cell);
extern Void printType    (FILE *,Cell);
extern Void printContext (FILE *,List);
extern Void printPred    (FILE *,Cell);
extern Void printKind	 (FILE *,Kind);
extern Void printKinds	 (FILE *,Kinds);
extern Void printFD	 (FILE *,Pair);

#if OBSERVATIONS
#define ALLTAGS ""
extern Void printObserve (String);
#endif

#if DEBUG_SHOWSC
extern Void  printSc (FILE*, Text, Int, Cell);
#endif


#endif /* __OUTPUT_H__ */
