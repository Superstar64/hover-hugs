/* --------------------------------------------------------------------------
 * Definition of the external Hugs server API
 *
 * The Hugs 98 system is Copyright (c) Mark P Jones, Alastair Reid, the
 * Yale Haskell Group, and the OGI School of Science & Engineering at OHSU,
 * 1994-2003, All rights reserved.  It is distributed as free software under
 * the license in the file "License", which is included in the distribution.
 *
 * $RCSfile: HugsAPI.h,v $
 * $Revision: 1.3 $
 * $Date: 2004/02/04 17:53:38 $
 * ------------------------------------------------------------------------*/
#pragma once

#include "prelude.h"

#ifdef __cplusplus 
extern "C" {
#endif

typedef int HVal;     /* Haskell values are represented by stable pointers */

typedef struct _HugsServerAPI {
    char* (*clearError     ) (void);
    void  (*setHugsArgs    ) (int, char**);
    int   (*getNumScripts  ) (void);
    void  (*reset          ) (int);
    void  (*setOutputEnable) (unsigned);
    void  (*changeDir      ) (char*);
    void  (*loadProject    ) (char*);
    void  (*loadFile       ) (char*);
    void  (*loadFromBuffer ) (char*);
    void  (*setOptions     ) (char*);
    char* (*getOptions     ) (void);
    HVal  (*compileExpr    ) (char*,char*);
    void  (*garbageCollect ) (void);

    void  (*lookupName     ) (char*,char*); /* push values onto stack*/

    void  (*mkInt          ) (int);
    void  (*mkAddr         ) (void*);
    void  (*mkString       ) (char*);

    void  (*apply          ) (void);      /* manipulate top of stack */

    int   (*evalInt        ) (void);      /* evaluate top of stack   */
    void* (*evalAddr       ) (void);
    char* (*evalString     ) (void);

    int   (*doIO           ) (void);
    int   (*doIO_Int       ) (int*);
    int   (*doIO_Addr      ) (void**);

    HVal  (*popHVal        ) (void);      /* pop stack               */
    void  (*pushHVal       ) (HVal);      /* push back onto stack    */
    void  (*freeHVal       ) (HVal); 
} HugsServerAPI;

DLLEXPORT(HugsServerAPI*) initHugsServer (Int, String[]);
DLLEXPORT(Void) shutdownHugsServer (HugsServerAPI*);

#ifdef __cplusplus 
};
#endif
