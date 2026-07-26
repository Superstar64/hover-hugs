/*
 * Maintaining a stack of files / scripts.
 * 
 * The Hugs 98 system is Copyright (c) Mark P Jones, Alastair Reid, the
 * Yale Haskell Group, and the OGI School of Science & Engineering at OHSU,
 * 1994-2003, All rights reserved.  It is distributed as free software under
 * the license in the file "License", which is included in the distribution.
 *
 */
#ifndef __SCRIPT_H__
#define __SCRIPT_H__

#include "prelude.h"
#include "storage.h"

/*
 * The Script module handles the management of the stack of scripts
 * known to the interpreter. 
 *
 * When the interpreter processes a module, it adds the module's
 * imports to the script stack which are then subsequently loaded in.
 * Assuming the imported modules were all found and loaded without error,
 * the original module and its imports is then processed again, 
 * but this time all the imports are found on the stack of scripts
 * already loaded in.
 *
 */

extern Void initScripts         (Void);
extern Void stopScripts         (Void);
extern Void setScriptStableMark (Void);

extern String getScriptName     (Script);
extern String getScriptRealName (Script);
extern Int    getScriptHwMark   (Void);
extern Int    numLoadedScripts  (Void);

extern Void addScriptName       (String,Bool);
extern Bool chase               (List);
extern Void addScriptsFromArgs  (Int,String []);

extern Void forgetScriptsFrom (Script);
extern Void forgetAllScripts  (Void);
extern Void forgetAScript     (Script);

extern String scriptFile;

extern Void whatScripts       (Void);
extern Void readScripts       (Int);

extern Void script     (int);

#if HUGS_FOR_WINDOWS
extern Void setNumLoadedScripts (Script);
extern Void setScriptHwMark     (Script);
extern Void setScriptName       (Script,String);
extern Void setScriptRealName   (Script,String);
#endif


#endif /* __SCRIPT_H__ */
