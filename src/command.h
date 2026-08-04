/* --------------------------------------------------------------------------
 * Interpreter command structure
 *
 * The Hugs 98 system is Copyright (c) Mark P Jones, Alastair Reid, the
 * Yale Haskell Group, and the OGI School of Science & Engineering at OHSU,
 * 1994-2003, All rights reserved.  It is distributed as free software under
 * the license in the file "License", which is included in the distribution.
 *
 * $RCSfile: command.h,v $
 * $Revision: 1.15 $
 * $Date: 2006/01/10 23:31:44 $
 * ------------------------------------------------------------------------*/
#pragma once

#include "prelude.h"

typedef Int Command;

struct cmd {
    String cmdString;
    Command cmdCode;
};

extern Command readCommand (struct cmd *, Char, Char);

enum { EDIT =    0 };
enum { FIND =    1 };
enum { LOAD =    2 };
enum { ALSO =    3 };
enum { RELOAD =  4 };
enum { EVAL =    5 };
enum { TYPEOF =  6 };
enum { HELP =    7 };
enum { NAMES =   8 };
enum { BADCMD =  9 };
enum { SET =     10 };
enum { QUIT =    11 };
enum { SYSTEM =  12 };
enum { CHGDIR =  13 };
enum { INFO =    14 };
enum { COLLECT = 15 };
enum { SETMODULE = 16 };
enum { BROWSE =  17 };
enum { XPLAIN =  18 };
enum { PNTVER =  19 };
enum { NOCMD =   20 };
enum { MAIN =    22 };

/*-------------------------------------------------------------------------*
 * Commands available after breakpoint                                     *
 *-------------------------------------------------------------------------*/
enum { BRK_DISPLAY =     0 };
enum { BRK_CONTINUE =    1 };
enum { BRK_SET =         2 };
enum { BRK_RESET =       3 };

/*-------------------------------------------------------------------------*/