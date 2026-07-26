/*
 * Keeping track of progress towards a goal.
 */
#ifndef __GOAL_H__
#define __GOAL_H__

#include "prelude.h"

typedef long   Target;
extern Void setGoal (String,Target);
extern Void soFar   (Target);
extern Void done    (Void);
extern Void failed  (Void);

#endif /* __GOAL_H__ */
