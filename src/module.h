/* --------------------------------------------------------------------------
 * Haskell 98 module system implementation for Hugs.
 *
 * The Hugs 98 system is Copyright (c) Mark P Jones, Alastair Reid, the
 * Yale Haskell Group, and the OGI School of Science & Engineering at OHSU,
 * 1994-2003, All rights reserved.  It is distributed as free software under
 * the license in the file "License", which is included in the distribution.
 *
 * -sof 2002.
 * 
 * ------------------------------------------------------------------------*/
#ifndef __MODULE_H__
#define __MODULE_H__

#include "prelude.h"
#include "storage.h"

extern Void addQualImport       (Text,Text,List);
extern Void addUnqualImport     (Text,Text,List);
extern Void importName          (Module,Name);
extern Void checkImportList     (Bool,Pair);
extern Void checkQualImportList (Pair);
extern Void fixupImportExports  (List);
extern List checkExports        (List);
extern Void browseModule	(Module,Bool);

#endif /* __MODULE_H__ */
