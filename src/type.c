/* --------------------------------------------------------------------------
 * This is the Hugs type checker
 *
 * The Hugs 98 system is Copyright (c) Mark P Jones, Alastair Reid, the
 * Yale Haskell Group, and the OGI School of Science & Engineering at OHSU,
 * 1994-2003, All rights reserved.  It is distributed as free software under
 * the license in the file "License", which is included in the distribution.
 *
 * $RCSfile: type.c,v $
 * $Revision: 1.81 $
 * $Date: 2005/12/10 11:25:13 $
 * ------------------------------------------------------------------------*/

#include "config.h"
#include "prelude.h"
#include "storage.h"
#include "connect.h"
#include "errors.h"
#include "output.h"
#include "subst.h"
#include "goal.h"
#include "opts.h"	/* needed for EXPLAIN_INSTANCE_RESOLUTION|MULTI_INST */

Bool catchAmbigs       = FALSE;		/* TRUE => functions with ambig.   */
					/* 	   types produce error	   */

Type typeArrow,   typeList;		/* Important primitive types	   */
Type typeUnit;

Module modulePrelude;
Module moduleUserPrelude;

Type typeInt;                          
Type typeInt8;                          
Type typeInt16;                          
Type typeInt32;                          
Type typeInt64;                          
Type typeWord;
Type typeWord8;
Type typeWord16;
Type typeWord32;
Type typeWord64;
Type typeFunPtr;
Type typePtr;
Type typeAddr;
Type typeFloat;
Type typeDouble;
Type typeChar;
Type typeForeignP;
Type typeForeign;
Type typeStable;
Type typeBool;
Type typeString;

static Type typeInteger; 
static Type typeMaybe;
static Type typeOrdering;

Class classEq,    classOrd;		/* `standard' classes		   */
Class classIx,    classEnum;
Class classShow,  classRead;
Class classBounded;

Class classReal,       classIntegral;	/* `numeric' classes		   */
Class classRealFrac,   classRealFloat;
Class classFractional, classFloating;
Class classNum;

List stdDefaults;			/* standard default values	   */

Name nameFromInt, nameFromDouble;	/* coercion of numerics		   */
Name nameFromInteger;
Name nameEq,      nameCompare;		/* derivable names		   */
Name nameLe;
Name nameShowsPrec;
Name nameReadsPrec;
Name nameMinBnd,  nameMaxBnd;
Name nameIndex,	  nameInRange;
Name nameRange;
Name nameMult,	  namePlus;
Name nameTrue,	  nameFalse;		/* primitive boolean constructors  */
Name nameNil,     nameCons;		/* primitive list constructors	   */
Name nameJust,	  nameNothing;		/* primitive Maybe constructors	   */
Name nameLeft,    nameRight;		/* primitive Either constructors   */
Name nameUnit;				/* primitive Unit type constructor */
Name nameLT,	  nameEQ;		/* Ordering constructors	   */
Name nameGT;
Class classMonad;			/* Monads			   */
Name nameReturn,  nameBind, nameThen;   /* for translating do/monad comps  */
Name nameMFail;
Name nameGt;				/* for readsPrec		   */

Class classMonadRec;			/* Recursive monads		   */
Name nameMFix;

Type   typeIO;			        /* For the IO monad, IO 	   */
Type   typeProgIO;			/* For the IO monad, IO a	   */

Name   nameIOError, nameUserErr;	/* loosely coupled IOError cfuns   */
Name   namePermDenied;
Name   nameAlreadyExists, nameAlreadyInUse, nameDoesNotExist, nameIsFull;
Name   nameIllegal;
Name   nameEOFErr;
Name   nameProtocolError;
Name   nameArithException;
Name   nameArrayException;
Name   nameErrorCall;
Name   nameIOException;
Name   nameNoMethodError;
Name   nameNonTermination;
Name   namePatternMatchFail;
Name   nameRecConError;
Name   nameRecSelError;
Name   nameRecUpdError;

Name   nameOverflow;
Name   nameDivideByZero;
Name   nameIndexOutOfBounds;
Name   nameUndefinedElement;

Type  typeNoRow;			/* Empty row			   */
Type  typeRec;				/* Record formation		   */
Name  nameNoRec;			/* Empty record			   */



/* --------------------------------------------------------------------------
 * Local function prototypes:
 * ------------------------------------------------------------------------*/

static Void   local emptyAssumption   (Void);
static Void   local enterBindings     (Void);
static Void   local leaveBindings     (Void);
static Int    local defType	      (Cell);
static Type   local useType	      (Cell);
static Void   local markAssumList     (List);
static Cell   local findAssum         (Text);
static Pair   local findInAssumList   (Text,List);
static List   local intsIntersect     (List,List);
static List   local genvarAllAss      (List);
static List   local genvarAnyAss      (List);
static Int    local newVarsBind       (Cell);
static Void   local newDefnBind       (Cell,Type);

static Void   local enterPendingBtyvs (Void);
static Void   local leavePendingBtyvs (Void);
static Cell   local patBtyvs	      (Cell);
static Void   local doneBtyvs	      (Int);
static Void   local enterSkolVars     (Void);
static Void   local leaveSkolVars     (Int,Type,Int,Int);

static Void   local typeError         (Int,Cell,Cell,String,Type,Int);
static Void   local reportTypeError   (Int,Cell,Cell,String,Type,Type);
static Void   local cantEstablish     (Int,String,Cell,Type,List);
static Void   local tooGeneral        (Int,Cell,Type,Type);

static Cell   local typeExpr          (Int,Cell);

static Cell   local typeAp	      (Int,Cell);
static Type   local typeExpected      (Int,String,Cell,Type,Int,Int,Bool);
static Void   local typeAlt	      (String,Cell,Cell,Type,Int,Int);
static Int    local funcType          (Int);
static Void   local typeCase          (Int,Int,Cell);
static Void   local typeComp	      (Int,Type,Cell,List);
static Cell   local typeMonadComp     (Int,Cell);
static Cell   local typeZComp	      (Int,Type,Cell,List);
static Void   local typeCompy	      (Int,Type,List);
static Cell   local typeMonadZComp    (Int,Cell);
static List   local getPats           (List);
static Cell   local tupleUp           (List);
static Void   local typeDo	      (Int,Cell);
static Void   local typeRecComp	      (Int,Type,Cell,List);
static Void   local typeMDo	      (Int,Cell);
static Void   local typeRecursiveDo   (Int,Cell);
static Void   local typeConFlds	      (Int,Cell);
static Void   local typeUpdFlds	      (Int,Cell);
static Cell   local typeWith	      (Int,Cell);
static Cell   local typeFreshPat      (Int,Cell);

static Void   local typeBindings      (List);
static Void   local removeTypeSigs    (Cell);

static Void   local monorestrict      (List);
static Void   local restrictedBindAss (Cell);
static Void   local restrictedAss     (Int,Cell,Type);

static Void   local unrestricted      (List);
static List   local itbscc	      (List);
static Void   local addEvidParams     (List,Cell);

static Void   local typeClassDefn     (Class);
static Void   local typeInstDefn      (Inst);
static Void   local typeMember	      (String,Name,Cell,List,Cell,Int);

static Void   local typeBind          (Cell);
static Void   local typeDefAlt        (Int,Cell,Pair);
static Cell   local typeRhs           (Cell);
static Void   local guardedType       (Int,Cell);

static Void   local genBind	      (List,Cell);
static Void   local genAss	      (Int,List,Cell,Type);
static Type   local genTest	      (Int,Cell,List,Type,Type,Int);
static Type   local generalize	      (List,Type);
static Bool   local equalTypes        (Type,Type);

static Void   local typeDefnGroup     (List);
static Void   local typeForeignExport (Name);
static Pair   local typeSel	      (Name);

static Name   local linkName          (String);
static Tycon  local linkTycon         (String);
static Class  local linkClass         (String);

/* --------------------------------------------------------------------------
 * Frequently used type skeletons:
 * ------------------------------------------------------------------------*/

static Type  arrow;			/* mkOffset(0) -> mkOffset(1)      */
static Type  boundPair;			/* (mkOffset(0),mkOffset(0))	   */
static Type  listof;			/* [ mkOffset(0) ] 	    	   */
static Type  typeVarToVar;		/* mkOffset(0) -> mkOffset(0)  	   */

static Cell  predNum;			/* Num (mkOffset(0))		   */
static Cell  predFractional;		/* Fractional (mkOffset(0))	   */
static Cell  predIntegral;		/* Integral (mkOffset(0))	   */
static Kind  starToStar;		/* Type -> Type			   */
static Cell  predMonad;			/* Monad (mkOffset(0))		   */
static Cell  predMonadRec;		/* MonadRec/MonadFix (mkOffset(0)) */

/* --------------------------------------------------------------------------
 * Assumptions:
 *
 * A basic typing statement is a pair (Var,Type) and an assumption contains
 * an ordered list of basic typing statements in which the type for a given
 * variable is given by the most recently added assumption about that var.
 *
 * In practice, the assumption set is split between a pair of lists, one
 * holding assumptions for vars defined in bindings, the other for vars
 * defined in patterns/binding parameters etc.	The reason for this
 * separation is that vars defined in bindings may be overloaded (with the
 * overloading being unknown until the whole binding is typed), whereas the
 * vars defined in patterns have no overloading.  A form of dependency
 * analysis (at least as far as calculating dependents within the same group
 * of value bindings) is required to implement this.  Where it is known that
 * no overloaded values are defined in a binding (i.e., when the `dreaded
 * monomorphism restriction' strikes), the list used to record dependents
 * is flagged with a NODEPENDS tag to avoid gathering dependents at that
 * level.
 *
 * To interleave between vars for bindings and vars for patterns, we use
 * a list of lists of typing statements for each.  These lists are always
 * the same length.  The implementation here is very similar to that of the
 * dependency analysis used in the static analysis component of this system.
 *
 * To deal with polymorphic recursion, variables defined in bindings can be
 * assigned types of the form (POLYREC,(def,use)), where def is a type
 * variable for the type of the defining occurence, and use is a type
 * scheme for (recursive) calls/uses of the variable.
 * ------------------------------------------------------------------------*/

static List defnBounds;			/*::[[(Var,Type)]] possibly ovrlded*/
static List varsBounds;			/*::[[(Var,Type)]] not overloaded  */
static List depends;			/*::[?[Var]] dependents/NODEPENDS  */
static List skolVars;			/*::[[Var]] skolem vars		   */
static List localEvs;			/*::[[(Pred,offset,ev)]]	   */
static List savedPs;			/*::[[(Pred,offset,ev)]]	   */
static Cell dummyVar;			/* Used to put extra tvars into	ass*/

#define saveVarsAss()     List saveAssump = hd(varsBounds)
#define restoreVarsAss()  hd(varsBounds)  = saveAssump
#define addVarAssump(v,t) hd(varsBounds)  = cons(pair(v,t),hd(varsBounds))
#define findTopBinding(v) findInAssumList(textOf(v),hd(defnBounds))

static Void local emptyAssumption() {  	/* set empty type assumption	   */
    defnBounds = NIL;
    varsBounds = NIL;
    depends    = NIL;
    skolVars   = NIL;
    localEvs   = NIL;
    savedPs    = NIL;
}

static Void local enterBindings() {    /* Add new level to assumption sets */
    defnBounds = cons(NIL,defnBounds);
    varsBounds = cons(NIL,varsBounds);
    depends    = cons(NIL,depends);
}

static Void local leaveBindings() {    /* Drop one level of assumptions    */
    defnBounds = tl(defnBounds);
    varsBounds = tl(varsBounds);
    depends    = tl(depends);
}

static Int local defType(a)		/* Return type for defining occ.   */
Cell a; {				/* of a var from assumption pair  */
    return (isPair(a) && fst(a)==POLYREC) ? fst(snd(a)) : a;
}

static Type local useType(a)		/* Return type for use of a var	   */
Cell a; {				/* defined in an assumption	   */
    return (isPair(a) && fst(a)==POLYREC) ? snd(snd(a)) : a;
}

static Void local markAssumList(as)	/* Mark all types in assumption set*/
List as; {				/* :: [(Var, Type)]		   */
    for (; nonNull(as); as=tl(as)) {	/* No need to mark generic types;  */
	Type t = defType(snd(hd(as)));	/* the only free variables in those*/
	if (!isPolyType(t))		/* must have been free earlier too */
	    markType(t,0);
    }
}

static Cell local findAssum(t)	       /* Find most recent assumption about*/
Text t; {			       /* variable named t, if any	   */
    List defnBounds1 = defnBounds;     /* return translated variable, with */
    List varsBounds1 = varsBounds;     /* type in typeIs		   */
    List depends1    = depends;

    while (nonNull(defnBounds1)) {
	Pair ass = findInAssumList(t,hd(varsBounds1));/* search varsBounds */
	if (nonNull(ass)) {
	    typeIs = snd(ass);
	    return fst(ass);
	}

	ass = findInAssumList(t,hd(defnBounds1));     /* search defnBounds */
	if (nonNull(ass)) {
	    Cell v = fst(ass);
	    typeIs = snd(ass);

	    if (hd(depends1)!=NODEPENDS &&	      /* save dependent?   */
		  isNull(v=varIsMember(t,hd(depends1))))
		/* N.B. make new copy of variable and store this on list of*/
		/* dependents, and in the assumption so that all uses of   */
		/* the variable will be at the same node, if we need to    */
		/* overwrite the call of a function with a translation...  */
		hd(depends1) = cons(v=mkVar(t),hd(depends1));

	    return v;
	}

	defnBounds1 = tl(defnBounds1);		      /* look in next level*/
	varsBounds1 = tl(varsBounds1);		      /* of assumption set */
	depends1    = tl(depends1);
    }
    return NIL;
}

static Pair local findInAssumList(t,as)/* Search for assumption for var    */
Text t;				       /* named t in list of assumptions as*/
List as; {
    for (; nonNull(as); as=tl(as))
	if (textOf(fst(hd(as)))==t)
	    return hd(as);
    return NIL;
}

static List local intsIntersect(as,bs)	/* calculate intersection of lists */
List as, bs; {				/* of integers (as sets)	   */
    List ts = NIL;			/* destructively modifies as	   */
    while (nonNull(as))
	if (intIsMember(intOf(hd(as)),bs)) {
	    List temp = tl(as);
	    tl(as)    = ts;
	    ts	      = as;
	    as	      = temp;
	}
	else
	    as = tl(as);
    return ts;
}

static List local genvarAllAss(as)	/* calculate generic vars that are */
List as; {				/* in every type in assumptions as */
    List vs = genvarTyvar(intOf(defType(snd(hd(as)))),NIL);
    for (as=tl(as); nonNull(as) && nonNull(vs); as=tl(as))
	vs = intsIntersect(vs,genvarTyvar(intOf(defType(snd(hd(as)))),NIL));
    return vs;
}

static List local genvarAnyAss(as)	/* calculate generic vars that are */
List as; {				/* in any type in assumptions as   */
    List vs = genvarTyvar(intOf(defType(snd(hd(as)))),NIL);
    for (as=tl(as); nonNull(as); as=tl(as))
	vs = genvarTyvar(intOf(defType(snd(hd(as)))),vs);
    return vs;
}

static Int local newVarsBind(v)        /* make new assump for pattern var  */
Cell v; {
    Int beta = newTyvars(1);
    addVarAssump(v,mkInt(beta));
if (DEBUG_TYPES) {
    Printf("variable, assume ");
    printExp(stdout,v);
    Printf(" :: _%d\n",beta);
}
    return beta;
}

static Void local newDefnBind(v,type)  /* make new assump for defn var	   */
Cell v; 			       /* and set type if given (nonNull)  */
Type type; {
    Int  beta	   = newTyvars(1);
    Cell ta        = mkInt(beta);
    instantiate(type);
    if (nonNull(type) && isPolyType(type))
	ta = pair(POLYREC,pair(ta,type));
    hd(defnBounds) = cons(pair(v,ta), hd(defnBounds));
if (DEBUG_TYPES) {
    Printf("definition, assume ");
    printExp(stdout,v);
    Printf(" :: _%d\n",beta);
}
    bindTv(beta,typeIs,typeOff);       /* Bind beta to new type skeleton   */
}

/* --------------------------------------------------------------------------
 * Predicates:
 * ------------------------------------------------------------------------*/

/* --------------------------------------------------------------------------
 * Part of the type checker dealing with predicates and entailment
 *
 * The Hugs 98 system is Copyright (c) Mark P Jones, Alastair Reid, the
 * Yale Haskell Group, and the OGI School of Science & Engineering at OHSU,
 * 1994-2003, All rights reserved.  It is distributed as free software under
 * the license in the file "License", which is included in the distribution.
 *
 * $RCSfile: preds.c,v $
 * $Revision: 1.36 $
 * $Date: 2003/11/01 17:02:47 $
 * ------------------------------------------------------------------------*/

/* --------------------------------------------------------------------------
 * Local function prototypes:
 * ------------------------------------------------------------------------*/

static Cell   local assumeEvid        (Cell,Int);
static Cell   local findIPEvid	      (Text);
static Void   local removeIPEvid      (Text);
static List   local makePredAss	      (List,Int);
static List   local copyPreds	      (List);
static Void   local qualify	      (List,Cell);
static Void   local qualifyBinding    (List,Cell);
static Cell   local qualifyExpr	      (Int,List,Cell);
static Void   local overEvid	      (Cell,Cell);

static Void   local cutoffExceeded    (Cell,Int,List);
static Cell   local scFind	      (Cell,Cell,Int,Cell,Int,Int);
static Cell   local scEntail	      (List,Cell,Int,Int);
static Cell   local entail	      (List,Cell,Int,Int);
static Cell   local inEntail	      (List,Cell,Int,Int);
static Cell   local lacksNorm	      (Type,Int,Cell);

static List   local scSimplify	      (List);
static Void   local elimTauts	      (Void);
static Bool   local anyGenerics       (Type,Int);
static List   local elimOuterPreds    (List);
static List   local elimPredsUsing    (List,List);
static Void   local reducePreds	      (Void);
static Void   local normPreds	      (Int);

static Bool   local resolveDefs	      (List,Bool);
static Bool   local resolveVar	      (Int,Bool);
static Class  local classConstraining (Int,Cell,Int);

/* --------------------------------------------------------------------------
 * Predicate assignments:
 *
 * A predicate assignment is represented by a list of triples (pi,o,ev)
 * where o is the offset for types in pi, with evidence required at the
 * node pointed to by ev (which is taken as a dictionary parameter if
 * no other evidence is available).  Note that the ev node will be
 * overwritten at a later stage if evidence for that predicate is found
 * subsequently.
 * ------------------------------------------------------------------------*/

static List preds;			/* Current predicate assignment	   */

static Cell local assumeEvid(pi,o)	/* Add predicate pi (offset o) to  */
Cell pi;				/* preds with new dict var nd	   */
Int  o; {
    Cell nd = inventDictVar();
    preds   = cons(triple(pi,mkInt(o),nd),preds);
    return nd;
}

static Cell local findIPEvid(t)
Text t; {
    List ps = preds;
    for (; nonNull(ps); ps=tl(ps)) {
	Cell p = hd(ps);	
	if (ipMatch(fst3(p), t))
	    return p;
    }
    return NIL;
}

static Void local removeIPEvid(t)
Text t; {
    List ps = preds;
    List *prev = &preds;
    for (; nonNull(ps); ps = tl(ps))
	if (ipMatch(fst3(hd(ps)), t)) {
	    *prev = tl(ps);
	    return;
	} else {
	    prev = &tl(ps);
	}
}

static List local makePredAss(qs,o)	/* Make list of predicate assumps. */
List qs;				/* from qs (offset o), w/ new dict */
Int  o; {				/* vars for each predicate	   */
    List result = NIL;
    for (; nonNull(qs); qs=tl(qs))
	result = cons(triple(hd(qs),mkInt(o),inventDictVar()),result);
    return rev(result);
}

static List local copyPreds(qs)		/* Copy list of predicates         */
List qs; {
    List result = NIL;
    for (; nonNull(qs); qs=tl(qs)) {
	Cell pi = hd(qs);
	result  = cons(copyPred(fst3(pi),intOf(snd3(pi))),result);
    }
    return rev(result);
}

static Void local qualify(qs,alt)	/* Add extra dictionary args to	   */
List qs;				/* qualify alt by predicates in qs */
Cell alt; {				/* :: ([Pat],Rhs)		   */
    List ds;
    for (ds=NIL; nonNull(qs); qs=tl(qs))
	ds = cons(thd3(hd(qs)),ds);
    fst(alt) = revOnto(ds,fst(alt));
}

static Void local qualifyBinding(qs,b)	/* Add extra dict args to each	   */
List qs;				/* alternative in function binding */
Cell b ; {
    if (!isVar(fst(b)))			/* check for function binding	   */
	internal("qualifyBinding");
    map1Proc(qualify,qs,snd(snd(b)));
}

static Cell local qualifyExpr(l,ps,e)	/* Add dictionary params to expr   */
Int  l;
List ps;
Cell e; {
    if (nonNull(ps)) {			/* Qualify input expression with   */
	if (whatIs(e)!=LAMBDA)		/* additional dictionary params	   */
	    e = ap(LAMBDA,pair(NIL,pair(mkInt(l),e)));
	qualify(ps,snd(e));
    }
    return e;
}

static Void local overEvid(dv,ev)	/* Overwrite dict var dv with	   */
Cell dv;				/* evidence ev			   */
Cell ev; {
    fst(dv) = nameInd;
    snd(dv) = ev;
}

/* --------------------------------------------------------------------------
 * Predicate entailment:
 *
 * Entailment plays a prominent role in the theory of qualified types, and
 * so, unsurprisingly, in the implementation too.  For practical reasons,
 * we break down entailment into two pieces.  The first, scEntail, uses
 * only the information provided by class declarations, while the second,
 * entail, also uses the information in instance declarations.
 *
 * scEntail uses the following auxiliary function to do its work:
 *
 *   scFind (e : pi') pi : Find evidence for predicate pi using only
 *			     equality of predicates, superclass entailment,
 *			     and the evidence e for pi'.
 *
 *   scFind (e : pi') pi =
 *
 *      if pi = pi' then
 *          return e
 *
 *      if (pi.class.level < pi'.class.level)
 *	    get superclass entailment pi' ||- P
 *          for each (sc, pi0) in P
 *		if (ev := scFind (sc e : pi0) pi) /= NIL
 *                  return ev
 *
 *      return NIL
 *
 * This code assumes that the class hierarchy is acyclic, and that
 * each class has been assigned a `level', which is its height in
 * the hierachy.  The first of the assumptions guarantees that the
 * algorithm will terminate.  The comparison of levels is an
 * optimization that cuts down the search space: given that superclass
 * entailments can only be used to descend the hierarchy, there is no
 * way we can reach a higher level than the one that we start with,
 * and hence there is no point in looking if we reach such a position.
 *
 * scEntail extends scFind to work on whole predicate assignments:
 *
 *   scEntail P pi : Find evidence for predicate pi using the evidence
 *		     provided by the predicate assignment P, and using
 *		     only superclass entailments.
 *
 *   scEntail P pi =
 *
 *       for each (v:pi') in P
 *	     if (ev := scFind (v:pi') pi) /= NIL
 * 		 return ev;
 *	 return NIL
 *
 * ------------------------------------------------------------------------*/

Int cutoff = 40;			/* Used to limit depth of recursion*/

static Void local cutoffExceeded(pi,o,ps)
Cell pi;				/* Display error msg when cutoff   */
Int  o;
List ps; {
    clearMarks();
    ERRMSG(0)
	"\n*** The type checker has reached the cutoff limit while trying to\n"
    ETHEN ERRTEXT
	"*** determine whether:\n***     "     ETHEN ERRPRED(copyPred(pi,o));
    ps = copyPreds(ps);
    ERRTEXT
	"\n*** can be deduced from:\n***     " ETHEN ERRCONTEXT(ps);
    ERRTEXT
	"\n*** This may indicate that the problem is undecidable.  However,\n"
    ETHEN ERRTEXT
	"*** you may still try to increase the cutoff limit using the -c\n"
    ETHEN ERRTEXT
	"*** option and then try again.  (The current setting is -c%d)\n",
	cutoff
    EEND;
}

static Cell local scFind(e,pi1,o1,pi,o,d)/* Use superclass entailment to   */
Cell e;					/* find evidence for (pi,o) using  */
Cell pi1;				/* the evidence e for (pi1,o1).	   */
Int  o1;
Cell pi;
Int  o;
Int  d; {
    Class h1 = getHead(pi1);
    Class h  = getHead(pi);
    Cell ev = NIL;

    if (samePred(pi1,o1,pi,o))
	return e;

    if (isClass(h1) && (!isClass(h) || cclass(h).level<cclass(h1).level)) {
	Int  beta  = newKindedVars(cclass(h1).kinds);
	List scs   = cclass(h1).supers;
	List dsels = cclass(h1).dsels;
	List ps = NIL;
	if (!matchPred(pi1,o1,cclass(h1).head,beta))
	    internal("scFind");

	for (; nonNull(scs); scs=tl(scs), dsels=tl(dsels))
	    ps = cons(triple(hd(scs),mkInt(beta),ap(hd(dsels),e)),ps);
	ps = rev(ps);

	if (showInstRes) {
	    int i;
	    for (i = 0; i < d; i++)
	      fputc(' ', stdout);
	    fputs("scEntail(scFind): ", stdout);
	    printContext(stdout,copyPreds(ps));
	    fputs(" ||- ", stdout);
	    printPred(stdout, copyPred(pi, o));
	    fputc('\n', stdout);
	}
	improve1(0,ps,pi,o);
	ev = scEntail(ps,pi,o,d);
	if (showInstRes && nonNull(ev)) {
	    int i;
	    for (i = 0; i < d; i++)
	      fputc(' ', stdout);
	    fputs("scSat.\n", stdout);
	}
	return ev;
    }
    return NIL;
}

static Cell local scEntail(ps,pi,o,d)	/* Calc evidence for (pi,o) from ps*/
List ps;				/* Using superclasses and equality.*/
Cell pi;
Int  o;
Int  d; {
    if (d++ >= cutoff)
	cutoffExceeded(pi,o,ps);

    for (; nonNull(ps); ps=tl(ps)) {
	Cell pi1 = hd(ps);
	Cell ev  = scFind(thd3(pi1),fst3(pi1),intOf(snd3(pi1)),pi,o,d);
	if (nonNull(ev))
	    return ev;
    }
    return NIL;
}

/* --------------------------------------------------------------------------
 * Now we reach the main entailment routine:
 *
 *   entail P pi : Find evidence for predicate pi using the evidence
 *		   provided by the predicate assignment P.
 *
 *   entail P pi =
 *
 *	 if (ev := scEntail P pi) /= NIL
 *	     return ev;
 *
 *       if there is an instance entailment i : Q ||- pi
 *	     for each pi' in Q
 *		 if (ev := entail P pi') /= NIL
 *		     i := ap(i,ev)
 *		 else
 *		     return NIL
 *	     return i
 *
 *	 return NIL;
 *
 * The form of evidence expressions produced by scEntail can be described
 * by the grammar:
 *
 *    e  =  v  |  sc e            (v = evidence var, sc = superclass sel)
 *
 * while entail extends this to include dictionary expressions given by:
 *
 *    d  =  e  |  mki d1 ... dn   (mki = dictionary constructor)
 *
 * A full grammar for evidence expressions is:
 *
 *    d   =   v   |   sc d   |   mki d1 ... dn
 *
 * and this includes evidence expressions of the form  sc (mki d1 ... dn)
 * that can never be produced by either of the entail functions described
 * above.  This is good, from a practical perspective, because t means
 * that we won't waste effort building a dictionary (mki d1 ... dn) only
 * to extract just one superclass component and throw the rest away.
 * Moreover, conditions on instance decls already guarantee that any
 * expression of this form can be rewritten in the form  mki' d1' ... dn'.
 * (Minor point: they don't guarantee that such rewritings will lead to
 * smaller terms, and hence to termination.  However, we have already
 * accepted the benefits of an undecidable entailment relation over
 * guarantees of termination, and this additional quirk is unlikely
 * to cause any further concern, except in pathological cases.)
 * ------------------------------------------------------------------------*/

static Cell local entail(ps,pi,o,d)	/* Calc evidence for (pi,o) from ps*/
List ps;				/* Uses superclasses, equality,    */
Cell pi;				/* tautology, and construction	   */
Int  o;
Int  d; {
    Cell ev = NIL;

    if (showInstRes) {
	int i;
	for (i = 0; i < d; i++)
	  fputc(' ', stdout);
	fputs("entail: ", stdout);
	printContext(stdout,copyPreds(ps));
	fputs(" ||- ", stdout);
	printPred(stdout, copyPred(pi, o));
	fputc('\n', stdout);
    }

    ev = scEntail(ps,pi,o,d);
    if (nonNull(ev)) {
	if (showInstRes) {
	    int i;
	    for (i = 0; i < d; i++)
	      fputc(' ', stdout);
	    fputs("scSat.\n", stdout);
	}
    } else {
	ev =
             inEntail(ps,pi,o,d);
	if (nonNull(ev) && showInstRes) {
	    int i;
	    for (i = 0; i < d; i++)
	      fputc(' ', stdout);
	    fputs("inSat.\n", stdout);
	}
    }
    return ev;
}

static Cell local inEntail(ps,pi,o,d)	/* Calc evidence for (pi,o) from ps*/
List ps;				/* using a top-level instance	   */
Cell pi;				/* entailment			   */
Int  o;
Int  d; {
    int i;
    Inst in;

    if (d++ >= cutoff)
	cutoffExceeded(pi,o,ps);

    if (isAp(pi) && isExt(fun(pi))) {	/* Lacks predicates		   */
	Cell e  = fun(pi);
	Cell l;
	l  = lacksNorm(arg(pi),o,e);
	if (isNull(l) || isInt(l))
	    return l;
	else {
	    List qs = ps;
	    for (; nonNull(qs); qs=tl(qs)) {
		Cell qi = fst3(hd(qs));
		if (isAp(qi) && fun(qi)==e) {
		    Cell lq = lacksNorm(arg(qi),intOf(snd3(hd(qs))),e);
		    if (isAp(lq) && intOf(fst(l))==intOf(fst(lq))) {
			Int f = intOf(snd(l)) - intOf(snd(lq));
			return (f==0) ? thd3(hd(qs)) : ap2(nameAddEv,
							   mkInt(f),
							   thd3(hd(qs)));
		    }
		}
	    }
	    return NIL;
	}
    }
    else {

    in = findInstFor(pi,o);	/* Class predicates		   */
    if (nonNull(in)) {
	Int  beta = typeOff;
	Cell e    = inst(in).builder;
	List es   = inst(in).specifics;
	List fs   = NIL;
	for (; nonNull(es); es=tl(es))
	    fs = cons(triple(hd(es),mkInt(beta),NIL),fs);
	fs = rev(fs);
	improve(0,ps,fs);
	if (showInstRes) {
	    for (i = 0; i < d; i++)
	      fputc(' ', stdout);
	    fputs("try ", stdout);
	    printContext(stdout, copyPreds(fs));
	    fputs(" => ", stdout);
	    printPred(stdout, copyPred(inst(in).head,beta));
	    fputc('\n', stdout);
	}
	for (es=inst(in).specifics; nonNull(es); es=tl(es)) {
	    Cell ev;
	    improve1(0,ps,hd(es),beta);
	    ev = entail(ps,hd(es),beta,d);
	    if (nonNull(ev))
		e = ap(e,ev);
	    else
		return NIL;
	}
	return e;
    }
      else {
	if (showInstRes) {
	    for (i = 0; i < d; i++)
	      fputc(' ', stdout);
	    fputs("No instance found for ", stdout);
	    printPred(stdout, copyPred(pi, o));
	    fputc('\n', stdout);
	}
    }
    return NIL;
    }
}


Cell provePred(ks,ps,pi)                /* Find evidence for predicate pi  */
Kinds ks;                               /* assuming ps.  If ps is null,    */
List  ps;                               /* then we get to decide whether   */
Cell  pi; {                             /* is tautological, and we can use */
    Int  beta;                          /* the evidence as a dictionary.   */
    Cell ev;
    emptySubstitution();
    beta = newKindedVars(ks);           /* (ks provides kinds for any      */
    ps   = makePredAss(ps,beta);        /*  vars that appear in pi.)       */
    ev   = entail(ps,pi,beta,0);
    emptySubstitution();
    return ev;
}

Cell resolvePred(ks,pi)			/* Find evidence for predicate pi. */
Kinds ks;				/* Unlike `provePred', also uses   */
Cell  pi; {				/* desparate measures like context */
    Int  beta;				/* reduction and defaulting.       */
    List qs;

    emptySubstitution();
    beta = newKindedVars(ks);
    qs = makePredAss(cons(pi,NIL),beta);
    preds = qs;
    elimTauts();
    if (resolveDefs(NIL,TRUE))
	elimTauts();
    emptySubstitution();
    return (nonNull(preds) ? NIL : thd3(hd(qs)));
}

static Cell local lacksNorm(t,o,e)	/* Normalize lacks pred (t,o)\l	   */
Type t;					/* returning NIL (unsatisfiable),  */
Int  o;					/* Int (tautological) or pair (v,a)*/
Cell e; {				/* such that, if e is evid for v\l,*/
    Text l = extText(e);		/* then (e+a) is evid for (t,o)\l. */
    Int  a = 0;
    for (;;) {
	Tyvar *tyv;
	deRef(tyv,t,o);
	if (tyv)
	    return pair(mkInt(tyvNum(tyv)),mkInt(a));
	else {
	    Cell h = getDerefHead(t,o);
	    if (h==typeNoRow && argCount==0)
		return mkInt(a);
	    else if (isExt(h) && argCount==2) {
		Text l1 = extText(h);
		if (l1==l)
		    return NIL;
		else if (strcmp(textToStr(l1),textToStr(l))<0)
		    a++;
		t = arg(t);
	    }
	    else
		return NIL;
	}
    }
}

/* --------------------------------------------------------------------------
 * Predicate set Simplification:
 *
 * Calculate a minimal equivalent subset of a given set of predicates.
 * ------------------------------------------------------------------------*/

static List local scSimplify(qs)	/* Simplify predicates in qs,      */
List qs; {				/* returning equiv minimal subset  */
    Int n = length(qs);

    while (0<n--) {
	Cell pi = hd(qs);
	Cell ev = NIL;
	if (showInstRes) {
	    fputs("scSimplify: ", stdout);
	    printContext(stdout,copyPreds(tl(qs)));
	    fputs(" ||- ", stdout);
	    printPred(stdout, copyPred(fst3(pi),intOf(snd3(pi))));
	    fputc('\n', stdout);
	}
	ev = scEntail(tl(qs),fst3(pi),intOf(snd3(pi)),0);
	if (nonNull(ev)) {
	    if (showInstRes)
	        fputs("Simplified!\n", stdout);
	    overEvid(thd3(pi),ev);	/* Overwrite dict var with evidence*/
	    qs      = tl(qs);		/* ... and discard predicate	   */
	}
	else {				/* Otherwise, retain predicate	   */
	    Cell tmp = tl(qs);
	    tl(qs)   = NIL;
	    qs       = appendOnto(tmp,qs);
	}
    }
    return qs;
}

List simpleContext(ps,o)		/* Simplify context of skeletons   */
List ps;				/* skeletons, offset o, using	   */
Int  o; {				/* superclass hierarchy		   */
    return copyPreds(scSimplify(makePredAss(ps,o)));
}

/* --------------------------------------------------------------------------
 * Context splitting --- tautological and locally tautological predicates:
 * ------------------------------------------------------------------------*/

static Void local elimTauts() {		/* Remove tautological constraints */
    if (haskell98) {			/* from preds			   */
	reducePreds();			/* (or context reduce for Hask98)  */
    } else {
	List ps = preds;
	preds   = NIL;
	while (nonNull(ps)) {
	    Cell pi = hd(ps);
	    Cell ev = entail(NIL,fst3(pi),intOf(snd3(pi)),0);
	    if (nonNull(ev)) {
		overEvid(thd3(pi),ev);
		ps = tl(ps);
	    }
	    else {
		List tmp = tl(ps);
		tl(ps)   = preds;
		preds    = ps;
		ps	     = tmp;
	    }
	}
    }
}

static Int numFixedVars = 0;		/* Number of fixed vars found	   */

static Bool local anyGenerics(t,o)	/* Test for generic vars, and count*/
Type t;					/* fixed variables		   */
Int  o; {
    Type h = getDerefHead(t,o);		/* This code is careful to expand  */
    Int  a = argCount;			/* synonyms; mark* & copy* do not. */
    if (isSynonym(h) && a>=tycon(h).arity) {
	expandSyn(h,a,&t,&o);
	return anyGenerics(t,o);
    }
    else {
	Tyvar* tyv;
	for (; 0<a--; t=fun(t)) {	/* cycle through any arguments	   */
	    deRef(tyv,t,o);
	    if (anyGenerics(arg(t),o))
		return TRUE;
	}
	deRef(tyv,t,o);
	if (tyv)
	    if (tyv->offs == FIXED_TYVAR) {
		numFixedVars++;
		return FALSE;
	    }
	    else
		return TRUE;
	else
	    return FALSE;
    }
}

static List local elimOuterPreds(sps)	/* Simplify and defer any remaining*/
List sps; {				/* preds that contain no generics. */
    List qs = NIL;
    elimTauts();
    for (preds=scSimplify(preds); nonNull(preds); ) {
	Cell pi = hd(preds);
	Cell nx = tl(preds);
	if (anyGenerics(fst3(pi),intOf(snd3(pi)))
	    || !isAp(fst3(pi))
	    || isIP(fun(fst3(pi)))) {
	    tl(preds) = qs;				/* Retain predicate*/
	    qs	      = preds;
	}
	else {						/* Defer predicate */
	    tl(preds) = sps;
	    sps       = preds;
	}
	preds = nx;
    }
    preds = qs;
    return sps;
}

static List local elimPredsUsing(ps,sps)/* Try to discharge or defer preds,*/
List ps;				/* splitting if necessary to match */
List sps; {				/* context ps.  sps = savePreds.   */
    List rems = NIL;
    while (nonNull(preds)) {		/* Pick a predicate from preds	   */
	Cell p  = preds;
	Cell pi = fst3(hd(p));
	Int  o  = intOf(snd3(hd(p)));
	Cell ev = entail(ps,pi,o,0);
	preds   = tl(preds);

	if (nonNull(ev)) {		/* Discharge if ps ||- (pi,o)	   */
	    overEvid(thd3(hd(p)),ev);
	} else if (!isAp(pi) || isIP(fun(pi)) || !anyGenerics(pi,o)) {
	    tl(p) = sps;		/* Defer if no generics		   */
	    sps   = p;
	}
	else {				/* Try to split generics and fixed */
	    Inst in;
	    if (numFixedVars>0 && nonNull(in=findInstFor(pi,o))) {
		List qs = inst(in).specifics;
		for (ev=inst(in).builder; nonNull(qs); qs=tl(qs))
		    ev = ap(ev,assumeEvid(hd(qs),typeOff));
		overEvid(thd3(hd(p)),ev);
	    }
	    else {			/* No worthwhile progress possible */
		tl(p) = rems;
		rems  = p;
	    }
	}
    }
    preds = rems;			/* Return any remaining predicates */
    return sps;
}

static Void local reducePreds() {	/* Context reduce predicates: uggh!*/
    List rems = NIL;         		/* (A last resort for defaulting)  */
    while (nonNull(preds)) {		/* Pick a predicate from preds	   */
	Cell p  = preds;
	Cell pi = fst3(hd(p));
	Int  o  = intOf(snd3(hd(p)));
	Inst in = NIL;
	    in = findInstFor(pi,o);
	preds   = tl(preds);
	if (nonNull(in)) {
	    List qs = inst(in).specifics;
	    Cell ev = inst(in).builder;
	    for (; nonNull(qs); qs=tl(qs))
		ev = ap(ev,assumeEvid(hd(qs),typeOff));
	    overEvid(thd3(hd(p)),ev);
	}
	else {				/* No worthwhile progress possible */
	    tl(p) = rems;
	    rems  = p;
	}
    }
    preds = scSimplify(rems);		/* Return any remaining predicates */
}

static Void local normPreds(line)	/* Normalize each element of preds */
Int line; {				/* in some appropriate manner	   */
    List ps = preds;
    List pr = NIL;
    while (nonNull(ps)) {
	Cell pi = fst3(hd(ps));
	Cell ev = thd3(hd(ps));
	if (isAp(pi) && isExt(fun(pi))) {
	    Cell r = lacksNorm(arg(pi),intOf(snd3(hd(ps))),fun(pi));
	    if (isNull(r)) {
		ERRMSG(line) "Cannot satisfy constraint " ETHEN
		ERRPRED(copyPred(pi,intOf(snd3(hd(ps)))));
		ERRTEXT      "\n"
		EEND;
	    }
	    else if (isInt(r)) {
		overEvid(ev,r);
		ps = tl(ps);
		if (isNull(pr))
		    preds  = ps;
		else
		    tl(pr) = ps;
	    }
	    else if (intOf(snd(r))!=0) {
		Cell nd  = inventDictVar();
		Cell ev1 = ap2(nameAddEv,snd(r),nd);
		pi       = ap(fun(pi),aVar);
		hd(ps)   = triple(pi,fst(r),nd);
		overEvid(ev,ev1);
		pr       = ps;
		ps       = tl(ps);
	    }
	    else {
		fst3(hd(ps)) = ap(fun(pi),fst(r));
		pr = ps;
		ps = tl(ps);
	    }
	}
	else {
	    pr = ps;
	    ps = tl(ps);
	}
    }
}

/* --------------------------------------------------------------------------
 * Mechanisms for dealing with defaults:
 * ------------------------------------------------------------------------*/

static Bool local resolveDefs(vs,interactive)/* Attempt to resolve defaults */
List vs;				/* for variables vs subject to  */
Bool interactive; {			/* constraints in preds		*/
    List pvs       = NIL;
    List qs        = preds;
    Bool defaulted = FALSE;

if (DEBUG_DEFAULTS) {
    Printf("Attempt to resolve variables ");
    printExp(stdout,vs);
    Printf(" with context ");
    printContext(stdout,copyPreds(preds));
    Printf("\n");
}

    resetGenerics();			/* find type variables in ps	*/
    for (; nonNull(qs); qs=tl(qs)) {
	Cell pi = fst3(hd(qs));
	Int  o  = intOf(snd3(hd(qs)));
	for (; isAp(pi); pi=fun(pi))
	    pvs = genvarType(arg(pi),o,pvs);
    }

    for (; nonNull(pvs); pvs=tl(pvs)) {	/* now try defaults		*/
	Int vn = intOf(hd(pvs));

if (DEBUG_DEFAULTS) {
	Printf("is var %d included in ",vn);
	printExp(stdout,vs);
	Printf("?\n");
}

	if (!intIsMember(vn,vs))
	    defaulted |= resolveVar(vn,interactive);
	else if (DEBUG_DEFAULTS)
	    Printf("Yes, so no ambiguity!\n");
    }

    return defaulted;
}

static Bool local resolveVar(vn,interactive)/* Determine whether an ambig. */
Int  vn;				/* variable vn can be resolved  */
Bool interactive; {			/* by default in the context of */
    List ps        = preds;
    List cs	   = NIL;		/* the predicates in ps		*/
    Bool aNumClass = FALSE;

    if (tyvar(vn)->bound == SKOLEM)
	return FALSE;

    /* According to the Haskell definition, we can only default an ambiguous
     * variable if the set of classes that constrain it:
     *   (a) includes at least one numeric class.
     *       (However if interactive is TRUE, we also allow Show, Eq or Ord)
     *   (b) includes only numeric or standard classes.
     * In addition, we will not allow a variable to be defaulted unless it
     * appears only in predicates of the form (Class var).
     */

if (DEBUG_DEFAULTS) {
    Printf("Trying to default variable %d\n",vn);
}

    for (; nonNull(ps); ps=tl(ps)) {
	Cell  pi = hd(ps);
	Class c  = classConstraining(vn,fst3(pi),intOf(snd3(pi)));
	if (nonNull(c)) {
	    if (c==classRealFrac   || c==classRealFloat ||
		c==classFractional || c==classFloating  ||
		c==classReal	   || c==classIntegral  || c==classNum ||
		(interactive && (c==classEq || c==classOrd || c==classShow)))
		aNumClass = TRUE;
	    else if (c!=classEq    && c!=classOrd  && c!=classShow &&
		     c!=classRead  && c!=classIx   && c!=classEnum &&
		     c!=classBounded)
		return FALSE;

	    {	Type  t = arg(fst3(pi));/* Check for single var as arg	   */
		Int   o = intOf(snd3(pi));
		Tyvar *tyv;
		deRef(tyv,t,o);
		if (!tyv || tyvNum(tyv)!=vn)
		    return FALSE;
	    }
	    if (!cellIsMember(c,cs))
		cs = cons(c,cs);
	}
    }

    /* Now find the first class (if any) in the list of defaults that
     * is an instance of all of the required classes.
     *
     * If we get this far, then cs only mentions classes from the list
     * above, all of which have only a single parameter of kind *.
     */

    if (aNumClass) {
	List ds = defaultDefns;		/* N.B. guaranteed to be monotypes */
if (DEBUG_DEFAULTS) {
	Printf("Default conditions met, looking for type\n");
}
	for (; nonNull(ds); ds=tl(ds)) {
	    List cs1 = cs;
	    while (nonNull(cs1) && nonNull(entail(NIL,ap(hd(cs1),hd(ds)),0,0)))
		cs1 = tl(cs1);
	    if (isNull(cs1)) {
		bindTv(vn,hd(ds),0);
if (DEBUG_DEFAULTS) {
		Printf("Default type for variable %d is ",vn);
		printType(stdout,hd(ds));
		Printf("\n");
}
		return TRUE;
	    }
	}
    }

if (DEBUG_DEFAULTS) {
    Printf("No default permitted/found\n");
}
    return FALSE;
}

static Class local classConstraining(vn,pi,o)
Int  vn;				/* Return class constraining var*/
Cell pi;				/* vn in predicate pi, or NIL if*/
Int  o; { 				/* vn is not involved		*/
    for (; isAp(pi); pi=fun(pi))
	if (!doesntOccurIn(tyvar(vn),arg(pi),o))
	    return getHead(pi);
    return NIL;
}

/*-------------------------------------------------------------------------*/

/* --------------------------------------------------------------------------
 * Bound and skolemized type variables:
 * ------------------------------------------------------------------------*/

static List pendingBtyvs = NIL;

static Void local enterPendingBtyvs() {
    enterBtyvs();
    pendingBtyvs = cons(NIL,pendingBtyvs);
}

static Void local leavePendingBtyvsQuietly() {
    pendingBtyvs = tl(pendingBtyvs);
    leaveBtyvs();
}

static Void local leavePendingBtyvs() {
    List pts     = hd(pendingBtyvs);
    pendingBtyvs = tl(pendingBtyvs);
    for (; nonNull(pts); pts=tl(pts)) {
	Int  line = intOf(fst(hd(pts)));
	List vs   = snd(hd(pts));
	Int  i    = 0;
	clearMarks();
	for (; nonNull(vs); vs=tl(vs)) {
	    Cell v = fst(hd(vs));
	    Cell t = copyTyvar(intOf(snd(hd(vs))));
	    if (!isOffset(t)) {
		ERRMSG(line) "Type annotation uses variable " ETHEN ERREXPR(v);
		ERRTEXT      " where a more specific type "   ETHEN ERRTYPE(t);
		ERRTEXT      " was inferred"
		EEND;
	    }
	    else if (offsetOf(t)!=i) {
		List us = snd(hd(pts));
		Int  j  = offsetOf(t);
		if (j>=i)
		    internal("leavePendingBtyvs");
		for (; j>0; j--)
		    us = tl(us);
		ERRMSG(line) "Type annotation uses distinct variables " ETHEN
		ERREXPR(v);  ERRTEXT " and " ETHEN ERREXPR(fst(hd(us)));
		ERRTEXT      " where a single variable was inferred"
		EEND;
	    }
	    else
		i++;
	}
    }
    leaveBtyvs();
}

static Cell local patBtyvs(p)		/* Strip bound type vars from pat  */
Cell p; {
    if (whatIs(p)==BIGLAM) {
	List bts = hd(btyvars) = fst(snd(p));
	for (p=snd(snd(p)); nonNull(bts); bts=tl(bts)) {
	    Int beta          = newTyvars(1);
	    tyvar(beta)->kind = snd(hd(bts));
	    snd(hd(bts))      = mkInt(beta);
	}
    }
    return p;
}

static Void local doneBtyvs(l)
Int l; {
    if (nonNull(hd(btyvars))) {		/* Save bound tyvars		   */
	hd(pendingBtyvs) = cons(pair(mkInt(l),hd(btyvars)),hd(pendingBtyvs));
	hd(btyvars)	 = NIL;
    }
}

static Void local enterSkolVars() {
    skolVars = cons(NIL,skolVars);
    localEvs = cons(NIL,localEvs);
    savedPs  = cons(preds,savedPs);
    preds    = NIL;
}

static Void local leaveSkolVars(l,t,o,m)
Int  l;
Type t;
Int  o;
Int  m; {
    if (nonNull(hd(localEvs))) {	/* Check for local predicates	   */
	List sks = hd(skolVars);
	List sps = NIL;
	if (isNull(sks)) {
	    internal("leaveSkolVars");
	}
	markAllVars();			/* Mark all variables in current   */
	do {				/* substitution, then unmark sks.  */
	    tyvar(intOf(fst(hd(sks))))->offs = UNUSED_GENERIC;
	    sks = tl(sks);
	} while (nonNull(sks));
	normPreds(l);
	sps   = elimPredsUsing(hd(localEvs),sps);
	preds = revOnto(preds,sps);
    }

    if (nonNull(hd(skolVars))) {	/* Check that Skolem vars do not   */
	List vs;			/* escape their scope		   */
	Int  i = 0;

	clearMarks();			/* Look for occurences in the	   */
	for (; i<m; i++)		/* inferred type		   */
	    markTyvar(o+i);
	markType(t,o);

	for (vs=hd(skolVars); nonNull(vs); vs=tl(vs)) {
	    Int vn = intOf(fst(hd(vs)));
	    if (tyvar(vn)->offs == FIXED_TYVAR) {
		Cell tv = copyTyvar(vn);
		Type ty = liftRank2(t,o,m);
		ERRMSG(l) "Existentially quantified variable in inferred type"
		ETHEN
		ERRTEXT   "\n*** Variable     : " ETHEN ERRTYPE(tv);
		ERRTEXT   "\n*** From pattern : " ETHEN ERREXPR(snd(hd(vs)));
		ERRTEXT   "\n*** Result type  : " ETHEN ERRTYPE(ty);
		ERRTEXT   "\n"
		EEND;
	    }
	}

	markBtyvs();			/* Now check assumptions	   */
	mapProc(markAssumList,defnBounds);
	mapProc(markAssumList,varsBounds);
	mapProc(markPred,preds);

	for (vs=hd(skolVars); nonNull(vs); vs=tl(vs)) {
	    Int vn = intOf(fst(hd(vs)));
	    if (tyvar(vn)->offs == FIXED_TYVAR) {
		ERRMSG(l)
		  "Existentially quantified variable escapes from pattern "
		ETHEN ERREXPR(snd(hd(vs)));
		ERRTEXT "\n"
		EEND;
	    }
	}
    }
    localEvs = tl(localEvs);
    skolVars = tl(skolVars);
    preds    = revOnto(preds,hd(savedPs));
    savedPs  = tl(savedPs);
}

/* --------------------------------------------------------------------------
 * Type errors:
 * ------------------------------------------------------------------------*/

static Void local typeError(l,e,in,wh,t,o)
Int    l;			      /* line number near type error	   */
Cell   e;			      /* source of error		   */
Cell   in;			      /* context if any (NIL if not)	   */
String wh;			      /* place in which error occurs	   */
Type   t;			      /* should be of type (t,o)	   */
Int    o; {			      /* type inferred is (typeIs,typeOff) */

    clearMarks();		      /* types printed here are monotypes  */
				      /* use marking to give sensible names*/
if (DEBUG_KINDS)
{ List vs = genericVars;
  for (; nonNull(vs); vs=tl(vs)) {
     Int v = intOf(hd(vs));
     Printf("%c :: ", ('a'+tyvar(v)->offs));
     printKind(stdout,tyvar(v)->kind);
     Putchar('\n');
  }
}

    reportTypeError(l,e,in,wh,copyType(typeIs,typeOff),copyType(t,o));
}

static Void local reportTypeError(l,e,in,wh,inft,expt)
Int    l;				/* Error printing part of typeError*/
Cell   e, in;
String wh;
Type   inft, expt; {
    ERRMSG(l)   "Type error in %s", wh    ETHEN
    if (nonNull(in)) {
	ERRTEXT "\n*** Expression     : " ETHEN ERREXPR(in);
    }
    ERRTEXT     "\n*** Term           : " ETHEN ERREXPR(e);
    ERRTEXT     "\n*** Type           : " ETHEN ERRTYPE(inft);
    ERRTEXT     "\n*** Does not match : " ETHEN ERRTYPE(expt);
    if (unifyFails) {
	ERRTEXT "\n*** Because        : %s", unifyFails ETHEN
    }
    ERRTEXT "\n"
    EEND;
}

#define shouldBe(l,e,in,where,t,o) if (!unify(typeIs,typeOff,t,o)) \
				       typeError(l,e,in,where,t,o);
#define check(l,e,in,where,t,o)    e=typeExpr(l,e); shouldBe(l,e,in,where,t,o)
#define inferType(t,o)		   typeIs=t; typeOff=o
#define spTypeExpr(l,e)			svPreds = preds; preds = NIL; e = typeExpr(l,e); preds = revOnto(preds,svPreds);
#define spCheck(l,e,in,where,t,o)	svPreds = preds; preds = NIL; check(l,e,in,where,t,o); preds = revOnto(preds,svPreds);

static Void local cantEstablish(line,wh,e,t,ps)
Int    line;				/* Complain when declared preds	   */
String wh;				/* are not sufficient to discharge */
Cell   e;				/* or defer the inferred context.  */
Type   t;
List   ps; {
    ERRMSG(line) "Cannot justify constraints in %s", wh ETHEN
    ERRTEXT	 "\n*** Expression    : " ETHEN ERREXPR(e);
    ERRTEXT	 "\n*** Type          : " ETHEN ERRTYPE(t);
    ERRTEXT	 "\n*** Given context : " ETHEN ERRCONTEXT(ps);
    ERRTEXT	 "\n*** Constraints   : " ETHEN ERRCONTEXT(copyPreds(preds));
    ERRTEXT "\n"
    EEND;
}

static Void local tooGeneral(l,e,dt,it)	/* explicit type sig. too general  */
Int  l;
Cell e;
Type dt, it; {
    ERRMSG(l) "Inferred type is not general enough" ETHEN
    ERRTEXT   "\n*** Expression    : " ETHEN ERREXPR(e);
    ERRTEXT   "\n*** Expected type : " ETHEN ERRTYPE(dt);
    ERRTEXT   "\n*** Inferred type : " ETHEN ERRTYPE(it);
    ERRTEXT   "\n"
    EEND;
}

/* --------------------------------------------------------------------------
 * Typing of expressions:
 * ------------------------------------------------------------------------*/

#define EXPRESSION  0			/* type checking expression	   */
#define NEW_PATTERN 1			/* pattern, introducing new vars   */
#define OLD_PATTERN 2			/* pattern, involving bound vars   */
static int tcMode = EXPRESSION;

static Cell local mytypeExpr	(Int,Cell);
static Cell local typeExpr(l,e)
Int l;
Cell e; {
    static int number = 0;
	int mynumber;
    Cell retv;
if(DEBUG_TYPES) {
    mynumber = number++;
    STACK_CHECK
    Printf("%d) to check: ",mynumber);
    printExp(stdout,e);
    Putchar('\n');
}
    retv = mytypeExpr(l,e);
if(DEBUG_TYPES) {
	Printf("%d) result: ",mynumber);
    printType(stdout,debugType(typeIs,typeOff));
    Printf("\n%d) preds: ",mynumber);
    printContext(stdout,debugContext(preds));
    Putchar('\n');
}
    return retv;
}
static Cell local mytypeExpr(l,e)	/* Determine type of expr/pattern  */
Int  l;
Cell e; {
    static String cond	  = "conditional";
    static String list	  = "list";
    static String discr   = "case discriminant";
    static String aspat   = "as (@) pattern";
    static String typeSig = "type annotation";
    static String lambda  = "lambda expression";
    List svPreds;

    switch (whatIs(e)) {

	/* The following cases can occur in either pattern or expr. mode   */

	case AP		:
	case NAME	:
	case VAROPCELL	:
	case VARIDCELL	:
	case IPVAR	:
	  		  return typeAp(l,e);

	case TUPLE	: typeTuple(e);
			  break;

	case POSNUM	:
	case ZERONUM	:
	case NEGNUM	: {   Int alpha = newTyvars(1);
			      inferType(aVar,alpha);
			      return ap(ap(nameFromInteger,
					   assumeEvid(predNum,alpha)),
					   e);
			  }
	case INTCELL	: {   Int alpha = newTyvars(1);
			      inferType(aVar,alpha);
			      return ap(ap(nameFromInt,
					   assumeEvid(predNum,alpha)),
					   e);
			  }

	case DOUBLECELL	: {   Int alpha = newTyvars(1);
			      inferType(aVar,alpha);
			      return ap(ap(nameFromDouble,
					   assumeEvid(predFractional,alpha)),
					   e);
			  }

	case STRCELL	: inferType(typeString,0);
			  break;

	case CHARCELL	: inferType(typeChar,0);
			  break;

	case CONFLDS	: typeConFlds(l,e);
			  break;

	case ESIGN	: snd(snd(e)) = localizeBtyvs(snd(snd(e)));
			  return typeExpected(l,typeSig,
					      fst(snd(e)),snd(snd(e)),
					      0,0,FALSE);

	case EXT	: {   Int beta = newTyvars(2);
			      Cell pi  = ap(e,aVar);
			      Type t   = fn(aVar,
					 fn(ap(typeRec,bVar),
					    ap(typeRec,ap(ap(e,aVar),bVar))));
			      tyvar(beta+1)->kind = ROW;
			      inferType(t,beta);
			      return ap(e,assumeEvid(pi,beta+1));
			  }

	/* The following cases can only occur in expr mode		   */

	case UPDFLDS	: typeUpdFlds(l,e);
			  break;

	case WITHEXP	: return typeWith(l,e);

	case COND	: {   Int beta = newTyvars(1);
			      check(l,fst3(snd(e)),e,cond,typeBool,0);
			      spCheck(l,snd3(snd(e)),e,cond,aVar,beta);
			      spCheck(l,thd3(snd(e)),e,cond,aVar,beta);
			      tyvarType(beta);
			  }
			  break;

	case LETREC	: enterBindings();
			  enterSkolVars();
			  mapProc(typeBindings,fst(snd(e)));
			  spTypeExpr(l,snd(snd(e)));
			  leaveBindings();
			  leaveSkolVars(l,typeIs,typeOff,0);
			  break;

	case FINLIST	: {   Int  beta = newTyvars(1);
			      List xs;
			      for (xs=snd(e); nonNull(xs); xs=tl(xs)) {
				 spCheck(l,hd(xs),e,list,aVar,beta);
			      }
			      inferType(listof,beta);
			  }
			  break;

	case DOCOMP	: typeDo(l,e);
			  break;

	case MDOCOMP	: typeMDo(l,e);
			  break;
	case COMP	: return typeMonadComp(l,e);

	case ZCOMP	: return typeMonadZComp(l,e);

	case CASE	: {    Int beta = newTyvars(2);    /* discr result */
			       check(l,fst(snd(e)),NIL,discr,aVar,beta);
			       map2Proc(typeCase,l,beta,snd(snd(e)));
			       tyvarType(beta+1);
			  }
			  break;

	case LAMBDA	: {   Int beta = newTyvars(1);
			      enterPendingBtyvs();
			      typeAlt(lambda,e,snd(e),aVar,beta,1);
			      leavePendingBtyvs();
			      tyvarType(beta);
			  }
			  break;

	case RECSEL	: {   Int beta = newTyvars(2);
			      Cell pi  = ap(snd(e),aVar);
			      Type t   = fn(ap(typeRec,
					       ap(ap(snd(e),aVar),
							    bVar)),aVar);
			      tyvar(beta+1)->kind = ROW;
			      inferType(t,beta);
			      return ap(e,assumeEvid(pi,beta+1));
			  }

	/* The remaining cases can only occur in pattern mode: */

	case WILDCARD	: inferType(aVar,newTyvars(1));
			  break;

	case ASPAT	: {   Int beta = newTyvars(1);
			      snd(snd(e)) = typeExpr(l,snd(snd(e)));
			      bindTv(beta,typeIs,typeOff);
			      check(l,fst(snd(e)),e,aspat,aVar,beta);
			      tyvarType(beta);
			  }
			  break;

	case LAZYPAT	: snd(e) = typeExpr(l,snd(e));
			  break;

	case ADDPAT	: {   Int alpha = newTyvars(1);
			      inferType(typeVarToVar,alpha);
			      return ap(e,assumeEvid(predIntegral,alpha));
			  }

	default 	: internal("typeExpr");
   }

   return e;
}

/* --------------------------------------------------------------------------
 * Typing rules for particular special forms:
 * ------------------------------------------------------------------------*/

static Cell local typeAp(l,e)		/* Type check application, which   */
Int  l;					/* may be headed with a variable   */
Cell e; {				/* requires polymorphism, qualified*/
    static String app = "application";	/* types, and possible rank2 args. */
    Cell h = getHead(e);
    Int  n = argCount;
    Cell p = NIL;
    Cell a = e;
    Int  i;
    List svPreds;

    switch (whatIs(h)) {
	case NAME      : typeIs = name(h).type;
			 break;

	case VAROPCELL :
	case VARIDCELL : if (tcMode==NEW_PATTERN) {
			     inferType(aVar,newVarsBind(e));
			 }
			 else {
			     Cell v = findAssum(textOf(h));
			     if (nonNull(v)) {
				 h      = v;
				 typeIs = (tcMode==OLD_PATTERN)
						? defType(typeIs)
						: useType(typeIs);
			     }
			     else {
				 h = findName(textOf(h));
				 if (isNull(h))
				     internal("typeAp0");
				 typeIs = name(h).type;
			     }
			 }
			 break;
	case IPVAR    :	{   Text t    = textOf(h);
			    Int alpha = newTyvars(1);
			    Cell ip   = pair(ap(IPCELL,t),aVar);
			    Cell ev   = assumeEvid(ip,alpha);
			    typeIs    = mkInt(alpha);
			    h	      = ap(h,ev);
			}
			break;

	default        : h = typeExpr(l,h);
			 break;
    }

    if (isNull(typeIs))
       internal("typeAp1");

    instantiate(typeIs);		/* Deal with polymorphism ...	   */
    if (nonNull(predsAre)) {		/* ... and with qualified types.   */
	List evs = NIL;
	for (; nonNull(predsAre); predsAre=tl(predsAre)) {
	    evs = cons(assumeEvid(hd(predsAre),typeOff),evs);
	}
	if (!isName(h) || !isCfun(h)) {
	    h = applyToArgs(h,rev(evs));
	}
    }

    if (whatIs(typeIs)==CDICTS) {	/* Deal with local dictionaries	   */
	List evs = makePredAss(fst(snd(typeIs)),typeOff);
	List ps  = evs;
	typeIs   = snd(snd(typeIs));
	for (; nonNull(ps); ps=tl(ps)) {
	    h = ap(h,thd3(hd(ps)));
	}
	if (tcMode==EXPRESSION) {
	    preds = revOnto(evs,preds);
	} else {
	    hd(localEvs) = revOnto(evs,hd(localEvs));
	}
    }

    if (whatIs(typeIs)==EXIST) {	/* Deal with existential arguments */
	Int n  = intOf(fst(snd(typeIs)));
	typeIs = snd(snd(typeIs));
	if (!isCfun(getHead(h)) || n>typeFree) {
	    internal("typeAp2");
	} else if (tcMode!=EXPRESSION) {
	    Int alpha = typeOff + typeFree;
	    for (; n>0; n--) {
		bindTv(alpha-n,SKOLEM,0);
		hd(skolVars) = cons(pair(mkInt(alpha-n),e),hd(skolVars));
	    }
	}
    }

    if (whatIs(typeIs)==RANK2) {	/* Deal with rank 2 arguments	   */
	Int  alpha = typeOff;
	Int  m     = typeFree;
	Int  nr2   = intOf(fst(snd(typeIs)));
	Type body  = snd(snd(typeIs));
	List as    = e;
	Bool added = FALSE;

	if (n<nr2) {			/* Must have enough arguments	   */
	    ERRMSG(l)   "Use of " ETHEN ERREXPR(h);
	    if (n>1) {
		ERRTEXT " in "    ETHEN ERREXPR(e);
	    }
	    ERRTEXT     " requires at least %d argument%s\n",
			nr2, (nr2==1 ? "" : "s")
	    EEND;
	}

	for (i=nr2; i<n; ++i)		/* Find rank two arguments	   */
	    as = fun(as);

	for (as=getArgs(as); nonNull(as); as=tl(as), body=arg(body)) {
	    Type expect = dropRank1(arg(fun(body)),alpha,m);
	    if (isPolyOrQualType(expect)) {
		if (tcMode==EXPRESSION)		/* poly/qual type in expr  */
		    hd(as) = typeExpected(l,app,hd(as),expect,alpha,m,TRUE);
		else if (hd(as)!=WILDCARD) {	/* Pattern binding/match   */
		    if (!isVar(hd(as))) {
			ERRMSG(l) "Argument "    ETHEN ERREXPR(arg(as));
			ERRTEXT   " in pattern " ETHEN ERREXPR(e);
			ERRTEXT   " where a variable is required\n"
			EEND;
		    }
		    if (tcMode==NEW_PATTERN) {	/* Pattern match	   */
			if (m>0 && !added) {
			    for (i=0; i<m; i++)
				addVarAssump(dummyVar,mkInt(alpha+i));
			    added = TRUE;
			}
			addVarAssump(hd(as),expect);
		    }
		    else {			/* Pattern binding	   */
			Text t = textOf(hd(as));
			Cell a = findInAssumList(t,hd(defnBounds));
			if (isNull(a))
			    internal("typeAp3");
			instantiate(expect);
			if (nonNull(predsAre)) {
			    ERRMSG(l) "Cannot use pattern binding for " ETHEN
			    ERREXPR(hd(as));
			    ERRTEXT   " as a component with a qualified type\n"
			    EEND;
			}
			shouldBe(l,hd(as),e,app,aVar,intOf(defType(snd(a))));
		    }
		}
	    }
	    else {				/* Not a poly/qual type	   */
		spCheck(l,hd(as),e,app,expect,alpha);
	    }
	    h = ap(h,hd(as));			/* Save checked argument   */
	}
	inferType(body,alpha);
	n -= nr2;
    }

    if (n>0) {				/* Deal with remaining args	   */
	Int beta = funcType(n);		/* check h::t1->t2->...->tn->rn+1  */
	shouldBe(l,h,e,app,aVar,beta);
	for (i=n; i>0; --i) {		/* check e_i::t_i for each i	   */
	    spCheck(l,arg(a),e,app,aVar,beta+2*i-1);
	    p = a;
	    a = fun(a);
	}
	tyvarType(beta+2*n);		/* Inferred type is r_n+1          */
    }

    if (isNull(p))			/* Replace head with translation   */
	e = h;
    else
	fun(p) = h;

    return e;
}

static Cell local typeExpected(l,wh,e,reqd,alpha,n,addEvid)
Int    l;				/* Type check expression e in wh   */
String wh;				/* at line l, expecting type reqd, */
Cell   e;				/* and treating vars alpha through */
Type   reqd;				/* (alpha+n-1) as fixed.	   */
Int    alpha;
Int    n;
Bool   addEvid; {			/* TRUE => add \ev -> ...	   */
    List savePreds = preds;
    Type t;
    Int  o;
    Int  m;
    List ps;
    Int  i;

    instantiate(reqd);
    t     = typeIs;
    o     = typeOff;
    m     = typeFree;
    ps    = makePredAss(predsAre,o);

    preds = NIL;
    check(l,e,NIL,wh,t,o);
    improve(l,ps,preds);

    clearMarks();
    mapProc(markAssumList,defnBounds);
    mapProc(markAssumList,varsBounds);
    mapProc(markPred,savePreds);
    markBtyvs();

    if (n > 0) {		  /* mark alpha thru alpha+n-1, plus any   */
				  /* type vars that are functionally	   */
	List us = NIL, vs = NIL;  /* dependent on them			   */
	List fds = calcFunDepsPreds(preds);
	for (i=0; i<n; i++) {
	    Type t1 = zonkTyvar(alpha+i);
	    us = zonkTyvarsIn(t1,us);
	}
	vs = oclose(fds,us);
	for (; nonNull(vs); vs=tl(vs))
	    markTyvar(intOf(hd(vs)));
    }

    normPreds(l);
    savePreds = elimPredsUsing(ps,savePreds);
    if (nonNull(preds) && resolveDefs(genvarType(t,o,NIL),FALSE))
	savePreds = elimPredsUsing(ps,savePreds);
    elimTauts();
    if (nonNull(preds)) {
	Type ty = copyType(t,o);
	List qs = copyPreds(ps);
	cantEstablish(l,wh,e,ty,qs);
    }

    resetGenerics();
    for (i=0; i<m; i++)
	if (copyTyvar(o+i)!=mkOffset(i)) {
	    List qs = copyPreds(ps);
	    Type it = copyType(t,o);
	    tooGeneral(l,e,reqd,generalize(qs,it));
	}

    if (addEvid) {
	e     = qualifyExpr(l,ps,e);
	preds = savePreds;
    }
    else
	preds = revOnto(ps,savePreds);

    inferType(t,o);
    return e;
}

static Void local typeAlt(wh,e,a,t,o,m)	/* Type check abstraction (Alt)	   */
String wh;				/* a = ( [p1, ..., pn], rhs )	   */
Cell   e;
Cell   a;
Type   t;
Int    o;
Int    m; {
    Type origt = t;
    List ps    = fst(a) = patBtyvs(fst(a));
    Int  n     = length(ps);
    Int  l     = rhsLine(snd(a));
    Int  nr2   = 0;
    List as    = NIL;
    Bool added = FALSE;

    saveVarsAss();
    enterSkolVars();
    if (whatIs(t)==RANK2) {
	if (n<(nr2=intOf(fst(snd(t))))) {
	    ERRMSG(l) "Definition requires at least %d parameters on lhs",
		      intOf(fst(snd(t)))
	    EEND;
	}
	t = snd(snd(t));
    }

    while (getHead(t)==typeArrow && argCount==2 && nonNull(ps)) {
	Type ta = arg(fun(t));
	if (isPolyOrQualType(ta)) {
	    if (hd(ps)!=WILDCARD) {
		if (!isVar(hd(ps))) {
		   ERRMSG(l) "Argument " ETHEN ERREXPR(hd(ps));
		   ERRTEXT   " used where a variable or wildcard is required\n"
		   EEND;
		}
		if (m>0 && !added) {
		    Int i = 0;
		    for (; i<m; i++)
			addVarAssump(dummyVar,mkInt(o+i));
		    added = TRUE;
		}
		addVarAssump(hd(ps),ta);
	    }
	}
	else {
	    hd(ps) = typeFreshPat(l,hd(ps));
	    shouldBe(l,hd(ps),NIL,wh,ta,o);
	}
	t  = arg(t);
	ps = tl(ps);
	as = fn(ta,as);
	n--;
    }

    if (n==0)
	snd(a) = typeRhs(snd(a));
    else {
	Int beta = funcType(n);
	Int i    = 0;
	for (; i<n; ++i) {
	    hd(ps) = typeFreshPat(l,hd(ps));
	    bindTv(beta+2*i+1,typeIs,typeOff);
	    ps = tl(ps);
	}
	snd(a) = typeRhs(snd(a));
	bindTv(beta+2*n,typeIs,typeOff);
	tyvarType(beta);
    }

    if (!unify(typeIs,typeOff,t,o)) {
	Type req, got;
	clearMarks();
	req = liftRank2(origt,o,m);
	liftRank2Args(as,o,m);
	got = ap(RANK2,pair(mkInt(nr2),revOnto(as,copyType(typeIs,typeOff))));
	reportTypeError(l,e,NIL,wh,got,req);
    }

    restoreVarsAss();
    doneBtyvs(l);
    leaveSkolVars(l,origt,o,m);
}

static Int local funcType(n)		/*return skeleton for function type*/
Int n; {				/*with n arguments, taking the form*/
    Int beta = newTyvars(2*n+1);	/*    r1 t1 r2 t2 ... rn tn rn+1   */
    Int i;				/* with r_i := t_i -> r_i+1	   */
    for (i=0; i<n; ++i)
	bindTv(beta+2*i,arrow,beta+2*i+1);
    return beta;
}

static Void local typeCase(l,beta,c)   /* type check case: pat -> rhs	   */
Int  l; 			       /* (case given by c == (pat,rhs))   */
Int  beta;			       /* need:  pat :: (var,beta)	   */
Cell c; {			       /*	 rhs :: (var,beta+1)	   */
    static String casePat  = "case pattern";
    static String caseExpr = "case expression";

    saveVarsAss();
    enterSkolVars();
    fst(c) = typeFreshPat(l,patBtyvs(fst(c)));
    shouldBe(l,fst(c),NIL,casePat,aVar,beta);
    snd(c) = typeRhs(snd(c));
    shouldBe(l,rhsExpr(snd(c)),NIL,caseExpr,aVar,beta+1);

    restoreVarsAss();
    doneBtyvs(l);
    leaveSkolVars(l,typeIs,typeOff,0);
}

static Void local typeComp(l,m,e,qs)	/* type check comprehension	   */
Int  l;
Type m;					/* monad (mkOffset(0))		   */
Cell e;
List qs; {
    static String boolQual = "boolean qualifier";
    static String genQual  = "generator";
    List svPreds;

    STACK_CHECK
    if (isNull(qs)) {			/* no qualifiers left		   */
	spTypeExpr(l,fst(e));
    } else {
	Cell q   = hd(qs);
	List qs1 = tl(qs);
	switch (whatIs(q)) {
	    case BOOLQUAL : spCheck(l,snd(q),NIL,boolQual,typeBool,0);
			    typeComp(l,m,e,qs1);
			    break;

	    case QWHERE   : enterBindings();
			    enterSkolVars();
			    mapProc(typeBindings,snd(q));
			    typeComp(l,m,e,qs1);
			    leaveBindings();
			    leaveSkolVars(l,typeIs,typeOff,0);
			    break;

	    case FROMQUAL : {   Int beta = newTyvars(1);
				saveVarsAss();
				enterPendingBtyvs();
				spCheck(l,snd(snd(q)),NIL,genQual,m,beta);
				enterSkolVars();
				fst(snd(q))
				    = typeFreshPat(l,patBtyvs(fst(snd(q))));
				shouldBe(l,fst(snd(q)),NIL,genQual,aVar,beta);
				typeComp(l,m,e,qs1);
				restoreVarsAss();
			        leavePendingBtyvs();
				leaveSkolVars(l,typeIs,typeOff,0);
			    }
			    break;

	    case DOQUAL   : spCheck(l,snd(q),NIL,genQual,m,newTyvars(1));
			    typeComp(l,m,e,qs1);
			    break;
	}
    }
}


/* --------------------------------------------------------------------------
 * Parallel comprehensions
 *
 * This is an extension to the standard list comprehension notation,
 * allowing parallel lists of qualifiers that generate independently
 * of each other.  Parallel qualifier lists are separated with additional
 * `|' symbols:
 *     [ e | p1 <- e11, p2 <- e12, ...
 *         | q1 <- e21, q2 <- e22, ...
 *         ... ]
 *
 * The meaning of a parallel comprehension can be defined in terms of zip
 * and a regular comprehension:
 *     [ e | ((p1,p2), (q1,q2)) <- zip [(p1,p2) | p1 <- e11, p2 <- e12]
 *                                     [(q1,q2) | q1 <- e21, q2 <- e22]]
 * The use of let-bindings in the qualifier lists complicates this slightly.
 * Let-bound variables will scope over the rest of their qualifier list
 * as well as `e', but not over any part of a parallel qualifier list.
 * We can express this in the translation by including in the derived
 * patterns all the let-bound variables:
 *     [ e | p1 <- e11, let v1 = e12, p2 <- e13
 *         | q1 <- e21, let v2 = e22, q2 <- e23]
 *     =>
 *     [ e | ((p1,v1,p2), (q1,v2,q2)) <-
 *               zip [(p1,v1,p2) | p1 <- e11, let v1 = e12, p2 <- e13]
 *                   [(q1,v2,q2) | q1 <- e21, let v2 = e22, q2 <- e23]]
 * Where it is understood that the `v' patterns in the tuples are given
 * rank-2 types, so we don't lose the polymorphism.  We also have to be
 * careful to preserve any shadowing.
 *
 * ZZ We aren't dealing with WHEREs correctly...
 * ZZ We aren't dealing with skolem vars correctly...
 * ------------------------------------------------------------------------*/

static List gatheredAss;
static List gatheredDefns;
static List gatheredTyvars;
static List gatheredPTyvars;

#define enterGathering() List svGA = gatheredAss, svGD = gatheredDefns, svGT = gatheredTyvars, svGP = gatheredPTyvars; gatheredAss = gatheredDefns = gatheredTyvars = gatheredPTyvars = NIL
#define leaveGathering() gatheredAss = svGA; gatheredDefns = svGD; gatheredTyvars = svGT; gatheredPTyvars = svGP

static List local getPats(bs)
List bs; {
    List ps = NIL;
    for (; nonNull(bs); bs=tl(bs)) {
	ps = cons(fst(hd(bs)), ps);
    }
    return ps;
}

static Cell local tupleUp(xs)
List xs; {
    Int n = length(xs);
    if (n == 0)
	return nameUnit;
    else if (n == 1)
	return hd(xs);
    else {
	Cell x = mkTuple(n);
	for (; nonNull(xs); xs=tl(xs))
	    x = ap(x, hd(xs));
	return x;
    }
}

static Cell local typeZComp(l,m,e,qss)	/* type check comprehension	   */
Int l;
Type m;					/* monad (mkOffset(0))		   */
Cell e;
List qss; {
    List pss, ass;
    List zpat, zexp;
    Int  len;
    Text zName;
    List svPreds;
    enterGathering();
    enterBindings();
    for (pss = qss;nonNull(pss);pss=tl(pss)) {
	gatheredAss = cons(NIL,gatheredAss);
	gatheredDefns = cons(NIL,gatheredDefns);
	typeCompy(l,m,hd(pss));
	/* reset for next list of qualifiers */
	hd(varsBounds) = NIL;
    }
    /* add gathered vars */
    hd(varsBounds) = revOnto(concat(gatheredAss),hd(varsBounds));
    enterBindings();
    hd(defnBounds) = revOnto(concat(gatheredDefns),hd(defnBounds));
    enterPendingBtyvs();
    hd(btyvars) = gatheredTyvars;
    hd(pendingBtyvs) = gatheredPTyvars;
    spTypeExpr(l,fst(e));
    leavePendingBtyvs();
    leaveBindings();
    leaveBindings();

    /* now, we construct a regular comprehension out of the parallel one */
    len   = length(qss);
    zName = zipName(len);
    zpat  = mkTuple(len);
    zexp  = findQualFun(findText("Data.List"),zName);
    if (isNull(zexp))
	zexp = findQualFun(findText("List"),zName);
    if (isNull(zexp)) {
	/* if they don't have List loaded, we can still handle the most
	   common cases, because `zip' and `zip3' are defined in the Prelude */
	zexp = findQualFun(textPrelude,zName);
    }
    if (isNull(zexp)) {
	ERRMSG(l) "\"%s\" not in scope (introduced by parallel comprehension)", textToStr(zName) ETHEN
	ERRTEXT   "\n*** Possible cause: \"List\" module not imported"
	EEND;
    
    }
    for (pss=qss, ass=rev(gatheredAss);nonNull(pss);pss=tl(pss), ass=tl(ass)) {
	List ps = tupleUp(getPats(hd(ass)));
	zpat = ap(zpat, ps);
	zexp = ap(zexp, ap(MONADCOMP,pair(nameListMonad,pair(ps, hd(pss)))));
    }
    leaveGathering();
    return pair(fst(e),singleton(ap(FROMQUAL,pair(zpat,zexp))));
}

static Void local typeCompy(l,m,qs)	/* type check comprehension	   */
Int  l;
Type m;					/* monad (mkOffset(0))		   */
List qs; {
    static String boolQual = "boolean qualifier";
    static String genQual  = "generator";
    List svPreds;

    STACK_CHECK
    if (!isNull(qs)) {			/* no qualifiers left		   */
	Cell q   = hd(qs);
	List qs1 = tl(qs);
	switch (whatIs(q)) {
	    case BOOLQUAL : spCheck(l,snd(q),NIL,boolQual,typeBool,0);
			    typeCompy(l,m,qs1);
			    break;

	    case QWHERE   : enterBindings();
			    enterSkolVars();
			    mapProc(typeBindings,snd(q));
			    hd(gatheredAss) = dupOnto(hd(varsBounds),hd(gatheredAss));
			    /* ZZ what is gatheredDefns used for ??? */
			    hd(gatheredDefns) = dupOnto(hd(defnBounds),hd(gatheredDefns));
			    typeCompy(l,m,qs1);
			    leaveBindings();
			    leaveSkolVars(l,typeIs,typeOff,0);
			    break;

	    case FROMQUAL : {   Int beta = newTyvars(1);
				enterPendingBtyvs();
				spCheck(l,snd(snd(q)),NIL,genQual,m,beta);
				enterSkolVars();
				fst(snd(q))
				    = typeFreshPat(l,patBtyvs(fst(snd(q))));
				shouldBe(l,fst(snd(q)),NIL,genQual,aVar,beta);
				hd(gatheredAss) = dupOnto(hd(varsBounds),hd(gatheredAss));
				gatheredTyvars = dupOnto(hd(btyvars),gatheredTyvars);
				gatheredPTyvars = dupOnto(hd(pendingBtyvs),gatheredPTyvars);
				typeCompy(l,m,qs1);
			        leavePendingBtyvsQuietly();
				leaveSkolVars(l,typeIs,typeOff,0);
			    }
			    break;

	    case DOQUAL   : spCheck(l,snd(q),NIL,genQual,m,newTyvars(1));
			    typeCompy(l,m,qs1);
			    break;

	    default:	    internal("typeComp");
	}
    }
}

static Cell local typeMonadComp(l,e)	/* type check monad comprehension  */
Int  l;
Cell e; {
    Int  alpha	      = newTyvars(1);
    Int  beta	      = newTyvars(1);
    Cell mon	      = ap(mkInt(beta),aVar);
    Cell m	      = assumeEvid(predMonad,beta);
    tyvar(beta)->kind = starToStar;
    bindTv(beta,typeList,0);
    m = nameListMonad;

    typeComp(l,mon,snd(e),snd(snd(e)));
    bindTv(alpha,typeIs,typeOff);
    inferType(mon,alpha);
    return ap(MONADCOMP,pair(m,snd(e)));
}

static Cell local typeMonadZComp(l,e)	/* type check monad comprehension  */
Int  l;
Cell e; {
    Int  alpha	      = newTyvars(1);
    Int  beta	      = newTyvars(1);
    Cell mon	      = ap(mkInt(beta),aVar);
    Cell m	      = assumeEvid(predMonad,beta);
    Cell new;
    tyvar(beta)->kind = starToStar;
    bindTv(beta,typeList,0);
    m = nameListMonad;

    new = typeZComp(l,mon,snd(e),snd(snd(e)));
    bindTv(alpha,typeIs,typeOff);
    inferType(mon,alpha);
    return ap(MONADCOMP,pair(m,new));
}

static Void local typeDo(l,e)		/* type check do-notation	   */
Int  l;
Cell e; {
    static String finGen = "final generator";
    Int  alpha		 = newTyvars(1);
    Int  beta		 = newTyvars(1);
    Cell mon		 = ap(mkInt(beta),aVar);
    Cell m		 = assumeEvid(predMonad,beta);
    tyvar(beta)->kind    = starToStar;

    typeComp(l,mon,snd(e),snd(snd(e)));
    shouldBe(l,fst(snd(e)),NIL,finGen,mon,alpha);
    snd(e) = pair(m,snd(e));
}


#define segRecs(seg)	fst3(fst(seg))
#define segExps(seg)	snd3(fst(seg))
#define segDefs(seg)	thd3(fst(seg))
#define segQuals(seg)	snd(seg)

static Void local typeRecComp(l,m,e,qs)		/* type check rec-comp	     */
Int  l;
Type m;						/* monad (mkOffset(0))	     */
Cell e;
List qs; {
    static String boolQual = "boolean qualifier";
    static String genQual  = "generator";
    static String letQual  = "mdo-transformed let generator";
    String mesg		   = genQual;

    List svPreds;


    STACK_CHECK
    if (isNull(qs)) {			/* no qualifiers left		   */
	spTypeExpr(l,fst(e));
    } else {
	Cell q   = hd(qs);
	List qs1 = tl(qs);
	switch (whatIs(q)) {
	    case BOOLQUAL : spCheck(l,snd(q),NIL,boolQual,typeBool,0);
			    typeRecComp(l,m,e,qs1);
			    break;

	    case QWHERE	  : mesg = letQual;
			    fst(q) = FROMQUAL;
			    
			    /* intentional fall-thru */
	    case FROMQUAL : {   Int beta = newTyvars(1);
				saveVarsAss();
				enterPendingBtyvs();
				spCheck(l,snd(snd(q)),NIL,mesg,m,beta);
				enterSkolVars();

				tcMode = OLD_PATTERN;
				fst(snd(q))
				    = typeExpr(l,patBtyvs(fst(snd(q))));
				tcMode = EXPRESSION;

				shouldBe(l,fst(snd(q)),NIL,mesg,aVar,beta);
				typeRecComp(l,m,e,qs1);
				restoreVarsAss();
				leavePendingBtyvs();
				leaveSkolVars(l,typeIs,typeOff,0);
			    }
			    break;

	    case DOQUAL   : spCheck(l,snd(q),NIL,genQual,m,newTyvars(1));
			    typeRecComp(l,m,e,qs1);
			    break;
	}
    }
}

static Void local typeMDo(l,e)		/* type check recursive-do	   */
Int l;
Cell e; {
    String fixLib     = "Control.Monad.Fix";
    String fixClass   = "MonadFix";

    if( !classMonadRec ) {
	ERRMSG(0) "%s class not defined", fixClass ETHEN
        ERRTEXT   "\n*** Possible cause: \"%s\" module not imported", fixLib
	EEND;
    }

    predMonadRec = ap(classMonadRec,aVar);

    /* Now we're safe: do the actual type-checking now: */
    typeRecursiveDo(l,e);
}

static Void local typeRecursiveDo(l,e)	/* type check recursive-do exp.   */
Int l;
Cell e; {
    /* The structure at this point:
	e = (TAG, (1, [((2, 3, 4), 5)]))
	where
	    1 = expression
	    2 = rec vars of the segment
	    3 = exported vars of the segment
	    4 = defined vars of the segment
	    5 = qualifiers
    */
    static String finGen = "final generator";
    Int alpha		 = newTyvars(1);
    Int beta		 = newTyvars(1);
    Cell mon		 = ap(mkInt(beta),aVar);
    Cell monDict         = assumeEvid(predMonad,beta); 
    Cell m		 = assumeEvid(predMonadRec,beta);
    List tmp;
    List whole		 = NIL;

    tyvar(beta)->kind	 = starToStar;
    
    enterBindings();

    /* introduce defined variables into the typing environment: */
    for(tmp = snd(snd(e)); nonNull(tmp); tmp = tl(tmp)) {
	List rtmp = segDefs(hd(tmp));
	for(; nonNull(rtmp); rtmp = tl(rtmp)) {
	    newVarsBind(hd(rtmp)); 
	}
    }

    /* collect all qualifiers from all segments: */
    for(tmp = snd(snd(e)); nonNull(tmp); tmp = tl(tmp)) {
	List tmp2;
	for(tmp2 = segQuals(hd(tmp)); nonNull(tmp2); tmp2 = tl(tmp2)) {
	    whole=cons(hd(tmp2),whole);
	}
    }
	
    typeRecComp(l,mon,snd(e),rev(whole));
    shouldBe(l,fst(snd(e)),NIL,finGen,mon,alpha);
    leaveBindings();

    snd(e) = pair(pair(m,monDict),snd(e));
}

#undef segRecs
#undef segExps
#undef segDefs
#undef segQuals

static Void local typeConFlds(l,e)	/* Type check a construction	   */
Int  l;
Cell e; {
    static String conExpr = "value construction";
    Name c  = fst(snd(e));
    List fs = snd(snd(e));
    Type tc;
    Int  to;
    Int  tf;
    Int  i;
    List svPreds;

    instantiate(name(c).type);
    for (; nonNull(predsAre); predsAre=tl(predsAre))
	assumeEvid(hd(predsAre),typeOff);
    if (whatIs(typeIs)==RANK2)
	typeIs = snd(snd(typeIs));
    tc = typeIs;
    to = typeOff;
    tf = typeFree;

    for (; nonNull(fs); fs=tl(fs)) {
	Type t = tc;
	for (i=sfunPos(fst(hd(fs)),c); --i>0; t=arg(t))
	    ;
	t = dropRank1(arg(fun(t)),to,tf);
	if (isPolyOrQualType(t))
	    snd(hd(fs)) = typeExpected(l,conExpr,snd(hd(fs)),t,to,tf,TRUE);
	else {
	    spCheck(l,snd(hd(fs)),e,conExpr,t,to);
	}
    }
    for (i=name(c).arity; i>0; i--)
	tc = arg(tc);
    inferType(tc,to);
}

static Void local typeUpdFlds(line,e)	/* Type check an update		   */
Int  line;				/* (Written in what might seem a   */
Cell e; {				/* bizarre manner for the benefit  */
    static String update = "update";	/* of as yet unreleased extensions)*/
    List cs    = snd3(snd(e));		/* List of constructors		   */
    List fs    = thd3(snd(e));		/* List of field specifications	   */
    List ts    = NIL;			/* List of types for fields	   */
    Int  n     = length(fs);
    Int  alpha = newTyvars(2+n);
    Int  i;
    List fs1;
    List svPreds;

    /* Calculate type and translation for each expr in the field list	   */
    for (fs1=fs, i=alpha+2; nonNull(fs1); fs1=tl(fs1), i++) {
	spTypeExpr(line,snd(hd(fs1)));
	bindTv(i,typeIs,typeOff);
    }

    clearMarks();
    mapProc(markAssumList,defnBounds);
    mapProc(markAssumList,varsBounds);
    mapProc(markPred,preds);
    markBtyvs();

    for (fs1=fs, i=alpha+2; nonNull(fs1); fs1=tl(fs1), i++) {
	resetGenerics();
	ts = cons(generalize(NIL,copyTyvar(i)),ts);
    }
    ts = rev(ts);

    /* Type check expression to be updated				   */
    spTypeExpr(line,fst3(snd(e)));
    bindTv(alpha,typeIs,typeOff);

    for (; nonNull(cs); cs=tl(cs)) {	/* Loop through constrs		   */
	Name c  = hd(cs);
	List ta = replicate(name(c).arity,NIL);
	Type td, tr;
	Int  od, or;

	tcMode = NEW_PATTERN;		/* Domain type			   */
	instantiate(name(c).type);
	tcMode = EXPRESSION;
	td     = typeIs;
	od     = typeOff;
	for (; nonNull(predsAre); predsAre=tl(predsAre))
	    assumeEvid(hd(predsAre),typeOff);

	if (whatIs(td)==RANK2)	/* Skip rank2 annotation, if any   */
	    td = snd(snd(td));

	instantiate(name(c).type);	/* Range type			   */
	tr = typeIs;
	or = typeOff;
	for (; nonNull(predsAre); predsAre=tl(predsAre))
	    assumeEvid(hd(predsAre),typeOff);

	if (whatIs(tr)==RANK2)	/* Skip rank2 annotation, if any   */
	    tr = snd(snd(tr));

	for (fs1=fs, i=1; nonNull(fs1); fs1=tl(fs1), i++) {
	    Int n    = sfunPos(fst(hd(fs1)),c);
	    Cell ta1 = ta;
	    for (; n>1; n--)
		ta1 = tl(ta1);
	    hd(ta1) = mkInt(i);
	}

	for (; nonNull(ta); ta=tl(ta)) {	/* For each cfun arg	   */
	    if (nonNull(hd(ta))) {		/* Field to updated?	   */
		Int  n = intOf(hd(ta));
		Cell f = fs;
		Cell t = ts;
		for (; n-- > 1; f=tl(f), t=tl(t))
		    ;

		if (isPolyOrQualType(arg(fun(td)))) {
		    ERRMSG(line) "Sorry, record update syntax cannot currently be used for polymorphic components"
		    EEND;
		}

		f = hd(f);
		t = hd(t);
		instantiate(t);
		shouldBe(line,snd(f),e,update,arg(fun(tr)),or);
	    }					/* Unmentioned component   */
	    else if (!unify(arg(fun(td)),od,arg(fun(tr)),or))
		internal("typeUpdFlds");

	    tr = arg(tr);
	    td = arg(td);
	}

	inferType(td,od);			/* Check domain type	   */
	shouldBe(line,fst3(snd(e)),e,update,aVar,alpha);
	inferType(tr,or);			/* Check range type	   */
	shouldBe(line,e,NIL,update,aVar,alpha+1);
    }
    /* (typeIs,typeOff) still carry the result type when we exit the loop  */
}

static Cell local typeWith(line,e)	/* Type check a with		   */
Int  line;
Cell e; {
    List fs    = snd(snd(e));		/* List of field specifications	   */
    Int  n     = length(fs);
    Int  alpha = newTyvars(2+n);
    Int  i;
    List fs1;
    Cell tIs;
    Cell tOff;
    List dpreds = NIL, dp;
    Cell bs = NIL;

    /* Type check expression to be updated				   */
    fst(snd(e)) = typeExpr(line,fst(snd(e)));
    bindTv(alpha,typeIs,typeOff);
    tIs = typeIs;
    tOff = typeOff;
    /* elim duplicates */
    improve(line,NIL,preds);
    preds = scSimplify(preds);
    /* extract preds that we're going to bind */
    for (fs1=fs; nonNull(fs1); fs1=tl(fs1)) {
        Text t = textOf(fst(hd(fs1)));
	Cell p = findIPEvid(t);
	dpreds = cons(p, dpreds);
	if (nonNull(p)) {
	    removeIPEvid(t);
	} else {
	    /* maybe give a warning message here... */
	}
    }
    dpreds = rev(dpreds);

    /* Calculate type and translation for each expr in the field list	   */
    for (fs1=fs, dp=dpreds, i=alpha+2; nonNull(fs1); fs1=tl(fs1), dp=tl(dp), i++) {
	static String with = "with";
        Cell ev = hd(dp);
	snd(hd(fs1)) = typeExpr(line,snd(hd(fs1)));
	bindTv(i,typeIs,typeOff);
	if (nonNull(ev)) {
	    shouldBe(line,fst(hd(fs1)),e,with,snd(fst3(ev)),intOf(snd3(ev)));
	    bs = cons(cons(pair(thd3(ev), cons(triple(NIL, mkInt(line), snd(hd(fs1))), NIL)), NIL), bs);
	}
    }
    typeIs = tIs;
    typeOff = tOff;
    return (ap(LETREC,pair(bs,fst(snd(e)))));
}

static Cell local typeFreshPat(l,p)    /* find type of pattern, assigning  */
Int  l; 			       /* fresh type variables to each var */
Cell p; {			       /* bound in the pattern		   */
    tcMode = NEW_PATTERN;
    p	   = typeExpr(l,p);
    tcMode = EXPRESSION;
    return p;
}

/* --------------------------------------------------------------------------
 * Type check group of bindings:
 * ------------------------------------------------------------------------*/

static Void local typeBindings(bs)	/* type check a binding group	   */
List bs; {
    Bool usesPatBindings = FALSE;	/* TRUE => pattern binding in bs   */
    Bool usesUntypedVar  = FALSE;	/* TRUE => var bind w/o type decl  */
    List bs1;

    /* The following loop is used to determine whether the monomorphism	   */
    /* restriction should be applied.  It could be written marginally more */
    /* efficiently by using breaks, but clarity is more important here ... */

    for (bs1=bs; nonNull(bs1); bs1=tl(bs1)) {  /* Analyse binding group    */
	Cell b = hd(bs1);
	if (!isVar(fst(b)))
	    usesPatBindings = TRUE;
	else if (isNull(fst(hd(snd(snd(b)))))		/* no arguments	   */
		 && whatIs(fst(snd(b)))==IMPDEPS)	/* implicitly typed*/
	    usesUntypedVar  = TRUE;
    }

    if (usesPatBindings || usesUntypedVar)
	monorestrict(bs);
    else
	unrestricted(bs);

    elimTauts();			       /* clean up any additional */
    					       /* tauts that arose due to */
					       /* late-stage `improvement' */

    mapProc(removeTypeSigs,bs);		       /* Remove binding type info */
    hd(varsBounds) = revOnto(hd(defnBounds),   /* transfer completed assmps*/
			     hd(varsBounds));  /* out of defnBounds        */
    hd(defnBounds) = NIL;
    hd(depends)    = NIL;
}

static Void local removeTypeSigs(b)    /* Remove type info from a binding  */
Cell b; {
    snd(b) = snd(snd(b));
}

/* --------------------------------------------------------------------------
 * Type check a restricted binding group:
 * ------------------------------------------------------------------------*/

static Void local monorestrict(bs)	/* Type restricted binding group   */
List bs; {
    List savePreds = preds;
    Int  line 	   = isVar(fst(hd(bs))) ? rhsLine(snd(hd(snd(snd(hd(bs))))))
					: rhsLine(snd(snd(snd(hd(bs)))));
    hd(defnBounds) = NIL;
    hd(depends)    = NODEPENDS;		/* No need for dependents here	   */

    preds = NIL;			/* Type check the bindings	   */
    mapProc(restrictedBindAss,bs);
    mapProc(typeBind,bs);
    improve(line,NIL,preds);
    normPreds(line);
    elimTauts();
    preds = revOnto(preds,savePreds);


    clearMarks();			/* Mark fixed variables		   */
    mapProc(markAssumList,tl(defnBounds));
    mapProc(markAssumList,tl(varsBounds));
    mapProc(markPred,preds);
    markBtyvs();

    if (isNull(tl(defnBounds))) {	/* Top-level may need defaulting   */
	normPreds(line);
	if (nonNull(preds) && resolveDefs(genvarAnyAss(hd(defnBounds)),FALSE))
	    elimTauts();

	clearMarks();
	reducePreds();
	improve(line,NIL,preds);
	if (nonNull(preds))
	    resolveDefs(NIL,FALSE);		/* Nearly Haskell 1.4?		   */
	elimTauts();

	if (nonNull(preds)) {		/* Look for unresolved overloading */
	    Cell v   = isVar(fst(hd(bs))) ? fst(hd(bs)) : hd(fst(hd(bs)));
	    Cell ass = findInAssumList(textOf(v),hd(varsBounds));
	    preds    = scSimplify(preds);

	    ERRMSG(line) "Unresolved top-level overloading" ETHEN
	    ERRTEXT     "\n*** Binding             : %s", textToStr(textOf(v))
	    ETHEN
	    if (nonNull(ass)) {
		ERRTEXT "\n*** Inferred type       : " ETHEN ERRTYPE(snd(ass));
	    }
	    ERRTEXT     "\n*** Outstanding context : " ETHEN
						ERRCONTEXT(copyPreds(preds));
	    ERRTEXT     "\n"
	    EEND;
	}
    }

    map1Proc(genBind,NIL,bs);		/* Generalize types of def'd vars  */
}

static Void local restrictedBindAss(b)	/* Make assums for vars in binding */
Cell b; {				/* gp with restricted overloading  */

    if (isVar(fst(b))) {		/* function-binding?		   */
	Cell t = fst(snd(b));
	if (whatIs(t)==IMPDEPS)	 {	/* Discard implicitly typed deps   */
	    fst(snd(b)) = t = NIL;	/* in a restricted binding group.  */
	}
	fst(snd(b)) = localizeBtyvs(t);
	restrictedAss(rhsLine(snd(hd(snd(snd(b))))), fst(b), t);
    } else {				/* pattern-binding?		   */
	List vs   = fst(b);
	List ts   = fst(snd(b));
	Int  line = rhsLine(snd(snd(snd(b))));

	for (; nonNull(vs); vs=tl(vs)) {
	    if (nonNull(ts)) {
		restrictedAss(line,hd(vs),hd(ts)=localizeBtyvs(hd(ts)));
		ts = tl(ts);
	    } else {
		restrictedAss(line,hd(vs),NIL);
	    }
	}
    }
}

static Void local restrictedAss(l,v,t) /* Assume that type of binding var v*/
Int  l; 			       /* is t (if nonNull) in restricted  */
Cell v; 			       /* binding group 		   */
Type t; {
    newDefnBind(v,t);
    if (nonNull(predsAre)) {
	ERRMSG(l) "Explicit overloaded type for \"%s\"",textToStr(textOf(v))
	ETHEN
	ERRTEXT   " not permitted in restricted binding"
	EEND;
    }
}

/* --------------------------------------------------------------------------
 * Unrestricted binding group:
 * ------------------------------------------------------------------------*/

static Void local unrestricted(bs)	/* Type unrestricted binding group */
List bs; {
    List savePreds = preds;
    List imps      = NIL;		/* Implicitly typed bindings	   */
    List exps      = NIL;		/* Explicitly typed bindings	   */
    List bs1;

    /* ----------------------------------------------------------------------
     * STEP 1: Separate implicitly typed bindings from explicitly typed	
     * bindings and do a dependency analyis, where f depends on g iff f
     * is implicitly typed and involves a call to g.
     * --------------------------------------------------------------------*/

    for (; nonNull(bs); bs=tl(bs)) {
	Cell b = hd(bs);
	if (whatIs(fst(snd(b)))==IMPDEPS)
	    imps = cons(b,imps);	/* N.B. New lists are built to	   */
	else				/* avoid breaking the original	   */
	    exps = cons(b,exps);	/* list structure for bs.	   */
    }

    for (bs=imps; nonNull(bs); bs=tl(bs)) {
	Cell b  = hd(bs);		/* Restrict implicitly typed dep   */
	List ds = snd(fst(snd(b)));	/* lists to bindings in imps	   */
	List cs = NIL;
	while (nonNull(ds)) {
	    bs1 = tl(ds);
	    if (cellIsMember(hd(ds),imps)) {
		tl(ds) = cs;
		cs     = ds;
	    }
	    ds = bs1;
	}
	fst(snd(b)) = cs;
    }
    imps = itbscc(imps);		/* Dependency analysis on imps	   */
    for (bs=imps; nonNull(bs); bs=tl(bs))
	for (bs1=hd(bs); nonNull(bs1); bs1=tl(bs1))
	    fst(snd(hd(bs1))) = NIL; 	/* reset imps type fields	   */

if (DEBUG_DEPENDS) {
    Printf("Binding group:");
    for (bs1=imps; nonNull(bs1); bs1=tl(bs1)) {
	Printf(" [imp:");
	for (bs=hd(bs1); nonNull(bs); bs=tl(bs))
	    Printf(" %s",textToStr(textOf(fst(hd(bs)))));
	Printf("]");
    }
    if (nonNull(exps)) {
	Printf(" [exp:");
	for (bs=exps; nonNull(bs); bs=tl(bs))
	    Printf(" %s",textToStr(textOf(fst(hd(bs)))));
	Printf("]");
    }
    Printf("\n");
}

    /* ----------------------------------------------------------------------
     * STEP 2: Add type assumptions about any explicitly typed variable.
     * --------------------------------------------------------------------*/

    for (bs=exps; nonNull(bs); bs=tl(bs)) {
	fst(snd(hd(bs))) = localizeBtyvs(fst(snd(hd(bs))));
	hd(varsBounds)   = cons(pair(fst(hd(bs)),fst(snd(hd(bs)))),
				hd(varsBounds));
    }

    /* ----------------------------------------------------------------------
     * STEP 3: Calculate types for each group of implicitly typed bindings.
     * --------------------------------------------------------------------*/

    for (; nonNull(imps); imps=tl(imps)) {
	Cell b   = hd(hd(imps));
	Int line = isVar(fst(b)) ? rhsLine(snd(hd(snd(snd(b)))))
				 : rhsLine(snd(snd(snd(b))));
	hd(defnBounds) = NIL;
	hd(depends)    = NIL;
	for (bs1=hd(imps); nonNull(bs1); bs1=tl(bs1))
	    newDefnBind(fst(hd(bs1)),NIL);

	preds = NIL;
	mapProc(typeBind,hd(imps));
	improve(line,NIL,preds);

	clearMarks();
	mapProc(markAssumList,tl(defnBounds));
	mapProc(markAssumList,tl(varsBounds));
	mapProc(markPred,savePreds);
	markBtyvs();

	normPreds(line);
	savePreds = elimOuterPreds(savePreds);
	if (nonNull(preds) && resolveDefs(genvarAllAss(hd(defnBounds)),FALSE)) {
	    savePreds = elimOuterPreds(savePreds);
	}

	map1Proc(genBind,preds,hd(imps));
	if (nonNull(preds)) {
	    map1Proc(addEvidParams,preds,hd(depends));
	    map1Proc(qualifyBinding,preds,hd(imps));
	}

	h98CheckInferredType(line,
			fst(hd(hd(defnBounds))),snd(hd(hd(defnBounds))));
	hd(varsBounds) = revOnto(hd(defnBounds),hd(varsBounds));
    }

    /* ----------------------------------------------------------------------
     * STEP 4: Now infer a type for each explicitly typed variable and
     * check for compatibility with the declared type.
     * --------------------------------------------------------------------*/

    for (; nonNull(exps); exps=tl(exps)) {
	static String extbind = "explicitly typed binding";
	Cell b    = hd(exps);
	List alts = snd(snd(b));
	Int  line = rhsLine(snd(hd(alts)));
	Type t;
	Int  o;
	Int  m;
	List ps;

	hd(defnBounds) = NIL;
	hd(depends)    = NODEPENDS;
	preds	       = NIL;

	instantiate(fst(snd(b)));
	o	       = typeOff;
	m	       = typeFree;
	t              = dropRank2(typeIs,o,m);
	ps	       = makePredAss(predsAre,o);

	enterPendingBtyvs();
	for (; nonNull(alts); alts=tl(alts))
	    typeAlt(extbind,fst(b),hd(alts),t,o,m);
	improve(line,ps,preds);
	leavePendingBtyvs();

	if (nonNull(ps))		/* Add dict params, if necessary   */
	    qualifyBinding(ps,b);

	clearMarks();
	mapProc(markAssumList,tl(defnBounds));
	mapProc(markAssumList,tl(varsBounds));
	mapProc(markPred,savePreds);
	markBtyvs();

	normPreds(line);
	savePreds = elimPredsUsing(ps,savePreds);
	if (nonNull(preds)) {
	    List vs = NIL;
	    Int  i  = 0;
	    for (; i<m; ++i)
		vs = cons(mkInt(o+i),vs);
	    if (resolveDefs(vs,FALSE)) {
		savePreds = elimPredsUsing(ps,savePreds);
	    }
	    if (nonNull(preds)) {
		clearMarks();
		reducePreds();
		if (nonNull(preds) && resolveDefs(vs,FALSE))
		    savePreds = elimPredsUsing(ps,savePreds);
	    }
	}

	resetGenerics();		/* Make sure we're general enough  */
	ps = copyPreds(ps);
	t  = generalize(ps,liftRank2(t,o,m));

	if (!sameSchemes(t,fst(snd(b))))
	    tooGeneral(line,fst(b),fst(snd(b)),t);
	h98CheckInferredType(line,fst(b),t);

	if (nonNull(preds))		/* Check context was strong enough */
	    cantEstablish(line,extbind,fst(b),t,ps);
    }

    preds	   = savePreds;			/* Restore predicates	   */
    hd(defnBounds) = NIL;
}

#define  SCC		 itbscc		/* scc for implicitly typed binds  */
#define  LOWLINK	 itblowlink
#define  DEPENDS(t)	 fst(snd(t))
#define  SETDEPENDS(c,v) fst(snd(c))=v
/* --------------------------------------------------------------------------
 * Strongly connected components algorithm for static.c.
 *
 * The Hugs 98 system is Copyright (c) Mark P Jones, Alastair Reid, the
 * Yale Haskell Group, and the OGI School of Science & Engineering at OHSU,
 * 1994-2003, All rights reserved.  It is distributed as free software under
 * the license in the file "License", which is included in the distribution.
 *
 * $RCSfile: scc.c,v $
 * $Revision: 1.5 $
 * $Date: 2003/10/14 13:56:24 $
 * ------------------------------------------------------------------------*/

#ifndef SCC_C
#define SCC_C
#define visited(d) (isInt(DEPENDS(d)))		/* binding already visited?*/

static Cell daSccs = NIL;
static Int  daCount;

static Int local sccMin	(Int,Int);

static Int local sccMin(x,y)		/* calculate minimum of x,y	   */
Int x, y; {				/* (unless y is zero)		   */
    return (x<=y || y==0) ? x : y;
}
#endif

/* --------------------------------------------------------------------------
 * A couple of parts of this program require an algorithm for sorting a list
 * of values (with some added dependency information) into a list of strongly
 * connected components in which each value appears before its dependents.
 *
 * The algorithm used here is based on those described in:
 * 1) Robert Tarjan, Depth-first search and Linear Graph Algorithms,
 *    SIAM J COMPUT, vol 1, no 2, June 1972, pp.146-160.
 * 2) Aho, Hopcroft and Ullman, Design and Analysis of Algorithms,
 *    Addison Wesley, 1972.  pp.189-195.
 * The version used here probably owes most to the latter presentation but
 * has been modified to simplify the algorithm and improve the use of space.
 *
 * This would probably have been a good application for C++ templates ...
 * ------------------------------------------------------------------------*/

static Int local LOWLINK (Cell);	/* local function		   */
static Int local LOWLINK(v)		/* calculate `lowlink' of v	   */
Cell v; {
    Int  low = daCount;
    Int  dfn = daCount; 		/* depth first search no. of v	   */
    List ws  = DEPENDS(v);		/* adjacency list for v		   */

    SETDEPENDS(v,mkInt(daCount++));     /* push v onto stack		   */
    push(v);

    while (nonNull(ws)) {		/* scan adjacency list for v	   */
	Cell w = hd(ws);
	ws     = tl(ws);
	low    = sccMin(low, (visited(w) ? intOf(DEPENDS(w)) : LOWLINK(w)));
    }

    if (low == dfn) {			/* start a new scc?		   */
	List temp=NIL;
	do {				/* take elements from stack	   */
	    SETDEPENDS(top(),mkInt(0));
	    temp = cons(top(),temp);
	} while (pop()!=v);
	daSccs = cons(temp,daSccs);	/* make new strongly connected comp*/
    }

    return low;
}

#ifdef SCC
static List local SCC(bs)		/* sort list with added dependency */
List bs; {				/* info into SCCs		   */
    clearStack();
    daSccs = NIL;			/* clear current list of SCCs	   */

    for (daCount=1; nonNull(bs); bs=tl(bs))	 /* visit each binding	   */
	if (!visited(hd(bs)))
	    LOWLINK(hd(bs));

    return rev(daSccs);			/* reverse to obtain correct order */
}
#endif

#ifdef SCC2				/* Two argument version		   */
static List local SCC2(bs,cs)		/* sort lists with added dependency*/
List bs, cs; {				/* info into SCCs		   */
    clearStack();
    daSccs = NIL;			/* clear current list of SCCs	   */

    for (daCount=1; nonNull(bs); bs=tl(bs))	 /* visit each binding	   */
	if (!visited(hd(bs)))
	    LOWLINK(hd(bs));
    for (; nonNull(cs); cs=tl(cs))
	if (!visited(hd(cs)))
	    LOWLINK(hd(cs));

    return rev(daSccs);			/* reverse to obtain correct order */
}
#endif

/*-------------------------------------------------------------------------*/
#undef   SETDEPENDS
#undef	 DEPENDS
#undef 	 LOWLINK
#undef	 SCC

static Void local addEvidParams(qs,v)  /* overwrite VARID/OPCELL v with	   */
List qs;			       /* application of variable to evid. */
Cell v; {			       /* parameters given by qs	   */
    if (nonNull(qs)) {
	Cell nv;

	if (!isVar(v))
	    internal("addEvidParams");

	for (nv=mkVar(textOf(v)); nonNull(tl(qs)); qs=tl(qs))
	    nv = ap(nv,thd3(hd(qs)));
	fst(v) = nv;
	snd(v) = thd3(hd(qs));
    }
}

/* --------------------------------------------------------------------------
 * Type check bodies of class and instance declarations:
 * ------------------------------------------------------------------------*/

static Void local typeClassDefn(c)	/* Type check implementations of   */
Class c; {				/* defaults for class c		   */

    /* ----------------------------------------------------------------------
     * Generate code for default dictionary builder functions:
     * --------------------------------------------------------------------*/

    Int  beta   = newKindedVars(cclass(c).kinds);
    Cell d      = inventDictVar();
    List dparam = singleton(triple(cclass(c).head,mkInt(beta),d));
    List mems   = cclass(c).members;
    List defs   = cclass(c).defaults;
    List dsels  = cclass(c).dsels;
    Cell pat    = cclass(c).dcon;
    Int  width  = cclass(c).numSupers + cclass(c).numMembers;
    Int  i      = 0;

    if (isNull(defs) && nonNull(mems)) {
        defs = cclass(c).defaults = cons(NIL,NIL);
    }

    for (; nonNull(mems); mems=tl(mems)) {
	Name   n;
	Text member           = name(hd(mems)).text;
	n = newName(inventText(),c);

	if (isNull(hd(defs))) {		/* No default definition	   */
	    name(n).line  = cclass(c).line;
	    name(n).arity = 1;
	    name(n).defn  = singleton(pair(singleton(d),
					   ap(mkInt(cclass(c).line),
					      ap(namePrimThrow,
					         ap(nameNoMethodError,
						    mkStr(member))))));
	} else {			/* User supplied default defn	   */
	    List alts = snd(hd(defs));
	    Int  line = rhsLine(snd(hd(alts)));

	    typeMember("default member binding",
		       hd(mems),
		       alts,
		       dparam,
		       cclass(c).head,
		       beta);

	    name(n).line  = line;
	    name(n).arity = 1+length(fst(hd(alts)));
	    name(n).defn  = alts;

	    for (; nonNull(alts); alts=tl(alts)) {
		fst(hd(alts)) = cons(d,fst(hd(alts)));
	    }
	}

        hd(defs) = n;
	genDefns = cons(n,genDefns);
	if (isNull(tl(defs)) && nonNull(tl(mems))) {
	    tl(defs) = cons(NIL,NIL);
	}
	defs     = tl(defs);
    }

    /* ----------------------------------------------------------------------
     * Generate code for superclass and member function selectors:
     * --------------------------------------------------------------------*/

    for (i=0; i<width; i++) {
	pat = ap(pat,inventVar());
    }
    pat = singleton(pat);
    for (i=0; nonNull(dsels); dsels=tl(dsels)) {
	name(hd(dsels)).defn = singleton(pair(pat,
					      ap(mkInt(cclass(c).line),
						 nthArg(i++,hd(pat)))));
	genDefns	     = cons(hd(dsels),genDefns);
    }
    for (mems=cclass(c).members; nonNull(mems); mems=tl(mems)) {
	name(hd(mems)).defn  = singleton(pair(pat,
					      ap(mkInt(name(hd(mems)).line),
						 nthArg(i++,hd(pat)))));
	genDefns	     = cons(hd(mems),genDefns);
    }
}

static Void local typeInstDefn(in)	/* Type check implementations of   */
Inst in; {				/* member functions for instance in*/

    /* ----------------------------------------------------------------------
     * Generate code for instance specific dictionary builder function:
     *
     *   inst.maker d1 ... dn = let sc1    = ...
     *					   .
     *					   .
     *					   .
     *				    scm    = ...
     *				    vj ... = ...
     *				    d      = Make.C sc1 ... scm v1 ... vk
     *				in d
     *
     * where sci are superclass dictionaries, d is a new name, vj
     * is a newly generated name corresponding to the implementation of a
     * member function.  (Additional line number values must be added at
     * appropriate places but, for clarity, these are not shown above.)
     * If no implementation of a particular vj is available, then we use
     * the default implementation, partially applied to d.
     * --------------------------------------------------------------------*/

    Int  alpha   = newKindedVars(cclass(inst(in).c).kinds);
    List supers  = makePredAss(cclass(inst(in).c).supers,alpha);
    Int  beta    = newKindedVars(inst(in).kinds);
    List params  = makePredAss(inst(in).specifics,beta);
    Cell d       = inventDictVar();
    /* A previous comment indicated that evids may be wrong
       in some cases when you have overlapping instances.
       (the fix was to add self to the list of evidence)
       I can't reconstruct what I meant by that comment,
       and adding self can cause definitions that are unintentionally
       self-recursive. */
    List evids   = dupList(params);

    List imps    = inst(in).implements;
    Cell l	 = mkInt(inst(in).line);
    Cell dictDef = cclass(inst(in).c).dcon;
    List mems    = cclass(inst(in).c).members;
    List defs    = cclass(inst(in).c).defaults;
    List args	 = NIL;
    List locs	 = NIL;
    List ps;

    if (!unifyPred(cclass(inst(in).c).head,alpha,inst(in).head,beta))
	internal("typeInstDefn");

    for (ps=params; nonNull(ps); ps=tl(ps))	/* Build arglist	   */
	args = cons(thd3(hd(ps)),args);
    args = rev(args);

    for (ps=supers; nonNull(ps); ps=tl(ps)) {	/* Superclass dictionaries */
	Cell pi = hd(ps);
	Cell ev = NIL;
	if (showInstRes) {
	    fputs("scEntail: ", stdout);
	    printContext(stdout,copyPreds(params));
	    fputs(" ||- ", stdout);
	    printPred(stdout, copyPred(fst3(pi),intOf(snd3(pi))));
	    fputc('\n', stdout);
	}
	ev = scEntail(params,fst3(pi),intOf(snd3(pi)),0);
	if (isNull(ev)) {
	    if (showInstRes) {
		fputs("inEntail: ", stdout);
		printContext(stdout,copyPreds(evids));
		fputs(" ||- ", stdout);
		printPred(stdout, copyPred(fst3(pi),intOf(snd3(pi))));
		fputc('\n', stdout);
	    }
	    ev = inEntail(evids,fst3(pi),intOf(snd3(pi)),0);
	}
	if (isNull(ev)) {
	    clearMarks();
	    ERRMSG(inst(in).line) "Cannot build superclass instance" ETHEN
	    ERRTEXT "\n*** Instance            : " ETHEN
		ERRPRED(copyPred(inst(in).head,beta));
	    ERRTEXT "\n*** Context supplied    : " ETHEN
		ERRCONTEXT(copyPreds(params));
	    ERRTEXT "\n*** Required superclass : " ETHEN
		ERRPRED(copyPred(fst3(pi),intOf(snd3(pi))));
	    ERRTEXT "\n"
	    EEND;
	}
	locs	= cons(pair(thd3(pi),singleton(pair(NIL,ap(l,ev)))),locs);
	dictDef = ap(dictDef,thd3(pi));
    }

    for (; nonNull(defs); defs=tl(defs)) {
	Cell imp = NIL;
	if (nonNull(imps)) {
	    imp  = hd(imps);
	    imps = tl(imps);
	}
	if (isNull(imp)) {
	    dictDef = ap(dictDef,ap(hd(defs),d));
	} else {
	    Cell v  = inventVar();
	    dictDef = ap(dictDef,v);
	    typeMember("instance member binding",
		       hd(mems),
		       snd(imp),
		       evids,
		       inst(in).head,
		       beta);
	    locs     = cons(pair(v,snd(imp)),locs);
	}
	mems = tl(mems);
    }
    locs = cons(pair(d,singleton(pair(NIL,ap(l,dictDef)))),locs);

    name(inst(in).builder).defn			/* Register builder imp	   */
	     = singleton(pair(args,ap(LETREC,pair(singleton(locs),
						  ap(l,d)))));
    genDefns = cons(inst(in).builder,genDefns);
}

static Void local typeMember(wh,mem,alts,evids,head,beta)
String wh;				/* Type check alternatives alts of */
Name   mem;				/* member mem for inst type head   */
Cell   alts;				/* at offset beta using predicate  */
List   evids;				/* assignment evids		   */
Cell   head;
Int    beta; {
    Int  line = rhsLine(snd(hd(alts)));
    Type t;
    Int  o;
    Int  m;
    List ps;
    List qs;
    Type rt;

if (DEBUG_TYPES) {
    Printf("Type check member: ");
    printExp(stdout,mem);
    Printf(" :: ");
    printType(stdout,name(mem).type);
    Printf("\nfor the instance: ");
    printPred(stdout,head);
    Printf("\n");
}

    instantiate(name(mem).type);	/* Find required type		   */
    o  = typeOff;
    m  = typeFree;
    t  = dropRank2(typeIs,o,m);
    ps = makePredAss(predsAre,o);
    if (!unifyPred(hd(predsAre),typeOff,head,beta))
	internal("typeMember1");
    clearMarks();
    qs = copyPreds(ps);
    rt = generalize(qs,liftRank2(t,o,m));

if (DEBUG_TYPES) {
    Printf("Required type is: ");
    printType(stdout,rt);
    Printf("\n");
}

    hd(defnBounds) = NIL;		/* Type check each alternative	   */
    hd(depends)    = NODEPENDS;
    enterPendingBtyvs();
    for (preds=NIL; nonNull(alts); alts=tl(alts)) {
	typeAlt(wh,mem,hd(alts),t,o,m);
	qualify(tl(ps),hd(alts));	/* Add any extra dict params	   */
    }
    improve(line,evids,preds);
    leavePendingBtyvs();

    evids = appendOnto(dupList(tl(ps)),	/* Build full complement of dicts  */
		       evids);
    clearMarks();
    normPreds(line);
    qs = elimPredsUsing(evids,NIL);
    if (nonNull(preds) && resolveDefs(genvarType(t,o,NIL),FALSE))
	qs = elimPredsUsing(evids,qs);
    if (nonNull(qs)) {
	ERRMSG(line)
		"Implementation of %s requires extra context",
		 textToStr(name(mem).text) ETHEN
	ERRTEXT "\n*** Expected type   : " ETHEN ERRTYPE(rt);
	ERRTEXT "\n*** Missing context : " ETHEN ERRCONTEXT(copyPreds(qs));
	ERRTEXT "\n"
	EEND;
    }

    resetGenerics();			/* Make sure we're general enough  */
    ps = copyPreds(ps);
    t  = generalize(ps,liftRank2(t,o,m));
if (DEBUG_TYPES) {
    Printf("Inferred type is: ");
    printType(stdout,t);
    Printf("\n");
}
    if (!sameSchemes(t,rt))
	tooGeneral(line,mem,rt,t);
    if (nonNull(preds)) {
	preds = scSimplify(preds);
	cantEstablish(line,wh,mem,t,ps);
    }
}

/* --------------------------------------------------------------------------
 * Type check bodies of bindings:
 * ------------------------------------------------------------------------*/

static Void local typeBind(b)	       /* Type check binding		   */
Cell b; {
    if (isVar(fst(b))) {			       /* function binding */
	Cell ass = findTopBinding(fst(b));
	Int  beta;

	if (isNull(ass))
	    internal("typeBind");

	beta = intOf(defType(snd(ass)));
	enterPendingBtyvs();
	map2Proc(typeDefAlt,beta,fst(b),snd(snd(b)));
	leavePendingBtyvs();
    }
    else {					       /* pattern binding  */
	static String lhsPat = "lhs pattern";
	static String rhs    = "right hand side";
	Int  beta	     = newTyvars(1);
	Pair pb		     = snd(snd(b));
	Int  l		     = rhsLine(snd(pb));

	tcMode  = OLD_PATTERN;
	enterPendingBtyvs();
	fst(pb) = patBtyvs(fst(pb));
	check(l,fst(pb),NIL,lhsPat,aVar,beta);
	tcMode  = EXPRESSION;
	snd(pb) = typeRhs(snd(pb));
	shouldBe(l,rhsExpr(snd(pb)),NIL,rhs,aVar,beta);
	doneBtyvs(l);
	leavePendingBtyvs();
    }
}

static Void local typeDefAlt(beta,v,a) /* type check alt in func. binding  */
Int  beta;
Cell v;
Pair a; {
    static String valDef = "function binding";
    typeAlt(valDef,v,a,aVar,beta,0);
}

static Cell local typeRhs(e)	       /* check type of rhs of definition  */
Cell e; {
    switch (whatIs(e)) {
	case GUARDED : {   Int beta = newTyvars(1);
			   map1Proc(guardedType,beta,snd(e));
			   tyvarType(beta);
		       }
		       break;

	case LETREC  : enterBindings();
		       enterSkolVars();
		       mapProc(typeBindings,fst(snd(e)));
		       snd(snd(e)) = typeRhs(snd(snd(e)));
		       leaveBindings();
		       leaveSkolVars(rhsLine(snd(snd(e))),typeIs,typeOff,0);
		       break;

	case RSIGN   : fst(snd(e)) = typeRhs(fst(snd(e)));
		       shouldBe(rhsLine(fst(snd(e))),
				rhsExpr(fst(snd(e))),NIL,
				"result type",
				snd(snd(e)),0);
		       return fst(snd(e));

	default      : snd(e) = typeExpr(intOf(fst(e)),snd(e));
		       break;
    }
    return e;
}

static Void local guardedType(beta,gded)/* check type of guard (li,(gd,ex))*/
Int  beta;			       /* should have gd :: Bool,	   */
Cell gded; {			       /*	      ex :: (var,beta)	   */
    static String guarded = "guarded expression";
    static String guard   = "guard";
    Int line = intOf(fst(gded));
    List svPreds;

    gded     = snd(gded);
    spCheck(line,fst(gded),NIL,guard,typeBool,0);
    spCheck(line,snd(gded),NIL,guarded,aVar,beta);
}

Cell rhsExpr(rhs)		       /* find first expression on a rhs   */
Cell rhs; {
    STACK_CHECK
    switch (whatIs(rhs)) {
	case GUARDED : return snd(snd(hd(snd(rhs))));
	case LETREC  : return rhsExpr(snd(snd(rhs)));
	case RSIGN   : return rhsExpr(fst(snd(rhs)));
	default      : return snd(rhs);
    }
}

Int rhsLine(rhs)		       /* find line number associated with */
Cell rhs; {			       /* a right hand side		   */
    STACK_CHECK
    switch (whatIs(rhs)) {
	case GUARDED : return intOf(fst(hd(snd(rhs))));
	case LETREC  : return rhsLine(snd(snd(rhs)));
	case RSIGN   : return rhsLine(fst(snd(rhs)));
	default      : return intOf(fst(rhs));
    }
}

/* --------------------------------------------------------------------------
 * Calculate generalization of types and compare with declared type schemes:
 * ------------------------------------------------------------------------*/

static Void local genBind(ps,b)		/* Generalize the type of each var */
List ps;				/* defined in binding b, qualifying*/
Cell b; {				/* each with the predicates in ps. */
    Cell v = fst(b);
    Cell t = fst(snd(b));

    if (isVar(fst(b)))
	genAss(rhsLine(snd(hd(snd(snd(b))))),ps,v,t);
    else {
	Int line = rhsLine(snd(snd(snd(b))));
	for (; nonNull(v); v=tl(v)) {
	    Type ty = NIL;
	    if (nonNull(t)) {
		ty = hd(t);
		t  = tl(t);
	    }
	    genAss(line,ps,hd(v),ty);
	}
    }
}

static Void local genAss(l,ps,v,dt)	/* Calculate inferred type of v and*/
Int  l;					/* compare with declared type, dt, */
List ps;				/* if given & check for ambiguity. */
Cell v;
Type dt; {
    Cell ass = findTopBinding(v);

    if (isNull(ass))
	internal("genAss");

    snd(ass) = genTest(l,v,ps,dt,aVar,intOf(defType(snd(ass))));

if (DEBUG_TYPES) {
    printExp(stdout,v);
    Printf(" :: ");
    printType(stdout,snd(ass));
    Printf("\n");
}
}

static Type local genTest(l,v,ps,dt,t,o)/* Generalize and test inferred	   */
Int  l;					/* type (t,o) with context ps	   */
Cell v;					/* against declared type dt for v. */
List ps;
Type dt;
Type t;
Int  o; {
    Type bt = NIL;			/* Body of inferred type	   */
    Type it = NIL;			/* Full inferred type		   */

    resetGenerics();			/* Calculate Haskell typing	   */
    ps = copyPreds(ps);
    bt = copyType(t,o);
    it = generalize(ps,bt);

    if (nonNull(dt)) {			/* If a declared type was given,   */
	instantiate(dt);		/* check body for match.	   */
	if (!equalTypes(typeIs,bt))
	    tooGeneral(l,v,dt,it);
    }
    else if (nonNull(ps))		/* Otherwise test for ambiguity in */
	if (isAmbiguous(it))		/* inferred type.		   */
	    ambigError(l,"inferred type",v,it);

    return it;
}

static Type local generalize(qs,t)	/* calculate generalization of t   */
List qs;				/* having already marked fixed vars*/
Type t; {				/* with qualifying preds qs	   */
    if (nonNull(qs))
	t = ap(QUAL,pair(qs,t));
    if (nonNull(genericVars)) {
	Kind k  = STAR;
	List vs = genericVars;
	for (; nonNull(vs); vs=tl(vs)) {
	    Tyvar *tyv = tyvar(intOf(hd(vs)));
	    Kind   ka  = tyv->kind;
	    k = ap(ka,k);
	}
	t = mkPolyType(k,t);
if (DEBUG_KINDS) {
	Printf("Generalized type: ");
	printType(stdout,t);
	Printf(" ::: ");
	printKind(stdout,k);
	Printf("\n");
}
    }
    return t;
}

static Bool local equalTypes(t1,t2)    /* Compare simple types for equality*/
Type t1, t2; {
    STACK_CHECK
et: if (whatIs(t1)!=whatIs(t2))
	return FALSE;

    switch (whatIs(t1)) {
	case EXT     :
	case TYCON   :
	case OFFSET  :
	case TUPLE   : return t1==t2;

	case INTCELL : return intOf(t1)!=intOf(t2);

	case AP      : if (equalTypes(fun(t1),fun(t2))) {
			   t1 = arg(t1);
			   t2 = arg(t2);
			   goto et;
		       }
		       return FALSE;

	default      : internal("equalTypes");
    }

    return TRUE;/*NOTREACHED*/
}

/* --------------------------------------------------------------------------
 * Entry points to type checker:
 * ------------------------------------------------------------------------*/

Type typeCheckExp(useDefs)		/* Type check top level expression */
Bool useDefs; {				/* using defaults if reqd	   */
    Type type;
    List ctxt;
    Int  beta;

    typeChecker(RESET);
    emptySubstitution();
    enterBindings();
    inputExpr = typeExpr(0,inputExpr);
    type      = typeIs;
    beta      = typeOff;
    clearMarks();
    improve(0,NIL,preds);
    normPreds(0);
    elimTauts();
    preds     = scSimplify(preds);
    if (useDefs && nonNull(preds)) {
	clearMarks();
	reducePreds();
	if (nonNull(preds) && resolveDefs(NIL,TRUE)) /* Nearly Haskell 1.4? */
	    elimTauts();
    }
    resetGenerics();
    ctxt      = copyPreds(preds);
    type      = generalize(ctxt,copyType(type,beta));
    inputExpr = qualifyExpr(0,preds,inputExpr);
    h98CheckInferredType(0,inputExpr,type);
    typeChecker(RESET);
    emptySubstitution();
    return type;
}

Void typeCheckDefns() { 	       /* Type check top level bindings    */
    Target t  = length(selDefns)  + length(valDefns) +
		length(instDefns) + length(classDefns);
    Target i  = 0;
    List   gs;

    typeChecker(RESET);
    emptySubstitution();
    enterSkolVars();
    enterBindings();
    setGoal("Type checking",t);

    for (gs=selDefns; nonNull(gs); gs=tl(gs)) {
	mapOver(typeSel,hd(gs));
	soFar(i++);
    }
    for (gs=valDefns; nonNull(gs); gs=tl(gs)) {
	typeDefnGroup(hd(gs));
	soFar(i++);
    }
    mapProc(typeForeignExport,foreignExports); /* ToDo: soFar magic */
    clearTypeIns();
    for (gs=classDefns; nonNull(gs); gs=tl(gs)) {
	emptySubstitution();
	typeClassDefn(hd(gs));
	soFar(i++);
    }
    for (gs=instDefns; nonNull(gs); gs=tl(gs)) {
	emptySubstitution();
	typeInstDefn(hd(gs));
	soFar(i++);
    }

    typeChecker(RESET);
    emptySubstitution();
    done();
}

static Void local typeDefnGroup(bs)	/* type check group of value defns */
List bs; {				/* (one top level scc)		   */
    List as;

    emptySubstitution();
    hd(defnBounds) = NIL;
    preds	   = NIL;
    setTypeIns(bs);
    typeBindings(bs);			/* find types for vars in bindings */

    if (nonNull(preds)) {
	Cell v = fst(hd(hd(varsBounds)));
	Name n = findName(textOf(v));
	Int  l = nonNull(n) ? name(n).line : 0;
	preds  = scSimplify(preds);
	ERRMSG(l) "Instance%s of ", (length(preds)==1 ? "" : "s") ETHEN
	ERRCONTEXT(copyPreds(preds));
	ERRTEXT	  " required for definition of " ETHEN
	ERREXPR(nonNull(n)?n:v);
	ERRTEXT   "\n"
	EEND;
    }

    if (nonNull(hd(skolVars))) {
	Cell b = hd(bs);
	Name n = findName(isVar(fst(b)) ? textOf(fst(b)) : textOf(hd(fst(b))));
	Int  l = nonNull(n) ? name(n).line : 0;
	leaveSkolVars(l,typeUnit,0,0);
	enterSkolVars();
    }

    for (as=hd(varsBounds); nonNull(as); as=tl(as)) {
	Cell a = hd(as);		/* add infered types to environment*/
	Name n = findName(textOf(fst(a)));
	if (isNull(n))
	    internal("typeDefnGroup");
	name(n).type = snd(a);
    }
    hd(varsBounds) = NIL;
}

static Void local typeForeignExport(n)	/* Typecheck a foreign export decl */
Name n; {				
    Int line = name(n).line;
    /* todo */
    ERRMSG(line) "Foreign export not implemented yet."
    EEND;
}

static Pair local typeSel(s)		/* Calculate a suitable type for a */
Name s; {				/* particular selector, s.	   */
    List cns  = name(s).defn;
    Int  line = name(s).line;
    Type dom  = NIL;			/* Inferred domain		   */
    Type rng  = NIL;			/* Inferred range		   */
    Cell nv   = inventVar();
    List alts = NIL;
    Int  o;
    Int  m;

if (DEBUG_SELS) {
    Printf("Selector %s, cns=",textToStr(name(s).text));
    printExp(stdout,cns);
    Putchar('\n');
}

    emptySubstitution();
    preds = NIL;

    for (; nonNull(cns); cns=tl(cns)) {
	Name c   = fst(hd(cns));
	Int  n   = intOf(snd(hd(cns)));
	Int  a   = name(c).arity;
	Cell pat = c;
	Type dom1;
	Type rng1;
	Int  o1;
	Int  m1;

	instantiate(name(c).type);	/* Instantiate constructor type	   */
	o1 = typeOff;
	m1 = typeFree;
	for (; nonNull(predsAre); predsAre=tl(predsAre))
	    assumeEvid(hd(predsAre),o1);

	if (whatIs(typeIs)==RANK2)	/* Skip rank2 annotation, if any   */
	    typeIs = snd(snd(typeIs));
	for (; --n>0; a--) {		/* Get range			   */
	    pat    = ap(pat,WILDCARD);
	    typeIs = arg(typeIs);
	}
	rng1   = dropRank1(arg(fun(typeIs)),o1,m1);
	pat    = ap(pat,nv);
	typeIs = arg(typeIs);
	while (--a>0) {			/* And then look for domain	   */
	    pat    = ap(pat,WILDCARD);
	    typeIs = arg(typeIs);
	}
	dom1   = typeIs;

	if (isNull(dom)) {		/* Save first domain type and then */
	    dom = dom1;			/* unify with subsequent domains to*/
	    o   = o1;			/* match up preds and range types  */
	    m   = m1;
	}
	else if (!unify(dom1,o1,dom,o))
	    internal("typeSel1");

	if (isNull(rng))		/* Compare component types	   */
	    rng = rng1;
	else if (!sameSchemes(rng1,rng)) {
	    clearMarks();
	    rng  = liftRank1(rng,o,m);
	    rng1 = liftRank1(rng1,o1,m1);
	    ERRMSG(name(s).line) "Mismatch in field types for selector \"%s\"",
				 textToStr(name(s).text) ETHEN
	    ERRTEXT "\n*** Field type     : " 		 ETHEN ERRTYPE(rng1);
	    ERRTEXT "\n*** Does not match : " 		 ETHEN ERRTYPE(rng);
	    ERRTEXT "\n"
	    EEND;
	}
	alts = cons(pair(singleton(pat),pair(mkInt(line),nv)),alts);
    }
    alts = rev(alts);

    if (isNull(dom) || isNull(rng))	/* Should have been initialized by */
	internal("typeSel2");		/* now, assuming length cns >= 1.  */

    clearMarks();			/* No fixed variables here	   */
    preds = scSimplify(preds);		/* Simplify context		   */
    dom   = copyType(dom,o);		/* Calculate domain type	   */
    instantiate(rng);
    rng   = copyType(typeIs,typeOff);
    if (nonNull(predsAre)) {
	List ps    = makePredAss(predsAre,typeOff);
	List alts1 = alts;
	for (; nonNull(alts1); alts1=tl(alts1)) {
	    Cell body = nv;
	    List qs   = ps;
	    for (; nonNull(qs); qs=tl(qs))
		body = ap(body,thd3(hd(qs)));
	    snd(snd(hd(alts1))) = body;
	}
	preds = appendOnto(preds,ps);
    }
    name(s).type  = generalize(copyPreds(preds),fn(dom,rng));
    name(s).arity = 1 + length(preds);
    map1Proc(qualify,preds,alts);

if (DEBUG_SELS) {
    Printf("Inferred arity = %d, type = ",name(s).arity);
    printType(stdout,name(s).type);
    Putchar('\n');
}

    return pair(s,alts);
}

/* --------------------------------------------------------------------------
 * Type checker control:
 * ------------------------------------------------------------------------*/

Void typeChecker(what)
Int what; {
    switch (what) {
	case RESET   : tcMode	    = EXPRESSION;
		       daSccs	    = NIL;
		       preds	    = NIL;
		       pendingBtyvs = NIL;
		       emptyAssumption();
		       break;

	case MARK    : mark(defnBounds);
		       mark(varsBounds);
		       mark(depends);
		       mark(pendingBtyvs);
		       mark(skolVars);
		       mark(localEvs);
		       mark(savedPs);
		       mark(dummyVar);
		       mark(daSccs);
		       mark(preds);
		       mark(stdDefaults);
		       mark(arrow);
		       mark(boundPair);
		       mark(listof);
		       mark(typeVarToVar);
		       mark(predNum);
		       mark(predFractional);
		       mark(predIntegral);
		       mark(starToStar);
		       mark(predMonad);
		       mark(predMonadRec);

		       mark(typeProgIO);
		       break;

	case INSTALL : typeChecker(RESET);

		       dummyVar     = inventVar();

		       modulePrelude = newModule(textPrelude);
		       moduleUserPrelude = 0;
		       setCurrModule(modulePrelude);

		       starToStar   = simpleKind(1);

		       typeUnit     = addPrimTycon(findText("()"),
						   STAR,0,DATATYPE,NIL);
		       typeArrow    = addPrimTycon(findText("(->)"),
						   simpleKind(2),2,
						   DATATYPE,NIL);
		       typeList	    = addPrimTycon(findText("[]"),
						   starToStar,1,
						   DATATYPE,NIL);

		       arrow	    = fn(aVar,bVar);
		       listof	    = ap(typeList,aVar);
		       boundPair    = ap(ap(mkTuple(2),aVar),aVar);

		       nameUnit	    = addPrimCfun(findText("()"),0,0,typeUnit);
		       tycon(typeUnit).defn
				    = singleton(nameUnit);

		       nameNil	    = addPrimCfun(findText("[]"),0,1,
						   mkPolyType(starToStar,
							      listof));
		       nameCons     = addPrimCfun(findText(":"),2,2,
						   mkPolyType(starToStar,
							      fn(aVar,
							      fn(listof,
								 listof))));
		       name(nameCons).syntax
				    = mkSyntax(RIGHT_ASS,5);

		       tycon(typeList).defn
				    = cons(nameNil,cons(nameCons,NIL));

		       typeVarToVar = fn(aVar,aVar);
		       typeNoRow    = addPrimTycon(findText("EmptyRow"),
						   ROW,0,DATATYPE,NIL);
		       typeRec	    = addPrimTycon(findText("Rec"),
						   pair(ROW,STAR),1,
						   DATATYPE,NIL);
		       nameNoRec    = addPrimCfun(findText("EmptyRec"),0,0,
							ap(typeRec,typeNoRow));

		       break;
    }
}

static Name local linkName(s)
String s; {
    Name n = findName(findText(s));
    if (isNull(n)) {
        ERRMSG(0) "Prelude does not define standard name \"%s\"", s
        EEND;
    }
    return n;
}

static Tycon local linkTycon(s)
String s; {
    Tycon tc = findTycon(findText(s));
    if (isNull(tc)) {
        ERRMSG(0) "Prelude does not define standard type \"%s\"", s
        EEND;
    }
    return tc;
}

static Class local linkClass(s)
String s; {
    Class cc = findClass(findText(s));
    if (isNull(cc)) {
        ERRMSG(0) "Prelude does not define standard class \"%s\"", s
        EEND;
    }
    return cc;
}

Void linkPreludeTC() {			/* Hook to tycons and classes in   */
    if (isNull(typeBool)) {		/* prelude when first loaded	   */
	Int i;

	typeInt      = linkTycon("Int");
	typeInt8     = linkTycon("Int8");
	typeInt16    = linkTycon("Int16");
	typeInt32    = linkTycon("Int32");
	typeInt64    = linkTycon("Int64");
	typeWord     = linkTycon("Word");       /* deprecated */
	typeWord8    = linkTycon("Word8");
	typeWord16   = linkTycon("Word16");
	typeWord32   = linkTycon("Word32");
	typeWord64   = linkTycon("Word64");
	typeFunPtr   = linkTycon("FunPtr");
	typePtr      = linkTycon("Ptr");
	typeAddr     = linkTycon("Addr");       /* deprecated */
	typeFloat    = linkTycon("Float");
	typeDouble   = linkTycon("Double");
	typeChar     = linkTycon("Char");
	typeForeignP = linkTycon("ForeignPtr");
	typeForeign  = linkTycon("ForeignObj"); /* deprecated */
	typeStable   = linkTycon("StablePtr");
	typeBool     = linkTycon("Bool");
	typeString   = linkTycon("String");
	typeInteger  = linkTycon("Integer");
	typeMaybe    = linkTycon("Maybe");
	typeOrdering = linkTycon("Ordering");

	stdDefaults  = cons(typeInteger,cons(typeDouble,NIL));

	classEq      = linkClass("Eq");
	classOrd     = linkClass("Ord");
	classIx      = linkClass("Ix");
	classEnum    = linkClass("Enum");
	classShow    = linkClass("Show");
	classRead    = linkClass("Read");
	classBounded = linkClass("Bounded");

	classReal       = linkClass("Real");
	classIntegral   = linkClass("Integral");
	classRealFrac   = linkClass("RealFrac");
	classRealFloat  = linkClass("RealFloat");
	classFractional = linkClass("Fractional");
	classFloating   = linkClass("Floating");
	classNum        = linkClass("Num");
	predNum	        = ap(classNum,aVar);
	predFractional  = ap(classFractional,aVar);
	predIntegral    = ap(classIntegral,aVar);

	classMonad      = linkClass("Monad");
	predMonad       = ap(classMonad,aVar);

	typeIO          = linkTycon("IO");
	typeProgIO      = ap(typeIO,aVar);

	/* The following primitives are referred to in derived instances and
	 * hence require types; the following types are a little more general
	 * than we might like, but they are the closest we can get without a
	 * special datatype class.
	 */
	name(nameConCmp).type
	    = mkPolyType(starToStar,fn(aVar,fn(aVar,typeOrdering)));
	name(nameEnRange).type
	    = mkPolyType(starToStar,fn(boundPair,listof));
	name(nameEnIndex).type
	    = mkPolyType(starToStar,fn(boundPair,fn(aVar,typeInt)));
	name(nameEnInRng).type
	    = mkPolyType(starToStar,fn(boundPair,fn(aVar,typeBool)));
	name(nameEnToEn).type
	    = mkPolyType(starToStar,fn(aVar,fn(typeInt,aVar)));
	name(nameEnFrEn).type
	    = mkPolyType(starToStar,fn(aVar,typeInt));
	name(nameEnFrom).type
	    = mkPolyType(starToStar,fn(aVar,listof));
	name(nameEnFrTo).type
	    = name(nameEnFrTh).type
	    = mkPolyType(starToStar,fn(aVar,fn(aVar,listof)));

	for (i=2; i<=NUM_DTUPLES; i++) {/* Add derived instances of tuples */
	    addTupInst(classEq,i);
	    addTupInst(classOrd,i);
	    addTupInst(classShow,i);
	    addTupInst(classRead,i);
	    addTupInst(classIx,i);
	}
    }
}

Void linkPreludeCM() {			/* Hook to cfuns and mfuns in	   */
    if (isNull(nameFalse)) {		/* prelude when first loaded	   */
	nameFalse       = linkName("False");
	nameTrue        = linkName("True");
	nameJust        = linkName("Just");
	nameNothing     = linkName("Nothing");
	nameLeft        = linkName("Left");
	nameRight       = linkName("Right");
	nameLT	        = linkName("LT");
	nameEQ	        = linkName("EQ");
	nameGT	        = linkName("GT");

	nameFromInt     = linkName("fromInt");
	nameFromInteger = linkName("fromInteger");
	nameFromDouble  = linkName("fromDouble");
	nameEq	        = linkName("==");
	nameCompare     = linkName("compare");
	nameLe	        = linkName("<=");
	nameGt	        = linkName(">");
	nameShowsPrec   = linkName("showsPrec");
	nameReadsPrec   = linkName("readsPrec");
	nameIndex       = linkName("index");
	nameInRange     = linkName("inRange");
	nameRange       = linkName("range");
	nameMult        = linkName("*");
	namePlus        = linkName("+");
	nameMinBnd	= linkName("minBound");
	nameMaxBnd	= linkName("maxBound");
	nameReturn      = linkName("return");
	nameBind        = linkName(">>=");
	nameThen        = linkName(">>");
	nameMFail       = linkName("fail");

        /* The constructor names better match up with the defn
	   of IOErrorType in Prelude. 
	*/
	nameIOError        = linkName("IOError");
	nameAlreadyExists  = linkName("AlreadyExists");
	nameDoesNotExist   = linkName("NoSuchThing");
	nameAlreadyInUse   = linkName("ResourceBusy");
	nameIsFull         = linkName("ResourceExhausted");
	nameEOFErr         = linkName("EOF");
	nameProtocolError  = linkName("ProtocolError");
	nameIllegal        = linkName("IllegalOperation");
	namePermDenied     = linkName("PermissionDenied");
	nameUserErr        = linkName("UserError");

	nameArithException = linkName("ArithException");
	nameArrayException = linkName("ArrayException");
	nameErrorCall      = linkName("ErrorCall");
	nameIOException    = linkName("IOException");
	nameNoMethodError  = linkName("NoMethodError");
	nameNonTermination = linkName("NonTermination");
	namePatternMatchFail = linkName("PatternMatchFail");
	nameRecConError    = linkName("RecConError");
	nameRecSelError    = linkName("RecSelError");
	nameRecUpdError    = linkName("RecUpdError");

	nameOverflow       = linkName("Overflow");
	nameDivideByZero   = linkName("DivideByZero");

	nameIndexOutOfBounds = linkName("IndexOutOfBounds");
	nameUndefinedElement = linkName("UndefinedElement");
    }
}

Void linkPreludeFuns() {		/* Hook to cfuns and mfuns in	   */
					/* prelude when first loaded	   */
}
/*-------------------------------------------------------------------------*/
