/* --------------------------------------------------------------------------
 * Definition of the Hugs server API
 *
 * The Hugs 98 system is Copyright (c) Mark P Jones, Alastair Reid, the
 * Yale Haskell Group, and the OGI School of Science & Engineering at OHSU,
 * 1994-2003, All rights reserved.  It is distributed as free software under
 * the license in the file "License", which is included in the distribution.
 *
 * $RCSfile: server.h,v $
 * $Revision: 1.12 $
 * $Date: 2003/10/14 13:56:25 $
 * ------------------------------------------------------------------------*/
#include "HugsAPI.h"
#include "storage.h"

/* These have non-local scope, as they're used when creating 
 * extended/delegated versions of the server API (cf. the server
 * interface provided by the .NET extensions.)
 */
extern Void   setError        (String);
extern Void   startEval       (Void);
extern Bool   safeEval        (Cell);
extern String ClearError      (Void);
extern Cell   getTypeableDict (Type);
extern char*  lastError;

/* Get the API method table from the currently running interpreter.
 * => the interpreter / server is assumed to have already been initialized.
 */
extern HugsServerAPI* getHugsAPI (Void);
/* ------------------------------------------------------------------------*/
