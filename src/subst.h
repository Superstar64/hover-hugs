/* --------------------------------------------------------------------------
 * Definitions for substitution data structure and operations.
 *
 * The Hugs 98 system is Copyright (c) Mark P Jones, Alastair Reid, the
 * Yale Haskell Group, and the OGI School of Science & Engineering at OHSU,
 * 1994-2003, All rights reserved.  It is distributed as free software under
 * the license in the file "License", which is included in the distribution.
 *
 * $RCSfile: subst.h,v $
 * $Revision: 1.11 $
 * $Date: 2004/11/04 00:01:31 $
 * ------------------------------------------------------------------------*/
#pragma once

#include "prelude.h"
#include "storage.h"

typedef struct {			/* Each type variable contains:	   */
    Type bound;				/* A type skeleton (unbound==NIL)  */
    Int  offs;				/* Offset for skeleton		   */
    Kind kind;				/* kind annotation		   */
} Tyvar;

extern  Tyvar		*tyvars;	/* storage for type variables	   */
extern	Int		typeOff;	/* offset of result type 	   */
extern	Type		typeIs;		/* skeleton of result type	   */
extern  Int		typeFree;	/* freedom in instantiated type	   */
extern	List		predsAre;	/* list of predicates in type	   */
extern  List		genericVars;	/* list of generic vars		   */
extern  List		btyvars;	/* explicitly scoped type vars	   */

#define tyvar(n)	(tyvars+(n))	/* nth type variable		   */
#define tyvNum(t)	((t)-tyvars)	/* and the corresp. inverse funct. */
#define isBound(t)	(((t)->bound) && ((t)->bound!=SKOLEM))
#define aVar		mkOffset(0)	/* Simple skeletons for type vars  */
#define bVar		mkOffset(1)
#define enterBtyvs()	btyvars = cons(NIL,btyvars)
#define leaveBtyvs()	btyvars = tl(btyvars)

#define deRef(tyv,t,o)  while ((tyv=getTypeVar(t,o)) && isBound(tyv)) { \
			    t = tyv->bound;                             \
			    o = tyv->offs;                              \
			}

					/* offs values when isNull(bound): */
#define FIXED_TYVAR	0	        /* fixed in current assumption	   */
#define UNUSED_GENERIC	1	        /* not fixed, not yet encountered  */
#define GENERIC		2	        /* GENERIC+n==nth generic var found*/

extern  char		*unifyFails;	/* Unification error message	   */

extern Void  emptySubstitution	(Void);
extern Int   newTyvars		(Int);
#define      newKindvars(n)	newTyvars(n)
extern Int   newKindedVars	(Kind);
extern Void  instantiate	(Type);

extern Pair  findBtyvs		(Text);
extern Void  markBtyvs		(Void);
extern Type  localizeBtyvs	(Type);

extern Tyvar *getTypeVar	(Type,Int);
extern Void  tyvarType		(Int);
extern Void  bindTv		(Int,Type,Int);
extern Cell  getDerefHead	(Type,Int);
extern Void  expandSyn		(Tycon, Int, Type *, Int *);

extern Void  clearMarks		(Void);
extern Void  markAllVars	(Void);
extern Void  resetGenerics	(Void);
extern Void  markTyvar		(Int);
extern Void  markType		(Type,Int);
extern Void  markPred		(Cell);

extern Type  copyTyvar		(Int);
extern Type  copyType		(Type,Int);
extern Cell  copyPred		(Cell,Int);
extern Type  dropRank2		(Type,Int,Int);
extern Type  dropRank1		(Type,Int,Int);
extern Void  liftRank2Args	(List,Int,Int);
extern Type  liftRank2		(Type,Int,Int);
extern Type  liftRank1		(Type,Int,Int);

extern Type  debugTyvar		(Int);
extern Type  debugType		(Type,Int);
extern List  debugContext	(List);
extern Cell  debugPred		(Cell,Int);

extern Kind  copyKindvar	(Int);
extern Kind  copyKind		(Kind,Int);

extern Bool  eqKind		(Kind,Kind);
extern Kind  getKind		(Cell,Int);

extern List  genvarTyvar	(Int,List);
extern List  genvarType		(Type,Int,List);

extern Bool  doesntOccurIn	(Tyvar*,Type,Int);
extern Bool  unify		(Type,Int,Type,Int);
extern Bool  kunify		(Kind,Int,Kind,Int);

extern Void  typeTuple		(Cell);
extern Kind  simpleKind		(Int);
extern Void  varKind		(Int);

extern Bool  samePred		(Cell,Int,Cell,Int);
extern Bool  matchPred		(Cell,Int,Cell,Int);
extern Bool  unifyPred		(Cell,Int,Cell,Int);
extern Inst  findInstFor	(Cell,Int);

extern Void  improve		(Int,List,List);
extern Void  improve1		(Int,List,Cell,Int);

extern Bool  sameSchemes	(Type,Type);
extern Bool  sameType		(Type,Int,Type,Int);
extern Bool  matchType		(Type,Int,Type,Int);

/*-------------------------------------------------------------------------*/