/*
 * The Hugs evaluator / command interpreter + support functions.
 *
 * The Hugs 98 system is Copyright (c) Mark P Jones, Alastair Reid, the
 * Yale Haskell Group, and the OGI School of Science & Engineering at OHSU,
 * 1994-2003, All rights reserved.  It is distributed as free software under
 * the license in the file "License", which is included in the distribution.
 *
 */
#ifndef __EVALUATOR_H__
#define __EVALUATOR_H__

#include "prelude.h"
#include "storage.h"

extern Void startEvaluator    (Void);
extern Void stopEvaluator     (Void);
extern Void evaluator         (Module);
extern Void everybody         (Int);
extern Void loadPrelude       (Void);

#endif /* __EVALUATOR_H__ */
