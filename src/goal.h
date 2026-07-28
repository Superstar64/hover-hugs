/*
 * Keeping track of progress towards a goal.
 */
#pragma once

#include "prelude.h"

typedef long   Target;
extern Void setGoal (String,Target);
extern Void soFar   (Target);
extern Void done    (Void);
extern Void failed  (Void);
