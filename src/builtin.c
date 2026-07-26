/* --------------------------------------------------------------------------
 * Primitive functions, input output etc...
 *
 * The Hugs 98 system is Copyright (c) Mark P Jones, Alastair Reid, the
 * Yale Haskell Group, and the OGI School of Science & Engineering at OHSU,
 * 1994-2003, All rights reserved.  It is distributed as free software under
 * the license in the file "License", which is included in the distribution.
 *
 * $RCSfile: builtin.c,v $
 * $Revision: 1.93 $
 * $Date: 2006/06/01 22:20:17 $
 * ------------------------------------------------------------------------*/

/* We include math.h before prelude.h because SunOS 4's cpp incorrectly
 * reports an error if you use "name(n)" as a macro with arguments and
 * "name" as a normal identifier (with no arguments).  ADR
 */
#include <math.h>
#include "prelude.h"
#include "storage.h"
#include "connect.h"
#include "errors.h"
#include "machdep.h"
#include "char.h"
#include <ctype.h>

#if HAVE_IO_H
# include <io.h>
#endif

/* Header files needed to compile the IO primitives */
#if IO_MONAD

# include <sys/types.h>
# include <sys/stat.h>

# include <unistd.h>

# include <sys/times.h>

# include <sys/time.h>

# include <sys/resource.h>

# include <errno.h>

# include <sys/timeb.h>


# include <dirent.h>

#if HAVE_DIRECT_H
# include <direct.h>
#endif

# include <fcntl.h>

#endif /* IO_MONAD */

#include "builtin.h"

Name nameNegate,  nameFlip;             /* primitives reqd for parsing     */
Name nameFrom,    nameFromThen;
Name nameFromTo,  nameFromThenTo;
Name nameFatbar,  nameFail;             /* primitives reqd for translation */
Name nameIf,      nameSel;
Name nameId,      nameOtherwise;
Name nameConCmp,  nameEnRange;          /* primitives used for deriv inst  */
Name nameEnIndex, nameEnInRng;
Name nameEnToEn,  nameEnFrEn;
Name nameEnFrom,  nameEnFrTh;
Name nameEnFrTo;
Name nameBlackHole;                     /* for GC-detected black hole      */
Name nameInd;				/* for dict indirection		   */
Name namePrint,   nameNPrint;           /* primitives for printing         */

Name nameFst,     nameSnd;              /* 2-tuple selector functions      */
Name nameAnd,     nameOr;               /* built-in logical connectives    */
Name namePrimThrow;                     /* throw primitive function        */
Name nameComp;                          /* function composition            */
Name nameApp;                           /* list append                     */
Name nameShowField;                     /* display single field            */
Name nameShowParen;                     /* wrap with parens                */
Name nameReadField;                     /* read single field               */
Name nameReadParen;                     /* unwrap from parens              */
Name nameLex;                           /* lexer                           */
Name nameRangeSize;                     /* calculate size of index range   */
Name nameCompAux;                       /* auxiliary function for compares */
Name namePmInt,   namePmFlt;            /* primitives for pattern matching */
Name nameReturnIO;
Name namePmInteger;
#if NPLUSK
Name namePmNpk,   namePmSub;            /* primitives for (n+k) patterns   */
#endif
#if TREX
Name nameRecExt,  nameRecBrk;           /* Extend and break a record       */
Name nameRecSel,  nameRecShw;           /* Select and show a record        */
Name nameRecEq;				/* Compare records		   */
Name nameAddEv;				/* Add up evidence		   */
#endif

Name nameRationalToFloat;
Name nameRationalToDouble;
#if SHORT_CIRCUIT_COERCIONS
Name nameFloatToRational;
Name nameDoubleToRational;
Name nameDoubleToRatio;
Name nameIntToRatio;
Name nameIntToFloat;
Name nameIntToDouble;
Name nameDoubleToFloat;
Name nameFloatToDouble;
#endif

/* --------------------------------------------------------------------------
 * Local function prototypes:
 * ------------------------------------------------------------------------*/

PROTO_PRIM(primFatbar);
PROTO_PRIM(primFail);
PROTO_PRIM(primCatchError);
PROTO_PRIM(primThrowException);
PROTO_PRIM(primCatchException);
PROTO_PRIM(primBlackHole);
PROTO_PRIM(primIndirect);
PROTO_PRIM(primSel);
PROTO_PRIM(primIf);
PROTO_PRIM(primStrict);
PROTO_PRIM(primSeq);
PROTO_PRIM(primConCmp);
PROTO_PRIM(primEnRange);
PROTO_PRIM(primEnIndex);
PROTO_PRIM(primEnInRng);
PROTO_PRIM(primEnFrEn);
PROTO_PRIM(primEnToEn);
PROTO_PRIM(primEnFrom);
PROTO_PRIM(primEnFrTh);
PROTO_PRIM(primEnFrTo);

PROTO_PRIM(primMinInt);
PROTO_PRIM(primMaxInt);
PROTO_PRIM(primPlusInt);
PROTO_PRIM(primMinusInt);
PROTO_PRIM(primMulInt);
PROTO_PRIM(primDivInt);
PROTO_PRIM(primQuotInt);
PROTO_PRIM(primModInt);
PROTO_PRIM(primRemInt);
PROTO_PRIM(primQrmInt);
PROTO_PRIM(primNegInt);

PROTO_PRIM(primAndInt);
PROTO_PRIM(primOrInt);
PROTO_PRIM(primXorInt);
PROTO_PRIM(primComplementInt);
PROTO_PRIM(primShiftInt);
PROTO_PRIM(primBitInt);
PROTO_PRIM(primTestInt);

PROTO_PRIM(primCharToInt);
PROTO_PRIM(primIntToChar);
PROTO_PRIM(primWordToInt);
PROTO_PRIM(primIntToWord);
PROTO_PRIM(primIntToFloat);
PROTO_PRIM(primIntToDouble);
PROTO_PRIM(primDummyCvt);

PROTO_PRIM(primRationalToFloat);
PROTO_PRIM(primRationalToDouble);

#if WORD_OPS
PROTO_PRIM(primMaxWord);
PROTO_PRIM(primPlusWord);
PROTO_PRIM(primMinusWord);
PROTO_PRIM(primNegateWord);
PROTO_PRIM(primMulWord);
PROTO_PRIM(primDivWord);
PROTO_PRIM(primQuotWord);
PROTO_PRIM(primModWord);
PROTO_PRIM(primRemWord);
PROTO_PRIM(primQrmWord);

PROTO_PRIM(primAndWord);
PROTO_PRIM(primOrWord);
PROTO_PRIM(primXorWord);
PROTO_PRIM(primComplementWord);
PROTO_PRIM(primShiftWord);
PROTO_PRIM(primRotateWord);
PROTO_PRIM(primBitWord);
PROTO_PRIM(primTestWord);

PROTO_PRIM(primItoI8);
PROTO_PRIM(primItoI16);
PROTO_PRIM(primItoI32);

PROTO_PRIM(primI8toI);
PROTO_PRIM(primI16toI);
PROTO_PRIM(primI32toI);

PROTO_PRIM(primI32toI64);
PROTO_PRIM(primI64toI32);

PROTO_PRIM(primWtoW8);
PROTO_PRIM(primWtoW16);
PROTO_PRIM(primWtoW32);

PROTO_PRIM(primW8toW);
PROTO_PRIM(primW16toW);
PROTO_PRIM(primW32toW);

PROTO_PRIM(primW32toW64);
PROTO_PRIM(primW64toW32);
#endif

PROTO_PRIM(primFreeHFunPtr);

PROTO_PRIM(primDoubleToFloat);
PROTO_PRIM(primFloatToDouble);

PROTO_PRIM(primPlusFloat);
PROTO_PRIM(primMinusFloat);
PROTO_PRIM(primMulFloat);
PROTO_PRIM(primDivFloat);
PROTO_PRIM(primNegFloat);

PROTO_PRIM(primPlusDouble);
PROTO_PRIM(primMinusDouble);
PROTO_PRIM(primMulDouble);
PROTO_PRIM(primDivDouble);
PROTO_PRIM(primNegDouble);

PROTO_PRIM(primSinFloat);
PROTO_PRIM(primCosFloat);
PROTO_PRIM(primTanFloat);
PROTO_PRIM(primAsinFloat);
PROTO_PRIM(primAcosFloat);
PROTO_PRIM(primAtanFloat);
#if 0 /* Not used in current Prelude */
PROTO_PRIM(primAtan2Float);
#endif
PROTO_PRIM(primExpFloat);
PROTO_PRIM(primLogFloat);
PROTO_PRIM(primSqrtFloat);
PROTO_PRIM(primFloatToInt);
PROTO_PRIM(primFloatRadix);
PROTO_PRIM(primFloatDigits);
PROTO_PRIM(primFloatMinExp);
PROTO_PRIM(primFloatMaxExp);
PROTO_PRIM(primFloatDecode);
PROTO_PRIM(primFloatEncode);

PROTO_PRIM(primSinDouble);
PROTO_PRIM(primCosDouble);
PROTO_PRIM(primTanDouble);
PROTO_PRIM(primAsinDouble);
PROTO_PRIM(primAcosDouble);
PROTO_PRIM(primAtanDouble);
#if 0 /* Not used in current Prelude */
PROTO_PRIM(primAtan2Double);
#endif
PROTO_PRIM(primExpDouble);
PROTO_PRIM(primLogDouble);
PROTO_PRIM(primSqrtDouble);
PROTO_PRIM(primDoubleToInt);
PROTO_PRIM(primDoubleDigits);
PROTO_PRIM(primDoubleMinExp);
PROTO_PRIM(primDoubleMaxExp);
PROTO_PRIM(primDoubleDecode);
PROTO_PRIM(primDoubleEncode);

PROTO_PRIM(primNullPtr);
PROTO_PRIM(primPlusPtr);
PROTO_PRIM(primAlignPtr);
PROTO_PRIM(primMinusPtr);
PROTO_PRIM(primEqPtr);
PROTO_PRIM(primCmpPtr);

PROTO_PRIM(primEqInt);
PROTO_PRIM(primCmpInt);

PROTO_PRIM(primEqWord);
PROTO_PRIM(primCmpWord);

PROTO_PRIM(primEqChar);
PROTO_PRIM(primCmpChar);

PROTO_PRIM(primEqFloat);
PROTO_PRIM(primCmpFloat);

PROTO_PRIM(primEqDouble);
PROTO_PRIM(primCmpDouble);

PROTO_PRIM(primMaxChar);
PROTO_PRIM(primIsUpper);
PROTO_PRIM(primIsLower);
PROTO_PRIM(primIsAlpha);
PROTO_PRIM(primIsAlphaNum);
PROTO_PRIM(primIsPrint);
PROTO_PRIM(primToUpper);
PROTO_PRIM(primToLower);
PROTO_PRIM(primToTitle);
PROTO_PRIM(primUniGenCat);

#if TREX
PROTO_PRIM(primRecExt);
PROTO_PRIM(primRecBrk);
PROTO_PRIM(primRecSel);
PROTO_PRIM(primRecShw);
PROTO_PRIM(primRecEq);
#endif

#if OBSERVATIONS
EXT_PROTO_PRIM(primObserve);
EXT_PROTO_PRIM(primBkpt);
EXT_PROTO_PRIM(primSetBkpt);
#endif

PROTO_PRIM(primUnsafePtrEq);
PROTO_PRIM(primUnsafePtrToInt);

static Cell local followInd (Cell );

/* --------------------------------------------------------------------------
 * Table of primitive/built-in values:
 * ------------------------------------------------------------------------*/

static struct primitive builtinPrimTable[] = {
  {"fatbar",            2, primFatbar},
  {"fail",              0, primFail},
  {"catchError",        1, primCatchError},
  {"primThrowException",1, primThrowException},
  {"primCatchException",1, primCatchException},
  {"gcBhole",           0, primBlackHole},
  {"dictIndirect",      1, primIndirect},
  {"sel",               3, primSel},
  {"if",                3, primIf},
  {"conCmp",            2, primConCmp},
  {"enRange",           1, primEnRange},
  {"enIndex",           2, primEnIndex},
  {"enInRng",           2, primEnInRng},
  {"enToEn",            2, primEnToEn},
  {"enFrEn",            1, primEnFrEn},
  {"enFrom",            1, primEnFrom},
  {"enFrTh",            2, primEnFrTh},
  {"enFrTo",            2, primEnFrTo},

  {"primMinInt",        0, primMinInt},
  {"primMaxInt",        0, primMaxInt},
  {"primPlusInt",       2, primPlusInt},
  {"primMinusInt",      2, primMinusInt},
  {"primMulInt",        2, primMulInt},
  {"primDivInt",        2, primDivInt},
  {"primQuotInt",       2, primQuotInt},
  {"primModInt",        2, primModInt},
  {"primRemInt",        2, primRemInt},
  {"primNegInt",        1, primNegInt},
  {"primQrmInt",        2, primQrmInt},

  {"primAndInt",        2, primAndInt},
  {"primOrInt",         2, primOrInt},
  {"primXorInt",        2, primXorInt},
  {"primComplementInt", 1, primComplementInt},
  {"primShiftInt",      2, primShiftInt},
  {"primBitInt",        1, primBitInt},
  {"primTestInt",       2, primTestInt},

#if WORD_OPS
  {"primMaxWord",       0, primMaxWord},
  {"primPlusWord",      2, primPlusWord},
  {"primMinusWord",     2, primMinusWord},
  {"primNegateWord",    1, primNegateWord},
  {"primMulWord",       2, primMulWord},
  {"primDivWord",       2, primDivWord},
  {"primQuotWord",      2, primQuotWord},
  {"primModWord",       2, primModWord},
  {"primRemWord",       2, primRemWord},
  {"primQrmWord",       2, primQrmWord},

  {"primAndWord",       2, primAndWord},
  {"primOrWord",        2, primOrWord},
  {"primXorWord",       2, primXorWord},
  {"primComplementWord",1, primComplementWord},
  {"primShiftWord",     2, primShiftWord},
  {"primRotateWord",    3, primRotateWord},
  {"primBitWord",       1, primBitWord},
  {"primTestWord",      2, primTestWord},

  {"primIntToInt8",     1, primItoI8},
  {"primIntToInt16",    1, primItoI16},
  {"primIntToInt32",    1, primItoI32},

  {"primInt8ToInt",     1, primI8toI},
  {"primInt16ToInt",    1, primI16toI},
  {"primInt32ToInt",    1, primI32toI},

  {"primInt32ToInt64",  2, primI32toI64},
  {"primInt64ToInt32",  1, primI64toI32},

  {"primWordToWord8",   1, primWtoW8},
  {"primWordToWord16",  1, primWtoW16},
  {"primWordToWord32",  1, primWtoW32},

  {"primWord8ToWord",   1, primW8toW},
  {"primWord16ToWord",  1, primW16toW},
  {"primWord32ToWord",  1, primW32toW},

  {"primWord32ToWord64",2, primW32toW64},
  {"primWord64ToWord32",1, primW64toW32},
#endif

  {"freeHaskellFunPtr", 1+IOArity, primFreeHFunPtr},

#if !BIGNUMS                            /* Implement Integer as Int        */
  {"primPlusInteger",   2, primPlusInt},
  {"primMinusInteger",  2, primMinusInt},
  {"primMulInteger",    2, primMulInt},
  {"primQrmInteger",    2, primQrmInt},
  {"primNegInteger",    1, primNegInt},
  {"primIntToInteger",  1, primDummyCvt},
  {"primIntegerToInt",  1, primDummyCvt},
  {"primIntegerToFloat",1, primIntToFloat},
  {"primIntegerToDouble",1,primIntToDouble},
  {"primEqInteger",     2, primEqInt},
  {"primCmpInteger",    2, primCmpInt},
#endif

  {"primPlusFloat",     2, primPlusFloat},
  {"primMinusFloat",    2, primMinusFloat},
  {"primMulFloat",      2, primMulFloat},
  {"primDivFloat",      2, primDivFloat},
  {"primNegFloat",      1, primNegFloat},

  {"primPlusDouble",    2, primPlusDouble},
  {"primMinusDouble",   2, primMinusDouble},
  {"primMulDouble",     2, primMulDouble},
  {"primDivDouble",     2, primDivDouble},
  {"primNegDouble",     1, primNegDouble},

  {"primSinFloat",      1, primSinFloat},
  {"primCosFloat",      1, primCosFloat},
  {"primTanFloat",      1, primTanFloat},
  {"primAsinFloat",     1, primAsinFloat},
  {"primAcosFloat",     1, primAcosFloat},
  {"primAtanFloat",     1, primAtanFloat},
  {"primExpFloat",      1, primExpFloat},
  {"primLogFloat",      1, primLogFloat},
  {"primSqrtFloat",     1, primSqrtFloat},
  {"primFloatToInt",    1, primFloatToInt},
  {"primFloatRadix",    0, primFloatRadix},
  {"primFloatDigits",   0, primFloatDigits},
  {"primFloatMinExp",   0, primFloatMinExp},
  {"primFloatMaxExp",   0, primFloatMaxExp},
  {"primFloatDecode",   1, primFloatDecode},
  {"primFloatEncode",   2, primFloatEncode},

  {"primSinDouble",     1, primSinDouble},
  {"primCosDouble",     1, primCosDouble},
  {"primTanDouble",     1, primTanDouble},
  {"primAsinDouble",    1, primAsinDouble},
  {"primAcosDouble",    1, primAcosDouble},
  {"primAtanDouble",    1, primAtanDouble},
  {"primExpDouble",     1, primExpDouble},
  {"primLogDouble",     1, primLogDouble},
  {"primSqrtDouble",    1, primSqrtDouble},
  {"primDoubleToInt",   1, primDoubleToInt},
  {"primDoubleRadix",   0, primFloatRadix},
  {"primDoubleDigits",  0, primDoubleDigits},
  {"primDoubleMinExp",  0, primDoubleMinExp},
  {"primDoubleMaxExp",  0, primDoubleMaxExp},
  {"primDoubleDecode",  1, primDoubleDecode},
  {"primDoubleEncode",  2, primDoubleEncode},

  {"primIntToChar",     1, primIntToChar},
  {"primCharToInt",     1, primCharToInt},
  {"primIntToWord",     1, primIntToWord},
  {"primWordToInt",     1, primWordToInt},
  {"primIntToFloat",    1, primIntToFloat},
  {"primIntToDouble",   1, primIntToDouble},
  {"primDoubleToFloat", 1, primDoubleToFloat},
  {"primFloatToDouble", 1, primFloatToDouble},

  {"primRationalToFloat",  1, primRationalToFloat},
  {"primRationalToDouble", 1, primRationalToDouble},

  {"nullPtr",           0, primNullPtr},
  {"plusPtr",           2, primPlusPtr},
  {"alignPtr",          2, primAlignPtr},
  {"minusPtr",          2, primMinusPtr},
  {"primEqPtr",         2, primEqPtr},
  {"primCmpPtr",        2, primCmpPtr},

  {"primEqInt",         2, primEqInt},
  {"primCmpInt",        2, primCmpInt},
  {"primEqWord",        2, primEqWord},
  {"primCmpWord",       2, primCmpWord},
  {"primEqChar",        2, primEqChar},
  {"primCmpChar",       2, primCmpChar},
  {"primEqFloat",       2, primEqFloat},
  {"primCmpFloat",      2, primCmpFloat},
  {"primEqDouble",      2, primEqDouble},
  {"primCmpDouble",     2, primCmpDouble},

  {"primUnsafeCoerce",  1, primDummyCvt},       /* breaks the type system */

  {"strict",            2, primStrict},
  {"seq",               2, primSeq},

  {"primMaxChar",	0, primMaxChar},
  {"isUpper",		1, primIsUpper},
  {"isLower",		1, primIsLower},
  {"isAlpha",		1, primIsAlpha},
  {"isAlphaNum",	1, primIsAlphaNum},
  {"isPrint",		1, primIsPrint},
  {"toUpper",		1, primToUpper},
  {"toLower",		1, primToLower},
  {"toTitle",		1, primToTitle},
  {"primUniGenCat",	1, primUniGenCat},

#if TREX
  {"recExt",            3, primRecExt},
  {"recBrk",            2, primRecBrk},
  {"recSel",            2, primRecSel},
  {"recShw",            5, primRecShw},
  {"recEq",		6, primRecEq},
#endif		        

#if OBSERVATIONS
  {"observe",           2, primObserve},
  {"bkpt",              2, primBkpt},
  {"setBkpt",           2+IOArity, primSetBkpt},
#endif

  {"unsafePtrEq",       2, primUnsafePtrEq},    /* breaks the semantics  */
  {"unsafePtrToInt",    1, primUnsafePtrToInt}, /* breaks the semantics  */

  {0,                   0, 0}
};

/* --------------------------------------------------------------------------
 * Primitive functions:
 *
 * IMPORTANT NOTICE: the primitive function definitions in this file
 * should be written in a style that permits correct execution *without*
 * conservative garbage collection (i.e., without marking from the C stack).
 * Adding primitive definitions that do not meet this requirement may
 * corrupt the heap and lead to failed execution; do not modify this code
 * unless you are really confident about what you are doing.
 *
 * Some general guidelines follow, using c, e to denote expressions that
 * involve either at most 1 allocation, or the possibility/certainty of
 * multiple allocations, resp.
 *
 * push(c);             Ok.
 * push(e);             Bad -- intermediate result may be lost if GC occurs
 *                      in the middle of building e; break e into steps, and
 *                      use toparg(), topfun(), etc.
 *
 * Cell x = ...;        Safe if value assigned to x will never be an
 * <any code with a     indirection.  (Otherwise, cell assigned to x may
 * possible alloc>      be returned to freeList *before* the value is used.)
 * ... x ...            Probably best avoided in other circumstances.
 *
 * updateRoot(e);       All ok.
 * updapRoot(e,e);
 * updateRoot(mkInt(n));
 * eval(pop());
 *
 * eval(ap(c,pop()));   Bad -- a GC call may corrupt value pop'd off stack.
 *
 * It is also worth a reminder that the fst and snd values passed in any call
 * to the allocator are automatically marked and preserved if a GC is needed.
 * As a result, code like the following is guaranteed to be safe:
 *  return ap(ap(mkTuple(2),ZERONUM),ZERONUM);    (ZERONUM is a constant)
 *  for ( ... )                                   (PROVIDED that ds is the
 *     ds = cons(consChar(c),ds);                  only var that needs GC).
 *
 * If these restrictions are judged to be too onerous in particular cases,
 * temporarily enable conservative GC (and reset it to the original state,
 * either on or off at the beginning of the operation).  See bignums.c
 * for an example.
 *
 * There are also certain conventions that must always be obeyed, regardless
 * of whether conservative GC is in use.  For example:
 *
 * lhs = expr;          If lhs involves an address calculation that may be
 *                      invalidated by a gc, and expr could trigger an alloc,
 *                      then this expression is bad, or at least not portable:
 *                      it will only do the right thing under some evaluation
 *                      orders.  For example:  hd(top()) = ap(..,..) is bad,
 *                      unless you know that top() will never be modified
 *                      during a GC.
 *
 *                      This is no different from the problems that occur
 *                      with non-portable combinations of stack operators
 *                      like push(top());  The solution is also the same:
 *                      use an intermediate variable to make the order
 *                      of evaluation explicit.
 *
 * If this version of Hugs has been modified to allow different or
 * additional run-time representations for certain values, then the
 * examples and principles illustrated here may need to be reconsidered,
 * and possibly reclassified.  The same will also be true if the execution
 * mechanisms etc., are changed in any way.  (And all this is assuming
 * that the original implementations are correct...)
 * ------------------------------------------------------------------------*/

primFun(primFatbar) {                   /* Fatbar primitive                */
    Cell temp = evalWithNoError(primArg(2));
    if (nonNull(temp))
	if (temp==nameFail)             /* _FAIL [] r = r                  */
	    updateRoot(primArg(1));
	else
	    throwException(temp);
    else
	updateRoot(primArg(2));         /* l     [] r = l  -- otherwise    */
}

primFun(primFail) {                     /* Failure primitive               */
    throwException(nameFail);
}

primFun(primBlackHole) {
    throwException(nameNonTermination);
}

primFun(primIndirect) {
    throwException(nameNonTermination);
}

primFun(primCatchError) {               /* Error catching  primitive       */
    Cell err = NIL;
    err = evalWithNoError(primArg(1));  /*  :: a -> Maybe a                */
    if (isNull(err)) {
	updapRoot(nameJust, primArg(1));
    } else {
	updateRoot(nameNothing);
    }
}

primFun(primThrowException) {           /* Failure primitive               */
    throwException(primArg(1));         /*  :: Exception -> a              */
}

/* This function ought to be in the IO monad to preserve referential       */
/* transparency but it has tricky interactions with the concurrency parts  */
/* of the IO monad so we provide it in unsafe form here and make it safe   */
/* in the Prelude.                                                         */

primFun(primCatchException) {	       /* Error catching primitive         */
    Cell err = NIL;                    /*  :: a -> Either Exception a      */
    err = evalWithNoError(primArg(1)); 
    if (isNull(err)) {
	updapRoot(nameRight, primArg(1));
    } else {
	updapRoot(nameLeft, err);
    }
}

primFun(primSel) {                      /* Component selection             */
    eval(primArg(2));                   /* _sel c e n  return nth component*/
    if (whnfHead==primArg(3))           /* in expr e, built with cfun c    */
	updateRoot(pushed(intOf(primArg(1))-1));
    else
	internal("primSel");
}

primFun(primIf) {                       /* Conditional primitive           */
    eval(primArg(3));
    checkBool();
    if (whnfHead==nameTrue)
	updateRoot(primArg(2));
    else
	updateRoot(primArg(1));
}

primFun(primStrict) {                   /* Strict application primitive    */
    eval(primArg(1));                   /* evaluate 2nd argument           */
    updapRoot(primArg(2),primArg(1));   /* and apply 1st argument to result*/
}

primFun(primSeq) {                      /* Strict sequencing primitive     */
    eval(primArg(2));                   /* evaluate 1st argument           */
    updateRoot(primArg(1));             /* and return the first            */
}

primFun(primConCmp) {                   /* compare constructors            */
    Int l,r;                            /*  :: a -> a -> Ordering          */
    ConArg(l,2);
    ConArg(r,1);
    updateRoot(l<r ? nameLT : (l>r ? nameGT : nameEQ));
}

primFun(primEnRange) {                  /* derived range for enum type     */
    eval(primArg(1));                   /* :: (a,a) -> [a]                 */
    updapRoot(ap(nameEnFrTo,primArg(3)),primArg(2));
}

primFun(primEnIndex) {                  /* derived index for enum type     */
    Int l,h,ix;                         /*  :: (a,a) -> a -> Int           */
    eval(primArg(2));
    ConArg(l,4);                        /* evaluate lower bound            */
    ConArg(h,3);                        /* evaluate upper bound            */
    ConArg(ix,1);                       /* evaluate index                  */
    if (l<=ix && ix<=h) {
	IntResult(ix-l);
    } else {
	throwException(ap(nameErrorCall,
		 mkStr(findText("Ix.index: Index out of range."))));
    }
}

primFun(primEnInRng) {                  /* derived inRange for enum type   */
    Int l,h,ix;                         /*  :: (a,a) -> a -> Bool          */
    eval(primArg(2));
    ConArg(l,4);                        /* evaluate lower bound            */
    ConArg(h,3);                        /* evaluate upper bound            */
    ConArg(ix,1);                       /* evaluate index                  */
    BoolResult(l<=ix && ix<=h);
}

primFun(primEnToEn) {                   /* derived toEnum for enum type    */
    Name n;                             /* :: a -> Int -> a                */
    Int  i;
    eval(primArg(2));
    checkCon();
    n = whnfHead;
    IntArg(i,1);
    if (nonNull(n = cfunByNum(n,i)))
	updateRoot(n);
    else
	throwException(ap(nameErrorCall,
		mkStr(findText("toEnum: out of range"))));
}

primFun(primEnFrEn) {                   /* derived fromEnum for enum type  */
    Int i;                              /* :: a -> Int                     */
    ConArg(i,1);
    IntResult(i==0 ? 0 : (i-1));
}

primFun(primEnFrom) {                   /* derived enumFrom for enum type  */
    Name cfs;                           /* :: a -> [a]                     */
    eval(primArg(1));
    checkCon();
    cfs = succCfun(whnfHead);
    push(isNull(cfs) ? nameNil : ap(nameEnFrom,cfs));
    updapRoot(ap(nameCons,whnfHead),top());
}

primFun(primEnFrTo) {                   /* derived enumFromTo for enum type*/
    Name l,r;                           /* :: a -> a -> [a]                */
    eval(primArg(2));
    checkCon();
    l = whnfHead;
    eval(primArg(1));
    checkCon();
    r = whnfHead;
    if (cfunOf(l) < cfunOf(r)) {
	push(ap(nameEnFrTo,succCfun(l)));
	updapRoot(ap(nameCons,l),ap(top(),whnfHead));
    }
    else if (l==r) {
	updapRoot(ap(nameCons,l),nameNil);
    } else {
	updateRoot(nameNil);
    }
}

primFun(primEnFrTh) {                   /* derived enumFromThen for enum ty*/
    Name f,n;                           /* :: a -> a -> [a]                */
    eval(primArg(2));   
    checkCon();
    f = whnfHead;
    eval(primArg(1));
    checkCon();
    n = nextCfun(f,whnfHead);
    if (isNull(n)) {
	push(ap(nameCons,whnfHead));
	toparg(nameNil);
    }
    else {
	push(ap(nameEnFrTh,whnfHead));
	toparg(n);
    }
    updapRoot(ap(nameCons,f),top());
}



/* --------------------------------------------------------------------------
 * Integer arithmetic primitives:
 * ------------------------------------------------------------------------*/

CAFInt(primMinInt,MINNEGINT)           /* minimum integer CAF              */
CAFInt(primMaxInt,MAXPOSINT)           /* maximum integer CAF              */
IntInt2Int(primPlusInt,x+y)            /* Integer addition primitive       */
IntInt2Int(primMinusInt,x-y)           /* Integer subtraction primitive    */
IntInt2Int(primMulInt,x*y)             /* Integer multiplication primitive */
Int2Int(primNegInt,-x)                 /* Integer negation primitive       */
IntInt2IntNonZero(primQuotInt,x/y)     /* Integer division primitive       */
				       /* truncated towards zero           */
IntInt2IntNonZero(primRemInt,x%y)      /* Integer remainder primitive      */

/* quot and rem satisfy:                                                   */
/*     (x `quot` y)*y + (x `rem` y) == x                                   */
/* which is exactly the property described in K&R 2:                       */
/*     (a/b)*b + a%b == a                                                  */

primFun(primQrmInt) {                  /* Integer quotient and remainder   */
    Int x, y;                          /* truncated towards zero           */
    IntArg(x,2);
    IntArg(y,1);
    if (y==0)
	throwException(ap(nameArithException, nameDivideByZero));
    IntIntResult(x/y,x%y);
}

primFun(primDivInt) {                  /* Integer division primitive       */
    Int x,y,r;                         /* truncated towards -ve infinity   */
    IntArg(x,2);
    IntArg(y,1);
    if (y==0)
	throwException(ap(nameArithException, nameDivideByZero));
    r = x%y;
    x = x/y;
    if ((y<0 && r>0) || (y>0 && r<0))
	x--;
    IntResult(x);
}

primFun(primModInt) {                  /* Integer modulo primitive         */
    Int x,y,r;
    IntArg(x,2);
    IntArg(y,1);
    if (y==0)
	throwException(ap(nameArithException, nameDivideByZero));
    r = x%y;                           /* "... the modulo having the sign  */
    if ((r<0 && y>0) ||                /*              of the divisor ..." */
	(r>0 && y<0)) {                /* See definition on p.91 of Haskell*/
	IntResult(r+y);                /* report... (Haskell 1.1?)         */
    } else {
	IntResult(r);
    }
}

IntInt2Int(primAndInt,x&y)  
IntInt2Int(primOrInt, x|y)  
IntInt2Int(primXorInt,(x&~y) | (~x&y))   
Int2Int(primComplementInt,~x)
Int2Int(primBitInt, 1<<x)
IntInt2Bool(primTestInt,(x >> y) & 1)

primFun(primShiftInt) {
    Int x,y;
    IntArg(x,2);
    IntArg(y,1);
    if (y >= 0) {
	IntResult(x << y);
    } else {
	IntResult(x >> (-y));
    }
}

/* --------------------------------------------------------------------------
 * Unsigned arithmetic primitives:
 * ------------------------------------------------------------------------*/

#if WORD_OPS
CAFWord(primMaxWord,MAXHUGSWORD)       /* maximum integer CAF              */
WordWord2Word(primPlusWord,x+y)        /* Word addition primitive          */
WordWord2Word(primMinusWord,x-y)       /* Word subtraction primitive       */
Word2Word(primNegateWord,-(Int)x)      /* Word negation (modulo MAXWORD)   */
WordWord2Word(primMulWord,x*y)         /* Word multiplication primitive    */
WordWord2WordNonZero(primQuotWord,x/y) /* Word division primitive          */
				       /* truncated towards zero           */
WordWord2WordNonZero(primDivWord,x/y)  /* Word division primitive          */
				       /* truncated towards zero           */
WordWord2WordNonZero(primRemWord,x%y)  /* Word remainder primitive         */
WordWord2WordNonZero(primModWord,x%y)  /* Word modulo primitive            */

/* quot and rem satisfy:                                                   */
/*     (x `quot` y)*y + (x `rem` y) == x                                   */
/* which is exactly the property described in K&R 2:                       */
/*     (a/b)*b + a%b == a                                                  */

primFun(primQrmWord) {                 /* Integer quotient and remainder   */
    Unsigned x, y;                     /* truncated towards zero           */
    WordArg(x,2);
    WordArg(y,1);
    if (y==0)
	throwException(ap(nameArithException, nameDivideByZero));
    WordWordResult(x/y,x%y);
}

WordWord2Word(primAndWord,x&y)  
WordWord2Word(primOrWord, x|y)  
WordWord2Word(primXorWord,(x&~y) | (~x&y))   
Word2Word(primComplementWord,~x)
Int2Word(primBitWord, 1<<x)
WordInt2Bool(primTestWord,(x >> y) & 1)

primFun(primShiftWord) {
    Unsigned x;         
    Int      y;
    WordArg(x,2);
    IntArg(y,1);
    if (y >= 0) {
	/* << isn't defined for y larger than word size */
	WordResult(y >= (Int)(sizeof(x) * 8)  ? 0 : x << y);
    } else {
        y = -y;
	WordResult(y >= (Int)(sizeof(x) * 8) ? 0 : x >> y);
    }
}

primFun(primRotateWord) {
    Unsigned x;         
    Int      y, z;
    WordArg(x,2);
    IntArg(y,1);
    IntArg(z,3);
    y = y % z;
    if (y >= 0) {
	WordResult((x << y) | (x >> (z - y)));
    } else {
	WordResult((x >> (-y)) | (x << (z + y)));
    }
}

Int2Int(primItoI8,  x&0xff)
Int2Int(primItoI16, x&0xffff)
Int2Int(primItoI32, x&0xffffffff)

Int2Int(primI8toI,  (Int8)x)        /* casts used to cause sign extension */
Int2Int(primI16toI, (Int16)x)       /* casts used to cause sign extension */
Int2Int(primI32toI, x)

Word2Word(primWtoW8,  x&0xff)
Word2Word(primWtoW16, x&0xffff)
Word2Word(primWtoW32, x&0xffffffff)

Word2Word(primW8toW,  x)
Word2Word(primW16toW, x)
Word2Word(primW32toW, x)

primFun(primI64toI32) {
    Cell x, y;
    eval(primArg(1)); 
    x = fst(snd(whnfHead));
    y = snd(snd(whnfHead));
    updapRoot(ap(mkTuple(2),x),y);
}

primFun(primI32toI64) {
    Int x, y;
    IntArg(x,2);
    IntArg(y,1);
    updateRoot(pair(I64CELL,pair(mkInt(x),mkInt(y))));
}

primFun(primW64toW32) {
    Cell x, y;
    eval(primArg(1)); 
    x = fst(snd(whnfHead));
    y = snd(snd(whnfHead));
    updapRoot(ap(mkTuple(2),x),y);
}

primFun(primW32toW64) {
    Unsigned x, y;
    WordArg(x,2);
    WordArg(y,1);
    updateRoot(pair(I64CELL,pair(mkInt(x),mkInt(y))));
}

#endif /* WORD_OPS */

/* --------------------------------------------------------------------------
 * Haskell Integer (bignum) primitives:
 * ------------------------------------------------------------------------*/

#if BIGNUMS
/* --------------------------------------------------------------------------
 * Functions for manipulating Haskell Integers (bignums).
 *
 * The Hugs 98 system is Copyright (c) Mark P Jones, Alastair Reid, the
 * Yale Haskell Group, and the OGI School of Science & Engineering at OHSU,
 * 1994-2003, All rights reserved.  It is distributed as free software under
 * the license in the file "License", which is included in the distribution.
 *
 * $RCSfile: bignums.c,v $
 * $Revision: 1.15 $
 * $Date: 2004/10/29 12:43:09 $
 * ------------------------------------------------------------------------*/

/*#define DEBUG_BIGNUMS*/

#if DEBUG_BIGNUMS
static Void local bigDump(List ds, Int n) {
    while (nonNull(ds) && 0<n--) {
	Printf(":%04d",digitOf(hd(ds)));
	ds = tl(ds);
    }
}
#endif

static Cell bn;			/* Bignum accumulator; used in bignum	   */
				/* calcs, visited by the garbage collector.*/
static List bigRem = NIL;	/* used in bigQrm to return remainder	   */


/*-------------------------------------------------------------------------*/

PROTO_PRIM(primPlusInteger);
PROTO_PRIM(primMinusInteger);
PROTO_PRIM(primMulInteger);
PROTO_PRIM(primQrmInteger);
PROTO_PRIM(primNegInteger);
PROTO_PRIM(primIntToInteger);
PROTO_PRIM(primIntegerToInt);
PROTO_PRIM(primWordToInteger);
PROTO_PRIM(primIntegerToWord);
PROTO_PRIM(primIntegerToFloat);
PROTO_PRIM(primIntegerToDouble);
PROTO_PRIM(primEqInteger);
PROTO_PRIM(primCmpInteger);

static struct primitive bignumPrimTable[] = {
  {"primPlusInteger",	2, primPlusInteger},
  {"primMinusInteger",	2, primMinusInteger},
  {"primMulInteger",	2, primMulInteger},
  {"primQrmInteger",	2, primQrmInteger},
  {"primNegInteger",	1, primNegInteger},
  {"primIntToInteger",	1, primIntToInteger},
  {"primIntegerToInt",	1, primIntegerToInt},
  {"primWordToInteger",	1, primWordToInteger},
  {"primIntegerToWord",	1, primIntegerToWord},
  {"primIntegerToFloat",1, primIntegerToFloat},
  {"primIntegerToDouble",1,primIntegerToDouble},
  {"primEqInteger",	2, primEqInteger},
  {"primCmpInteger",	2, primCmpInteger},
  {0,			0, 0}
};

/* --------------------------------------------------------------------------
 * Bignum control:
 * ------------------------------------------------------------------------*/

#if SHORT_CIRCUIT_COERCIONS
static Name nameIntToInteger;
#endif

static Void bignumControl (Int);
static Void bignumControl(what)
Int what; {
    switch (what) {
	case MARK    : mark(bn);
		       mark(bigRem);
		       break;
	case INSTALL : 
#if SHORT_CIRCUIT_COERCIONS
#define predef(nm,str) nm=newName(findText(str),NIL); name(nm).defn=PREDEFINED
		       predef(nameIntToInteger, "primIntToInteger");
#undef predef
#endif
	case RESET   : bn     = NIL;
		       bigRem = NIL;
		       break;
    }
}

static struct primInfo bignumPrims = { bignumControl, bignumPrimTable, 0 };

/* Used by the IO implementation, so not 'static' like the rest. */
extern Bignum bigWord   (Unsigned);

/* --------------------------------------------------------------------------
 * Local function prototypes:
 * ------------------------------------------------------------------------*/

static Int    local digitsCmp (List,List);
static Bignum local bigAdd    (Bignum,Bignum);
static Bignum local bigSub    (Bignum,Bignum);
static Bignum local digitsAdd (Cell,List,List);
static Bignum local digitsSub (List,List);
static Bignum local bigMul    (Bignum,Bignum);
static Bignum local bigQrm    (Bignum,Bignum);
static List   local digitsQrm (List,List);

/*---------------------------------------------------------------------------
 * Simple bignum primitives:
 *-------------------------------------------------------------------------*/

Bignum bigInt(n)			/* convert Int to bignum	   */
Int n; {
    if (n==0)
	return ZERONUM;
    else {
	unsigned long no;
	Cell nx;
	if (n<0) {
	    no = (unsigned long)(-n);
	    bn = pair(NEGNUM,NIL);
	}
	else {
	    no = (unsigned long)(n);
	    bn = pair(POSNUM,NIL);
	}
	for (nx=bn; no>0; no/=BIGBASE, nx=tl(nx)) {
	    tl(nx) = pair(mkDigit(no%BIGBASE),NIL);
	}
	return bn;
    }
}

Bignum bigWord(n)		      /* convert Word to bignum	   */
Unsigned n; {
    if (n==0)
	return ZERONUM;
    else {
	unsigned long no;
	Cell nx;
	no = (unsigned long)(n);
	bn = pair(POSNUM,NIL);
	for (nx=bn; no>0; no/=BIGBASE, nx=tl(nx)) {
	    tl(nx) = pair(mkDigit(no%BIGBASE),NIL);
	}
	return bn;
    }
}

Bignum bigDouble(a)			/* convert double to bignum	   */
double a; {
    if (a==0) {
	return ZERONUM;
    } else {
	double b; /* Obscure use of a and b is to avoid a bug in Symantec C*/
	Cell nx;
	if (a<0) {
	    b  = (-a);
	    bn = pair(NEGNUM,NIL);
	} else {
	    b = a;
	    bn = pair(POSNUM,NIL);
	}
	b = floor(b);
	/* NB: in IEEE floating point, !(b>0) is not the same as b<=0 */
	if (!(b>0))			/* happens on IEEE NaN */
	    throwException(ap(nameArithException, nameOverflow));
	for (nx=bn; b>0; nx=tl(nx)) {
	    double n = fmod(b,(double)(BIGBASE));
	    Int d    = (Int)n;
	    if (d<0)			/* happens on IEEE inf and -inf */
		throwException(ap(nameArithException, nameOverflow));
	    tl(nx)   = pair(mkDigit(d),NIL);
	    b        = (b - n) / BIGBASE;
	}
	return bn;
    }
}

/* The library documentation says that conversion between signed and
 * unsigned types preserves representation, not sign.  Since this
 * conversion uses fromInteger, and we do note know the original type,
 * fromInteger is the same for Int and Word, and must accept values
 * in the range MINNEGINT..MAXHUGSWORD.
 */

Cell bigToInt(n)			/* convert bignum to Int	   */
Bignum n; {
    if (n!=ZERONUM) {
	List ds = snd(n);
	if (nonNull(ds)) {
	    Int m;
	    Int b = 1;
	    if (fst(n)==POSNUM) {
		m = digitOf(hd(ds));
		while (nonNull(ds=tl(ds))) {
		    Int d = digitOf(hd(ds));
		    if (b > (Int)(MAXHUGSWORD/BIGBASE))
			return NIL;
		    b *= BIGBASE;
		    if (d > (Int)((MAXHUGSWORD - m)/b))
			return NIL;
		    m += b*d;
		}
	    } else { /* fst(n)==NEGNUM */
		m = - digitOf(hd(ds));
		while (nonNull(ds=tl(ds))) {
		    Int d = - digitOf(hd(ds));
		    if (b > (MAXPOSINT/BIGBASE))
			return NIL;
		    b *= BIGBASE;
		    if (d < (MINNEGINT - m)/b)
			return NIL;
		    m += b*d;
		}
	    }
	    return mkInt(m);
	}
    }
    return mkInt(0);
}

double bigToDouble(n)			/* convert bignum to double	  */
Bignum n; {
    if (n==ZERONUM)
	return 0.0;
    else {
	double m  = 0.0;
	double b  = 1.0;
	List  ds = snd(n);
	for (; nonNull(ds); ds=tl(ds)) {
	    m += b*digitOf(hd(ds));
	    b *= BIGBASE;
	}
	return fst(n)==POSNUM ? m : (-m);
    }
}

Bignum bigStr(s)			/* convert String to bignum	   */
String s; {				/* Surprisingly, this is GC safe   */
    List   ds = NIL;			/* because ds is the only variable */
    String t  = s;			/* that needs marking, and always  */
    Int    i;				/* appears as the snd of each cons */

    if (*t == '-')			/* look for leading minus	   */
	t++;				/* and ignore for time being	   */
    if ((i=(size_t)strlen(t)%BIGEXP)!=0) {
	Int d = 0;
	while (0<i--)
	    d = d*10 + (*t++ - '0');
	if (d)
	    ds = cons(mkDigit(d),NIL);
    }
    while (*t) {			/* now scan rest of string	   */
	Int d = 0;
	for (i=BIGEXP; i>0; i--)
	    d = d*10 + (*t++ - '0');
	if (nonNull(ds) || d)
	    ds = cons(mkDigit(d),ds);
    }

    return isNull(ds) ? ZERONUM : pair((*s=='-' ? NEGNUM : POSNUM), ds);
}

Cell bigOut(a,s,b)			/* bignum output, prepend digits to*/
Bignum a;				/* stream s			   */
Cell   s;				/* GC safe; s is snd in each cons  */
Bool   b; {				/* TRUE => wrap neg int in parens  */
    if (a==ZERONUM)
	return ap(consChar('0'),s);
    else {
	Bool isNeg = fst(a)==NEGNUM;	/* keep record of sign		   */
	bn         = snd(a);		/* list of digits		   */

	if (b && isNeg)			/* print closing paren		   */
	    s = ap(consChar(')'),s);

	for (;;) {
	    Int d = digitOf(hd(bn));	/* get next digit		   */
	    bn    = tl(bn);		/* move to next digit		   */
	    if (nonNull(bn)) {		/* more digits to come after this  */
		Int i = BIGEXP;
		for (; i>0; i--, d/=10)
		    s = ap(consChar('0'+(d%10)),s);
	    }
	    else {			/* leading (non-zero) digits	   */
		for (; d; d/=10)
		    s = ap(consChar('0'+(d%10)),s);
		break;
	    }
	    allowBreak();
	}

	if (isNeg)			/* print minus sign		   */
	    s = ap(consChar('-'),s);
	if (b && isNeg)			/* print open paren		   */
	    s = ap(consChar('('),s);
	return s;
    }
}

Bignum bigShift(big,c,mult)		/* Digits 0 <= c, mult < BIGBASE   */
Bignum big;				/* Calculate big*mult+c, if big>=0 */
Int    c;				/*           big*mult-c, if big<0  */
Int    mult; {				/* UPDATE big, DESTRUCTIVELY!	   */
    if (big==ZERONUM)
	return (c==0) ? ZERONUM : pair(POSNUM,singleton(mkDigit(c)));
    else {
	Cell nx = bn = big;
	while (nonNull(tl(nx))) {
	    nx     = tl(nx);
	    c      = digitOf(hd(nx))*mult + c;
	    hd(nx) = mkDigit(c % BIGBASE);
	    c      = c / BIGBASE;	/* N.B. new carry is < BIGBASE	   */
	}
	if (c>0)
	    tl(nx) = singleton(mkDigit(c));
    }
    return big;
}

/*---------------------------------------------------------------------------
 * Simple Bignum operations:
 *-------------------------------------------------------------------------*/

primFun(primIntToInteger) {		/* Conversion :: Int -> Integer	   */
    eval(primArg(1));
    updateRoot(bigInt(whnfInt));
}

primFun(primIntegerToInt) {		/* Conversion :: Integer -> Int	   */
#if SHORT_CIRCUIT_COERCIONS
    /* Optimisation: we try to short-circuit trivial conversions */
    Cell x = followInd(primArg(1));
    if (isAp(x) && followInd(fun(x)) == nameIntToInteger) {
	updateRoot(arg(x));
	return;
    } else
#endif
    {
	eval(primArg(1));
	whnfHead = bigToInt(whnfHead);
	if (nonNull(whnfHead)) {
	    updateRoot(whnfHead);
	} else {
	    throwException(ap(nameArithException, nameOverflow));
	}
    }
}

primFun(primWordToInteger) {		/* Conversion :: Word -> Integer   */
    eval(primArg(1));
    updateRoot(bigWord(whnfInt));
}

primFun(primIntegerToWord) {		/* Conversion :: Integer -> Word   */
    eval(primArg(1));
    whnfHead = bigToInt(whnfHead);
    if (nonNull(whnfHead))
	updateRoot(whnfHead);
    else
	throwException(ap(nameArithException, nameOverflow));
}

primFun(primIntegerToFloat) {		/* Conversion :: Integer -> Float  */
    eval(primArg(1));
    updateRoot(mkFloat((FloatPro)bigToDouble(whnfHead)));
}

primFun(primIntegerToDouble) {		/* Conversion :: Integer -> Double */
    eval(primArg(1));
    updateRoot(mkDouble((DoublePro)bigToDouble(whnfHead)));
}

primFun(primNegInteger) {		/* Integer unary negate		   */
    eval(primArg(1));
    updateRoot(bigNeg(whnfHead));
}

Bignum bigNeg(a)		        /* unary negation		   */
Bignum a; {
    if (a==ZERONUM)
	return ZERONUM;
    else
	return pair(((fst(a)==POSNUM) ? NEGNUM : POSNUM), snd(a));
}

/*---------------------------------------------------------------------------
 * Bignum comparison routines:
 *-------------------------------------------------------------------------*/

primFun(primEqInteger) {		/* Integer equality test	   */
    eval(primArg(2));
    primArg(2) = whnfHead;
    eval(primArg(1));
    updateRoot(bigCmp(primArg(2),whnfHead)==0 ? nameTrue : nameFalse);
}

primFun(primCmpInteger) {		/* Integer comparison		   */
    eval(primArg(2));
    primArg(2) = whnfHead;
    eval(primArg(1));
    switch (bigCmp(primArg(2),whnfHead)) {
	case (-1) : updateRoot(nameLT); break;
	case   0  : updateRoot(nameEQ); break;
	case   1  : updateRoot(nameGT); break;
    }
}

Int bigCmp(a,b)				/* Compare bignums returning:	   */
Bignum a, b; {				/* -1 if a<b,  +1 if a>b,  0 o/w   */
    if (a==ZERONUM)
	return (b==ZERONUM) ? 0 : ((fst(b)==POSNUM) ? (-1) : 1);
    else if (fst(a)==NEGNUM)
	if (b==ZERONUM || fst(b)==POSNUM)
	    return (-1);
	else
	    return digitsCmp(snd(b),snd(a));
    else
	if (b==ZERONUM || fst(b)==NEGNUM)
	    return 1;
	else
	    return digitsCmp(snd(a),snd(b));
}

static Int local digitsCmp(xs,ys)	/* Compare positive digit streams  */
List xs, ys; {				/* -1 if xs<ys, +1 if xs>ys, 0 if= */
    Int s = 0;
    for (; nonNull(xs) && nonNull(ys); xs=tl(xs), ys=tl(ys)) {
	Int x = hd(xs);
	Int y = hd(ys);
	if (x<y)
	    s = (-1);
	else if (x>y)
	    s = 1;
    }
    return (nonNull(xs) ? 1 : (nonNull(ys) ? (-1) : s));
}

/*---------------------------------------------------------------------------
 * Addition and subtraction:
 *-------------------------------------------------------------------------*/

static Bignum local bigAdd(a,b)		/* Bignum addition		   */
Bignum a, b; {
    if (a==ZERONUM)
	return b;
    else if (b==ZERONUM)
	return a;
    else if (fst(a)==POSNUM)
	if (fst(b)==POSNUM)
	    return digitsAdd(POSNUM,snd(a),snd(b));
	else
	    return digitsSub(snd(a),snd(b));
    else /* fst(a)==NEGNUM */
	if (fst(b)==NEGNUM)
	    return digitsAdd(NEGNUM,snd(a),snd(b));
	else
	    return digitsSub(snd(b),snd(a));
}

static Bignum local bigSub(a,b)		/* Bignum subtraction		   */
Bignum a, b; {
    if (a==ZERONUM)
	return bigNeg(b);
    else if (b==ZERONUM)
	return a;
    else if (fst(a)==POSNUM)
	if (fst(b)==NEGNUM)
	    return digitsAdd(POSNUM,snd(a),snd(b));
	else
	    return digitsSub(snd(a),snd(b));
    else /* fst(a)==NEGNUM */
	if (fst(b)==POSNUM)
	    return digitsAdd(NEGNUM,snd(a),snd(b));
	else
	    return digitsSub(snd(b),snd(a));
}

static Bignum local digitsAdd(sign,xs,ys)/* Addition of digit streams	   */
Cell sign;
List xs, ys; {
    Cell nx = bn = pair(sign,NIL);
    Int  c  = 0;
    for (;;) {
	if (nonNull(xs)) {			/* Add any digits to carry */
	    if (nonNull(ys)) {
		c += digitOf(hd(xs)) + digitOf(hd(ys));
		xs = tl(xs);
		ys = tl(ys);
	    }
	    else if (c==0) {			/* look for short cut when */
		tl(nx) = xs;			/* a stream ends and there */
		break;				/* is no outstanding carry */
	    }
	    else {
		c += digitOf(hd(xs));
		xs = tl(xs);
	    }
	}
	else if (c==0) {
	    tl(nx) = ys;
	    break;
	}
	else if (nonNull(ys)) {
	    c += digitOf(hd(ys));
	    ys = tl(ys);
	}

	if (c>=BIGBASE) {			/* Calculate output digit  */
	    nx = tl(nx) = cons(mkDigit(c-BIGBASE),NIL);
	    c  = 1;
	}
	else {					/* Carry will always be >0 */
	    nx = tl(nx) = cons(mkDigit(c),NIL);	/* at this point	   */
	    c  = 0;
	}
	allowBreak();
    }
    return bn;
}

static Bignum local digitsSub(xs,ys)	/* Subtraction of digit streams    */
List xs, ys; {
    Cell nx;
    Int  b  = 0;
    Int  lz = 0;

    switch (digitsCmp(xs,ys)) {
	case (-1) : nx = xs;		/* if xs<ys, return -(ys-xs)	   */
		    xs = ys;
		    ys = nx;
		    bn = pair(NEGNUM,NIL);
		    break;

	case   0  : return ZERONUM;	/* if xs=ys, return 0		   */

	case   1  : bn = pair(POSNUM,NIL);
		    break;		/* if xs>ys, return +(xs-ys)	   */
    }

    nx = bn;				/* Now we can assume that xs>ys    */
    for (;;) {				/* Scan each digit		   */
	Int y = b;
	if (nonNull(ys)) {
	    y += digitOf(hd(xs)) - digitOf(hd(ys));
	    xs = tl(xs);
	    ys = tl(ys);
	}
	else if (y==0) {
	    if (nonNull(xs))
		for (; lz>0; lz--)
		    nx = tl(nx) = cons(mkDigit(0),NIL);
	    tl(nx) = xs;
	    break;
	}
	else {
	    y += digitOf(hd(xs));	/* xs>ys, so we can't run out of   */
	    xs = tl(xs);		/* digits of xs while y!=0	   */
	}

	if (y<0) {			/* Calculate output digit	   */
	    y += BIGBASE;
	    b  = (-1);
	}
	else
	    b  = 0;

	if (y==0)			/* Don't insert leading zeros	   */
	    lz++;
	else {
	    for (; lz>0; lz--)
		nx = tl(nx) = cons(mkDigit(0),NIL);
	    nx = tl(nx) = cons(mkDigit(y),NIL);
	}
	allowBreak();
    }
    return bn;
}

/*---------------------------------------------------------------------------
 * Multiplication:
 *-------------------------------------------------------------------------*/

static Bignum local bigMul(a,b)		/* Bignum multiply		   */
Bignum a, b; {
    if (a==ZERONUM || b==ZERONUM)	/* if either operand is zero, then */
	return ZERONUM;			/* so is the result ...		   */
    else {				/* otherwise, use rule of signs:   */
	Cell nx = bn = ap((hd(a)==hd(b) ? POSNUM : NEGNUM), NIL);
	for (; nonNull(b=tl(b)); nx=tl(nx)) {	/* loop through digits of b*/
	    List as = nx;		/* At each stage of the loop, add  */
	    List xs = tl(a);		/* y * xs to the value in result,  */
	    Int  y  = digitOf(hd(b));	/* using c as carry		   */
	    Int  c  = 0;
	    for (; nonNull(xs); xs=tl(xs)) {	/* loop through digits of a*/
		c += digitOf(hd(xs)) * y;
		if (nonNull(tl(as))) {
		    as = tl(as);
		    c += digitOf(hd(as));
		}
		else
		    as = tl(as) = cons(NIL,NIL);
		hd(as) = mkDigit(c % BIGBASE);
		c     /= BIGBASE;
	    }
	    if (c>0)			/* add carry digit, if required	   */
		tl(as) = cons(mkDigit(c),NIL);
	    allowBreak();
	}
	return bn;
    }
}

/*---------------------------------------------------------------------------
 * Division:
 *-------------------------------------------------------------------------*/

static Cell local bigQrm(a,b)		/* bignum quotient and remainder   */
Bignum a, b; {
    if (b==ZERONUM)			/* division by zero?		   */
	return NIL;
    else if (a==ZERONUM)   		/* 0 `div` x == 0 `mod` x == 0	   */
	return ap(ap(mkTuple(2),ZERONUM),ZERONUM);
    else {
	/* The sign of the quotient is positive if numerator and denominator
	 * have the same sign, negative if the signs differ.  The sign of the
	 * remainder is always the same as the sign of the numerator.
	 */
	Cell qsign = (fst(a)==fst(b) ? POSNUM : NEGNUM);
	Cell rsign = fst(a);
	bn         = digitsQrm(snd(a),snd(b));
	bn         = isNull(bn) ? ZERONUM : pair(qsign,bn);
	bn         = pair(mkTuple(2),bn);
	bigRem     = isNull(bigRem) ? ZERONUM : pair(rsign,bigRem);
	bn         = ap(bn,bigRem);
	bigRem     = NIL;
	return bn;
    }
}

static List local digitsQrm(us,vs)	/* digits quotient and remainder   */
List us, vs; {
    Bool gc = consGC;
    consGC  = TRUE;
    if (isNull(tl(vs))) {		/* single digit divisor		   */
	Int  v   = digitOf(hd(vs));
	Int  r   = 0;
	List us1 = NIL;			/* first, copy and reverse us	   */
	for (; nonNull(us); us=tl(us))
	    us1 = cons(hd(us),us1);
	while (nonNull(us1)) {		/* then do division, MSD first	   */
	    Cell tmp = tl(us1);
	    Int  u   = r * BIGBASE + digitOf(hd(us1));
	    r        = u % v;
	    u	     = u / v;
	    if (nonNull(us) || u) {	/* output quotient digit	   */
		hd(us1) = mkDigit(u);
		tl(us1) = us;
		us 	= us1;
	    }
	    us1	     = tmp;
	}
	bigRem = r ? singleton(mkDigit(r)) : NIL;
	consGC = gc;
	return us;
    }
    else {				/* at least two digits in divisor  */

	/* The division algorithm used here is, inevitably, based on the
	 * description in Knuth's volume 2 on Seminumerical algorithms,
	 * and is probably at least as incomprehensible as the MIX
	 * implementation given there :-)
	 */

	List us1 = NIL;
	List vs1 = NIL;
	List ds  = us;
	Int  v1  = 0, v2  = 0;
	Int  uj  = 0, uj1 = 0, uj2 = 0;
	Int  n   = 0;
	List qs  = NIL;
	Int  sc;

	while (nonNull(us) && nonNull(vs)) {
	    v2  = v1;
	    v1  = digitOf(hd(vs));
	    vs1 = cons(hd(vs),vs1);
	    vs  = tl(vs);

	    uj2 = uj1;
	    uj1 = digitOf(hd(us));
	    us1 = cons(hd(us),us1);
	    us  = tl(us);

	    n++;
	}

	if (nonNull(vs)) {		/* if us is shorter than vs, then  */
	    bigRem = ds;		/* remainder is us, quotient zero  */
	    consGC = gc;
	    return NIL;
	}
	vs = rev(vs1);

	/* Now we have:
	 *   n      = number of digits in vs which is at least two (we
	 *	      also know that us has at least n digits),
	 *   v1, v2 = most significant digits of vs
	 *   vs     = digits of vs with least significant digit first
	 */

#if DEBUG_BIGNUMS
Printf("initial vs (n=%d, v1=%d, v2=%d): ",n,v1,v2);
bigDump(vs,n);
Putchar('\n');
#endif
	while (nonNull(us)) {
	    uj2 = uj1;
	    uj1 = digitOf(hd(us));
	    us1 = cons(hd(us),us1);
	    us  = tl(us);
	}

	us = cons(mkDigit(uj=0),NIL);
	ds = us1;
	for (vs1=tl(vs); nonNull(vs1); vs1=tl(vs1)) {
	    us1    = tl(ds);
	    tl(ds) = us;
	    us     = ds;
	    ds     = us1;
	}

	/* And, at this point, we have:
	 *   us = first (n-1) significant digits of original numerator,
	 *	  with least significant digit first, and a zero at the
	 *	  end (MSD) of the list, so that length us == n.
	 *   ds = remaining digits of the numerator, most significant
	 *	  digit first.
	 *   uj, uj1, uj2
	 *      = first three significant digits of us.  (At this point,
	 *	  uj is actually zero.)
	 */

#if DEBUG_BIGNUMS
Printf("initial us (uj=%d, uj1=%d, uj2=%d): ",uj,uj1,uj2);
bigDump(us,n);
Putchar('\n');
Printf("initial ds: ");
bigDump(ds,1000);
Putchar('\n');
#endif
	sc = BIGBASE / (v1+1);
#if DEBUG_BIGNUMS
Printf("scaling factor %d\n",sc);
#endif
	if (sc!=1) {			/* Scale numerator and denominator */
	    Int c = 0;
	    v1 = v2 = 0;
	    for (vs1=vs; nonNull(vs1); vs1=tl(vs1)) {
		v2 = v1;
		v1 = sc * digitOf(hd(vs1)) + c;
		c  = v1 / BIGBASE;
		hd(vs1) = mkDigit(v1%=BIGBASE);
	    }				/* no carry here, guaranteed	   */

	    c = uj = uj1 = uj2 = 0;
	    for (us1=ds=rev(ds); nonNull(us1); us1=tl(us1)) {
		uj2 = uj1;
		uj1 = uj;
		uj  = sc * digitOf(hd(us1)) + c;
		c   = uj / BIGBASE;
		hd(us1) = mkDigit(uj%=BIGBASE);
	    }
	    for (ds=rev(ds), us1=us; nonNull(us1); us1=tl(us1)) {
		uj2 = uj1;
		uj1 = uj;
		uj  = sc * digitOf(hd(us1)) + c;
		c   = uj / BIGBASE;
		hd(us1) = mkDigit(uj%=BIGBASE);
	    }				/* no carry here, guaranteed	   */
	}

#if DEBUG_BIGNUMS
Printf("scaled vs (n=%d, v1=%d, v2=%d): ",n,v1,v2);
bigDump(vs,n);
Putchar('\n');
Printf("scaled us (uj=%d, uj1=%d, uj2=%d): ",uj,uj1,uj2);
bigDump(us,n);
Putchar('\n');
Printf("scaled ds: ");
bigDump(ds,1000);
Putchar('\n');
#endif
	/* Most of the conditions above are still valid, except that both
	 * the numerator and denominator have been multiplied by the scaling
	 * factor sc, and the values of the various digit positions have been
	 * updated accordingly.
	 *
	 * Now we can start the division algorithm proper:
	 */

	while (nonNull(ds)) {
	    Int nd, c;			/* Guess a value for quotient digit*/
	    Int qhat = (uj==v1) ? (BIGBASE-1) : (uj*BIGBASE+uj1)/v1;
	    while (v2*qhat > (uj*BIGBASE+uj1-qhat*v1)*BIGBASE+uj2)
		qhat--;			/* and then try to improve it	   */

	    us1    = tl(ds);		/* take digit off off front of ds  */
	    tl(ds) = us;		/* and add to front of us	   */
	    us     = ds;
	    ds	   = us1;
	    nd     = isNull(ds) ? 0 : digitOf(hd(ds));	/* next digit of ds*/

#if DEBUG_BIGNUMS
Printf("To divide us (uj=%d, uj1=%d, uj2=%d): ",uj,uj1,uj2);
bigDump(us,n+1);
Printf(" by vs (v1=%d, v2=%d): ",v1,v2);
bigDump(vs,n);
Printf(", guess qhat=%d\n",qhat);
#endif
	    uj  = nd;			/* us := us - qhat * vs		   */
	    uj1 = uj2 = c = 0;
	    us1 = us;
	    vs1 = vs;
	    do {
		uj2 = uj1;
		uj1 = uj;
		uj  = digitOf(hd(us1)) - qhat*digitOf(hd(vs1)) - c;
		if (uj>=0)
		    c = 0;
		else {
		    c   = (BIGBASE - 1 - uj) / BIGBASE;
		    uj += c*BIGBASE;
		}
		hd(us1) = mkDigit(uj);
		us1     = tl(us1);
		vs1     = tl(vs1);
	    } while (nonNull(vs1));

	    if (digitOf(hd(us1))<c) {	/* maybe we overestimated qhat?	   */
#if DEBUG_BIGNUMS
Printf("overestimated qhat by one!\n");
#endif
		qhat--;			/* so reduce guess		   */
		uj  = nd;		/* and set us := us + vs	   */
		uj1 = uj2 = c = 0;	/* (we can't have overestimated by */
		us1 = us;		/* more than one thanks to the	   */
		vs1 = vs;		/* initial scaling of us, vs by sc)*/
		do {
		    uj2 = uj1;
		    uj1 = uj;
		    uj  = digitOf(hd(us1)) + digitOf(hd(vs1)) + c;
		    if (uj>=BIGBASE)
			c = 1, uj -= BIGBASE;
		    else
			c = 0;
		    hd(us1) = mkDigit(uj);
		    us1     = tl(us1);
		    vs1     = tl(vs1);
		} while (nonNull(vs1));
	    }
#if DEBUG_BIGNUMS
Printf("There remains (uj=%d, uj1=%d, uj2=%d): ",uj,uj1,uj2);
bigDump(us,n);
Putchar('\n');
#endif

	    if (nonNull(qs) || qhat)	/* output quotient digit, without  */
		qs = cons(mkDigit(qhat),qs);	/* leading zeros	   */

	    allowBreak();
	}

#if DEBUG_BIGNUMS
Printf("done quotient\n");
#endif
	/* Now we have the quotient digits (if any) with least significant
	 * digit first in qs, and sc times the remainder is the first n
	 * digits of us.  All that remains is to adjust the remainder:
	 */

	us1 = rev(take(n,us));
	us  = NIL;
	uj  = 0;			/* reuse variable uj as a carry	   */
	while (nonNull(us1)) {
	    Int y = uj * BIGBASE + digitOf(hd(us1));
	    uj    = y % sc;
	    y    /= sc;
	    if (nonNull(us) || y) {
		vs1     = tl(us1);
		tl(us1) = us;
		hd(us1) = mkDigit(y);
		us      = us1;
		us1	= vs1;
	    }
	    else
		us1 = tl(us1);
	}
	bigRem = us;
	consGC = gc;
	return qs;
    }
}

/*-------------------------------------------------------------------------*/

/* e is a constant expression with no live pointers */
#define CAFBignum(nm,e)                   \
  primCAF(nm) {                           \
    push(e);                              \
    bn = NIL;                             \
  }

/* e is an expression with free variables x and y */
#define BignumBignum2Bignum(nm,e)         \
  primFun(nm) {                           \
    Bignum x, y;                          \
    eval(primArg(2)); push(whnfHead);     \
    eval(primArg(1)); y = whnfHead;  	  \
    x = pop();                    	  \
    if (!isBignum(x) || !isBignum(y))     \
	internal("Bignum expected");      \
    updateRoot(e);                   	  \
    bn = NIL;                             \
}

/* e is an expression with free variable x */
#define Bignum2Bignum(nm,e)      	  \
  primFun(nm) {                     	  \
    Bignum x;                       	  \
    eval(primArg(1)); x = whnfHead; 	  \
    if (!isBignum(x))                     \
	internal("Bignum expected");      \
    updateRoot(e);                  	  \
    bn = NIL;                             \
}

/* e is an expression with free variable x that does no heap allocation */
#define Bignum2Bool(nm,e)                 \
  primFun(nm) {                           \
    Bignum x;                             \
    eval(primArg(1)); x = whnfHead;       \
    if (!isBignum(x))                     \
	internal("Bignum expected");      \
    updateRoot(e ? nameTrue : nameFalse); \
    bn = NIL;                             \
}

/* e is an expression with free variable x that does no heap allocation */
#define BignumBignum2Bool(nm,e)           \
  primFun(nm) {                           \
    Bignum x, y;                          \
    eval(primArg(2)); push(whnfHead);     \
    eval(primArg(1)); y = whnfHead;       \
    x = pop();                            \
    if (!isBignum(x) || !isBignum(y))     \
	internal("Bignum expected");      \
    updateRoot((e) ? nameTrue : nameFalse); \
    bn = NIL;                             \
}

/*-------------------------------------------------------------------------*/

BignumBignum2Bignum(primPlusInteger,bigAdd(x,y))  /* Integer addition	   */
BignumBignum2Bignum(primMinusInteger,bigSub(x,y)) /* Integer subtraction   */
BignumBignum2Bignum(primMulInteger,bigMul(x,y))   /* Integer multiply	   */

primFun(primQrmInteger) {		/* Integer quotient and remainder  */
    Bignum x;
    eval(primArg(2));
    primArg(2) = whnfHead;
    eval(primArg(1));
    x = bigQrm(primArg(2),primArg(1)=whnfHead);
    if (isNull(x)) {
	throwException(ap(nameArithException, nameDivideByZero));
    }
    else {
	updateRoot(x);
    }
    bn = NIL;
}

/*-------------------------------------------------------------------------*/
#endif

/* --------------------------------------------------------------------------
 * Coercion primitives:
 * ------------------------------------------------------------------------*/

Char2Int(primCharToInt,x)              /* Character to integer primitive   */

primFun(primIntToChar) {               /* Integer to character primitive   */
    Int i;
    IntArg(i,1);
    if (i<0  || i>MAXCHARVAL)
	throwException(ap(nameErrorCall, mkStr(findText("chr: out of range"))));
    CharResult(i);
}

primFun(primWordToInt) {               /* Word to integer primitive        */
    Unsigned x;
    WordArg(x,1);
    IntResult(x);
}

primFun(primIntToWord) {               /* Integer to word primitive        */
    Int i;
    IntArg(i,1);
    WordResult(i);
}

primFun(primIntToFloat) {              /* Integer to Float primitive       */
    Int i;
    IntArg(i,1);
    FloatResult((Float)i);
}

primFun(primIntToDouble) {             /* Integer to Double primitive      */
    Int i;
    IntArg(i,1);
    DoubleResult((Double)i);
}

primFun(primDummyCvt) {                /* dummy (identity) conversion      */
    updateRoot(primArg(1));
}

primFun(primRationalToFloat) {
#if SHORT_CIRCUIT_COERCIONS
    /* Optimisation: we try to short-circuit trivial conversions */
    Cell x = followInd(primArg(1));
    if (isAp(x)) {
	Cell f = followInd(fun(x));
	Cell a = arg(x);
	if (f == nameFloatToRational) {
	    updateRoot(a);
	    return;
	} else if (f == nameDoubleToRational) {
	    updapRoot(nameDoubleToFloat,a);
	    return;
	} else if (isAp(f)) {
	    Cell g = followInd(fun(f));
	    if (g == nameDoubleToRatio) {
		/* ignore the dict - it must be right */
		updapRoot(nameDoubleToFloat,a);
		return;
	    } else if (g == nameIntToRatio) {
		updapRoot(nameIntToFloat,a);
		return;
	    }
	}
    }
#endif
    updapRoot(nameRationalToFloat,primArg(1));
}

primFun(primRationalToDouble) {
#if SHORT_CIRCUIT_COERCIONS
    /* Optimisation: we try to short-circuit trivial conversions */
    Cell x = followInd(primArg(1));
    if (isAp(x)) {
	Cell f = followInd(fun(x));
	Cell a = arg(x);
	if (f == nameFloatToRational) {
	    updapRoot(nameFloatToDouble,a);
	    return;
	} else if (f == nameDoubleToRational) {
	    updateRoot(a);
	    return;
	} else if (isAp(f)) {
	    Cell g = followInd(fun(f));
	    if (g == nameDoubleToRatio) {
		updateRoot(a); /* ignore the dict - it must be right */
		return;
	    } else if (g == nameIntToRatio) {
		updapRoot(nameIntToDouble,a);
		return;
	    }
	}
    }
#endif
    updapRoot(nameRationalToDouble,primArg(1));
}

/* --------------------------------------------------------------------------
 * Float arithmetic primitives:
 * ------------------------------------------------------------------------*/

primFun(primFloatToDouble) {
    Float f;
    FloatArg(f,1);
    DoubleResult((Double)f);
} 

primFun(primDoubleToFloat) {
    Double f;
    DoubleArg(f,1);
    FloatResult((Float)f);
} 

FloatFloat2Float(primPlusFloat,x+y)    /* Float addition primitive         */
FloatFloat2Float(primMinusFloat,x-y)   /* Float subtraction primitive      */
FloatFloat2Float(primMulFloat,x*y)     /* Float multiplication primitive   */
Float2Float(primNegFloat,-x)           /* Float negation primitive         */
FloatFloat2Float(primDivFloat,x/y)     /* Float division primitive       */

DoubleDouble2Double(primPlusDouble,x+y) /* Double addition primitive        */
DoubleDouble2Double(primMinusDouble,x-y)/* Double subtraction primitive    */
DoubleDouble2Double(primMulDouble,x*y)  /* Double multiplication primitive  */
Double2Double(primNegDouble,-x)         /* Double negation primitive        */
DoubleDouble2Double(primDivDouble,x/y)  /* Double division primitive  */

Float2Float(primSinFloat,sin(x))       /* Float sin (trig) primitive       */
Float2Float(primCosFloat,cos(x))       /* Float cos (trig) primitive       */
Float2Float(primTanFloat,tan(x))       /* Float tan (trig) primitive       */
Float2Float(primAsinFloat,asin(x))     /* Float arc sin (trig) primitive   */
Float2Float(primAcosFloat,acos(x))     /* Float arc cos (trig) primitive   */
Float2Float(primAtanFloat,atan(x))     /* Float arc tan (trig) primitive   */
#if 0 /* not used in current version of Prelude */
FloatFloat2Float(primAtan2Float,atan2(x,y)) /* Float arc tan with quadrant info*/
#endif
				       /*               (trig) primitive   */
Float2Float(primExpFloat,exp(x))       /* Float exponential primitive      */
Float2FloatPre(primLogFloat,log(x),x>0)/* Float logarithm primitive        */
Float2FloatPre(primSqrtFloat,sqrt(x),x>=0) /* Float square root primitive  */

Double2Double(primSinDouble,sin(x))    /* Double sin (trig) primitive      */
Double2Double(primCosDouble,cos(x))    /* Double cos (trig) primitive      */
Double2Double(primTanDouble,tan(x))    /* Double tan (trig) primitive      */
Double2Double(primAsinDouble,asin(x))  /* Double arc sin (trig) primitive  */
Double2Double(primAcosDouble,acos(x))  /* Double arc cos (trig) primitive  */
Double2Double(primAtanDouble,atan(x))  /* Double arc tan (trig) primitive  */
Double2Double(primExpDouble,exp(x))    /* Double exponential primitive     */
Double2DoublePre(primLogDouble,log(x),x>0)/* Double logarithm primitive    */
Double2DoublePre(primSqrtDouble,sqrt(x),x>=0) /* Double square root primitive */

#if 0 /* This was in Hugs 1.01 - not needed by prelude */
Float2FloatPre(primLog10Float,log10(x),x>0) /* Float logarithm (base 10) prim*/
#endif
/* Not used in Hugs prelude, rounds towards zero */
Float2Int(primFloatToInt,(Int) x)      /* Adhoc Float --> Int conversion   */
Double2Int(primDoubleToInt,(Int) x)    /* Adhoc Double --> Int conversion  */

#if BIGNUMS
CAFBignum(primFloatRadix,bigInt(HUGS_FLT_RADIX)) /* Float radix primitive  */
#else                                    
CAFInt(primFloatRadix,HUGS_FLT_RADIX)  /* from K&R2, I hope it's portable  */
#endif

CAFInt(primFloatDigits,HUGS_FLT_MANT_DIG)/* Float sig. digits primitive    */
				       /* again, courtesy K&R2             */

CAFInt(primFloatMinExp,HUGS_FLT_MIN_EXP)/* Float min exponent primitive    */
CAFInt(primFloatMaxExp,HUGS_FLT_MAX_EXP)/* Float max exponent primitive    */

CAFInt(primDoubleDigits,HUGS_DBL_MANT_DIG)/* Double sig. digits primitive  */
				       /* again, courtesy K&R2             */

CAFInt(primDoubleMinExp,HUGS_DBL_MIN_EXP)/* Double min exponent primitive  */
CAFInt(primDoubleMaxExp,HUGS_DBL_MAX_EXP)/* Double max exponent primitive  */

/* ToDo: GHC stole its decode code from Lennart - maybe we should too?     */
primFun(primFloatDecode) {             /* Float decode primitive           */
    double f;                          /*  :: Float -> (Integer,Int)       */
    Int    n;                          /* another gruesome hack            */
    FloatArg(f,1);
    f  = frexp((double)(f),&n);        /* 0.5   <= f < 1                   */
    f  = ldexp(f,HUGS_FLT_MANT_DIG);   /* 2^m-1 <= f < 2^m, m=HUGS_FLT_MANT_DIG*/
    n -= HUGS_FLT_MANT_DIG;
#if BIGNUMS
    push(bigDouble(f));
    updapRoot(ap(mkTuple(2),top()),mkInt(n));
#else
    push(mkInt((Int)f));
    updapRoot(ap(mkTuple(2),top()),mkInt(n));
#endif
}

primFun(primFloatEncode) {             /* Float encode primitive           */
    Int n;                             /*  :: Integer -> Int -> Float      */
    Float f;                           /* Ugly hack, don't use Hugs for    */
    IntArg(n,1);                       /* numerical work                   */
    eval(primArg(2));                  /* get integer                      */
#if DJGPP2                                     
    _fpreset();                        /* Get round a possible DJGPP bug?  */
#endif                                         
#if BIGNUMS                                    
    f = (Float)bigToDouble(whnfHead);  /* and turn it into a float         */
#else                                          
    f = (Float) whnfInt;               /* and turn it into a float         */
#endif
    updateRoot(mkFloat(ldexp(f,n)));
}

/* ToDo: GHC stole its decode code from Lennart - maybe we should too?     */
primFun(primDoubleDecode) {            /* Double decode primitive           */
    double f;                          /*  :: Double -> (Integer,Int)       */
    Int    n;                          /* another gruesome hack            */
    DoubleArg(f,1);
    f  = frexp((double)(f),&n);        /* 0.5   <= f < 1                   */
    f  = ldexp(f,HUGS_DBL_MANT_DIG);   /* 2^m-1 <= f < 2^m, m=HUGS_DBL_MANT_DIG*/
    n -= HUGS_DBL_MANT_DIG;
#if BIGNUMS
    push(bigDouble(f));
    updapRoot(ap(mkTuple(2),top()),mkInt(n));
#else
    push(mkInt((Int)f));
    updapRoot(ap(mkTuple(2),top()),mkInt(n));
#endif
}

primFun(primDoubleEncode) {            /* Double encode primitive          */
    Int n;                             /*  :: Integer -> Int -> Double     */
    Double f;                          /* Ugly hack, don't use Hugs for    */
    IntArg(n,1);                       /* numerical work                   */
    eval(primArg(2));                  /* get integer                      */
#if DJGPP2                                     
    _fpreset();                        /* Get round a possible DJGPP bug?  */
#endif                                         
#if BIGNUMS                                    
    f = (Double)bigToDouble(whnfHead); /* and turn it into a double        */
#else                                          
    f = (Double)whnfInt;               /* and turn it into a double        */
#endif
    DoubleResult(ldexp(f,n));
}

/* --------------------------------------------------------------------------
 * Ptr primitives:
 * ------------------------------------------------------------------------*/

CAFPtr(primNullPtr,0)                  /* Null pointer                     */
PtrInt2Ptr(primPlusPtr,(char*)x+y)     /* Pointer arithmetic               */
PtrInt2Ptr(primAlignPtr,(char*)x+(int)((y - (long)x%y)%y))
				       /* Aligning the pointer             */
PtrPtr2Int(primMinusPtr,(char*)x-(char*)y) /* Pointer arithmetic           */
PtrPtr2Bool(primEqPtr,x==y)            /* Pointer equality primitive       */

primFun(primCmpPtr) {                  /* Pointer compare primitive        */
    Pointer x, y;
    PtrArg(x,2);
    PtrArg(y,1);
    updateRoot( x<y ? nameLT :
	      ( x>y ? nameGT : 
		      nameEQ ));
}

/* --------------------------------------------------------------------------
 * Comparison primitives:
 * ------------------------------------------------------------------------*/

IntInt2Bool(primEqInt,x==y)            /* Integer equality primitive       */
WordWord2Bool(primEqWord,x==y)         /* Natural equality primitive       */
CharChar2Bool(primEqChar,x==y)         /* Character equality primitive     */
FloatFloat2Bool(primEqFloat, x==y)     /* Float equality primitive         */
DoubleDouble2Bool(primEqDouble, x==y)  /* Double equality primitive        */

primFun(primCmpInt) {                  /* Integer compare primitive        */
    Int x, y;
    IntArg(x,2);
    IntArg(y,1);
    updateRoot( x<y ? nameLT :
	      ( x>y ? nameGT : 
		      nameEQ ));
}

primFun(primCmpWord) {                 /* Natural compare primitive        */
    Unsigned x, y;
    WordArg(x,2);
    WordArg(y,1);
    updateRoot( x<y ? nameLT :
	      ( x>y ? nameGT : 
		      nameEQ ));
}

primFun(primCmpChar) {                 /* Character compare primitive      */
    Char x, y;
    CharArg(x,2);
    CharArg(y,1);
    updateRoot( x<y ? nameLT :
	      ( x>y ? nameGT : 
		      nameEQ ));
}

primFun(primCmpFloat) {                /* Float compare primitive          */
    Float x, y;
    FloatArg(x,2);
    FloatArg(y,1);
    updateRoot( x<y ? nameLT :
	      ( x>y ? nameGT : 
		      nameEQ ));
}

primFun(primCmpDouble) {               /* Double compare primitive         */
    Double x, y;
    DoubleArg(x,2);
    DoubleArg(y,1);
    updateRoot( x<y ? nameLT :
	      ( x>y ? nameGT : 
		      nameEQ ));
}

/* --------------------------------------------------------------------------
 * Print primitives:
 * ------------------------------------------------------------------------*/

/* --------------------------------------------------------------------------
 * Builtin printer, used as an alternative to overloaded "show", and also
 * used for certain primitive types.
 *
 * The Hugs 98 system is Copyright (c) Mark P Jones, Alastair Reid, the
 * Yale Haskell Group, and the OGI School of Science & Engineering at OHSU,
 * 1994-2003, All rights reserved.  It is distributed as free software under
 * the license in the file "License", which is included in the distribution.
 *
 * $RCSfile: printer.c,v $
 * $Revision: 1.15 $
 * $Date: 2006/09/05 22:43:44 $
 * ------------------------------------------------------------------------*/

static Void   local printer		(Name,Int);
static Void   local outName   		(Name);
static Void   local outVar		(Name);
static Void   local outOp		(Name);
static Void   local outStr		(String);
static Void   local outPr		(Name,Int,Cell);
static Void   local outLPr		(Name,Cell);
static Void   local outException	(Cell);
static Void   local outBadRedex		(Cell);
static Cell   local printDException	(Cell);
static Cell   local printException	(Cell,Cell);

static Name nameLPrint, nameNLPrint;	/* list printing primitives	   */
static Name nameSPrint, nameNSPrint;	/* string printing primitives	   */

static Cell out;			/* GC'd var used by printer code   */

#define outCh(c)	out = ap(consChar(c),out)
#define outSCh(c)	outStr(unlexChar(c,'\"'))
#define updOutRoot(ss)  out = revOnto(out,ss);		\
			updapRoot(fst(out),snd(out));	\
			out = NIL;

/* --------------------------------------------------------------------------
 * Printer control:
 * ------------------------------------------------------------------------*/

static Void printerControl (Int);
static Void printerControl(what)
Int what; {
    switch (what) {
	case MARK    : mark(out);
		       break;
	case INSTALL : 
		       setCurrModule(modulePrelude);
#define pFun(n,s,t)    addPrim(0,n=newName(findText(s),NIL),t,modulePrelude,NIL)
		       pFun(namePrint,	   "_print",   "print");
		       pFun(nameNPrint,	   "_nprint",  "nprint");
		       pFun(nameLPrint,	   "_lprint",  "lprint");
		       pFun(nameNLPrint,   "_nlprint", "nlprint");
		       pFun(nameSPrint,	   "_sprint",  "sprint");
		       pFun(nameNSPrint,   "_nsprint", "nsprint");
#undef pFun
		       /* deliberate fall through */
	case RESET   : out = NIL;
		       break;
    }
}

PROTO_PRIM(primPrint);
PROTO_PRIM(primBPrint);
PROTO_PRIM(primNPrint);
PROTO_PRIM(primLPrint);
PROTO_PRIM(primNLPrint);
PROTO_PRIM(primSPrint);
PROTO_PRIM(primNSPrint);

static struct primitive printerPrimTable[] = {
  {"print",		3, primPrint},
  {"nprint",		3, primNPrint},
  {"lprint",		2, primLPrint},
  {"nlprint",		2, primNLPrint},
  {"sprint",		2, primSPrint},
  {"nsprint",		2, primNSPrint},
  {"primShowsInt",	3, primBPrint},
  {"primShowsPtr",	3, primBPrint},
  {"primShowsInteger",	3, primBPrint},
  {"primShowsFloat",	3, primBPrint},
  {"primShowsDouble",	3, primBPrint},
  {0,			0, 0}
};

static struct primInfo printerPrims = { printerControl, printerPrimTable, 0 };

/*-------------------------------------------------------------------------*/

primFun(primPrint) {			/* Evaluate and print term	   */
    Cell temp;				/*    :: Int->Expr->[Char]->[Char] */
    Int  d;
    eval(primArg(3));
    d    = whnfInt;
    temp = evalWithNoError(primArg(2));
    out  = NIL;
    if (nonNull(temp)) {
	push(temp);
	outException(top());
    }
    else
	printer(namePrint,d);
    updOutRoot(primArg(1));
}

primFun(primBPrint) {			/* Eval and print value of basic   */
    Int d;				/* type -- Int, Integer, Float, or */
    eval(primArg(3));			/* Double -- as a string.	   */
    d   = whnfInt;
    eval(primArg(2));			/* Differs from primPrint only in  */
    out = NIL;				/* its use of weaker error handling*/
    printer(namePrint,d);		/* to make showsPrec strict.	   */
    updOutRoot(primArg(1));
}

primFun(primNPrint) {			/* print term without evaluation   */
    Int  d;				/*     :: Int->Expr->[Char]->[Char] */
    eval(primArg(3));
    d   = whnfInt;
    unwind(primArg(2));
    out = NIL;
    printer(nameNPrint,d);
    updOutRoot(primArg(1));
}

static Void local printer(pr,d)		/* Main part: primPrint/primNPrint */
Name pr;				/* printer to use on components	   */
Int  d; {				/* precedence level		   */
    Int used = 0;			/* Output, in reverse, to "out"	   */

    allowBreak();

    switch(whatIs(whnfHead)) {

	case NAME     : {   Syntax sy = syntaxOf(whnfHead);

			    if (!isCfun(whnfHead) ||
				    name(whnfHead).arity>whnfArgs)
				pr = nameNPrint;

#ifndef VERBOSE_PRINT
			    /* don't print dictionary arguments */
			    while (used<whnfArgs) {
				Cell h;
				Int w;
				h=pushed(used);
				for (;;) {
				    w = whatIs(h);
				    if (w==AP)
					h = fun(h);
				    else if (w==INDIRECT)
					h = arg(h);
				    else
					break;
				}
				if (w==NAME && isNull(name(h).type) &&
				    (whatIs(name(h).parent)==INSTANCE ||
				     isDfun(h) ||
				     strncmp(textToStr(name(h).text),"Make.",5) == 0))
				    used++;
				else
				    break;
			    }
#endif

			    if (whnfHead==nameCons && whnfArgs==2) {/*list */
				StackPtr ksp = sp;
				used 	     = 2;
				if (pr==namePrint) {
				    Cell temp = evalWithNoError(top());
				    if (nonNull(temp)) {
					push(temp);
					outCh('[');
					outException(top());
					sp = ksp;
					outLPr(nameLPrint,pushed(1));
				    }
				    else if (isChar(whnfHead) && whnfArgs==0) {
					outCh('"');
					outSCh(charOf(whnfHead));
					outLPr(nameSPrint,pushed(1));
				    }
				    else {
					sp = ksp;
					outCh('[');
					outPr(namePrint,MIN_PREC,top());
					outLPr(nameLPrint,pushed(1));
				    }
				}
				else {
				    unwind(top());
				    if (isChar(whnfHead) && whnfArgs==0) {
					outCh('"');
					outSCh(charOf(whnfHead));
					outLPr(nameNSPrint,pushed(1));
				    }
				    else {
					sp = ksp;
					outCh('[');
					outPr(nameNPrint,MIN_PREC,top());
					outLPr(nameNLPrint,pushed(1));
				    }
				}
				return;
			    }
#ifndef VERBOSE_PRINT
			    /* hide fromInt/fromDouble */
			    else if (whnfArgs==2 &&
				     ((whnfHead==nameFromInt && whatIs(pushed(1))==INTCELL) ||
				      (whnfHead==nameFromDouble && (whatIs(pushed(1))==FLOATCELL || whatIs(pushed(1))==DOUBLECELL)))) {
				unwind(pushed(1));
				printer(pr,d);
				return;
			    }
#endif
			    else if (whnfArgs-used==1 && sy!=APPLIC) { /* (e1+) */
				outCh('(');
				outPr(pr,FUN_PREC-1,pushed(used));
				outCh(' ');
				outOp(whnfHead);
				outCh(')');
				used++;
			    }
			    else if (whnfArgs-used>=2 && sy!=APPLIC) { /* e1+e2 */
				Syntax a = assocOf(sy);
				Int    p = precOf(sy);
				if (whnfArgs-used>2 || d>p)
				     outCh('(');
				outPr(pr,(a==LEFT_ASS? p:1+p),pushed(used));
				used++;
				outCh(' ');
				outOp(whnfHead);
				outCh(' ');
				outPr(pr,(a==RIGHT_ASS?p:1+p),pushed(used));
				used++;
				if (whnfArgs-used>2 || d>p)
				    outCh(')');
			    }
			    else				  /* f ... */
				outVar(whnfHead);
			}
			break;

#if BIGNUMS
	case NEGNUM   :
	case ZERONUM  :
	case POSNUM   : out = rev(bigOut(whnfHead,NIL,d>=UMINUS_PREC));
			pr  = nameNPrint;
			break;
#endif

	case INTCELL  : {   Int digit;

			    if (intOf(whnfHead)<0 && d>=UMINUS_PREC)
				outCh(')');

			    do {
				digit = whnfInt%10;
				if (digit<0)
				    digit= (-digit);
				outCh('0'+digit);
			    } while ((whnfInt/=10)!=0);

			    if (intOf(whnfHead)<0) {
				outCh('-');
				if (d>=UMINUS_PREC)
				    outCh('(');
			    }

			    out = rev(out);
			    pr  = nameNPrint;
			}
			break;

	case PTRCELL  : {   Pointer p = ptrOf(whnfHead);
			    char buffer[32];
			    char spec[16];
			    /* Fall into line with how GHC shows Addrs  */
			    sprintf(spec,"0x%%.%dx", (SIZEOF_INTP)*2);
			    sprintf(buffer,spec,(long)p);
#if 0
			    /* Old skool */
			    sprintf(buffer,"%p",p);
#endif
			    outStr(buffer);
			    pr  = nameNPrint;
			}
			break;

	case TUPLE    : {   Int tn   = tupleOf(whnfHead);
			    Int punc = '(';
			    Int i;

			    used     = tn<whnfArgs ? tn : whnfArgs;
			    for (i=0; i<used; ++i) {
				outCh(punc);
				outPr(pr,MIN_PREC,pushed(i));
				punc = ',';
			    }
			    for (; i<tn; ++i) {
				outCh(punc);
				punc = ',';
			    }
			    outCh(')');
			}
			pr = nameNPrint;
			break;

	case CHARCELL : outCh('\'');
			outStr(unlexChar(charOf(whnfHead), '\''));
			outCh('\'');
			pr = nameNPrint;
			break;

	case FLOATCELL: if (whnfFloat<0.0 && d>=UMINUS_PREC)
			    outCh('(');
			outStr(floatToString(whnfFloat));
			if (whnfFloat<0.0 && d>=UMINUS_PREC)
			    outCh(')');
			pr = nameNPrint;
			break;

	case DOUBLECELL:if (whnfDouble<0.0 && d>=UMINUS_PREC)
			    outCh('(');
			outStr(doubleToString(whnfDouble));
			if (whnfDouble<0.0 && d>=UMINUS_PREC)
			    outCh(')');
			pr = nameNPrint;
			break;

#if HASKELL_ARRAYS
	case ARRAY    : outStr("{array}");
			pr = nameNPrint;
			break;
#endif

#if IO_REFS
	case MUTVAR   : outStr("{mutable variable}");
			pr = nameNPrint;
			break;
#endif

	case HUGSOBJECT: 
			outStr("{Cell ...}");
			pr = nameNPrint;
			break;

#if TREX
	case RECORD   : outStr("{record}");
			pr = nameNPrint;
			break;
#endif
 
#if IO_HANDLES
	case HANDCELL : outStr("{handle}");
			pr = nameNPrint;
			break;
#endif

#if OBSERVATIONS
        case OBSERVE  : outStr("printer.c: printer(): OBSERVE");
                        break;
#endif

	default       : internal("Error in graph");
			break;
    }

    if (used<whnfArgs) {		/* Add remaining args to output	   */
	do {
	    outCh(' ');
	    outPr(pr,FUN_PREC,pushed(used));
	} while (++used<whnfArgs);

	if (d>=FUN_PREC) {		/* Determine if parens are needed  */
	    out = appendOnto(out,singleton(consChar('(')));
	    outCh(')');
	}
    }
}

/* --------------------------------------------------------------------------
 * List printing primitives:
 * ------------------------------------------------------------------------*/

primFun(primLPrint) {			/* evaluate and print list	   */
    Cell temp = evalWithNoError(primArg(2));
    out       = NIL;
    if (nonNull(temp)) {
	push(temp);
	outStr("] ++ ");
	outException(top());
    }
    else if (whnfHead==nameCons && whnfArgs==2) {
	outCh(',');
	outPr(namePrint,MIN_PREC,pushed(0));
	outLPr(nameLPrint,pushed(1));
    }
    else if (whnfHead==nameNil && whnfArgs==0)
	outCh(']');
    else {
	outStr("] ++ ");
	outBadRedex(primArg(2));
    }
    updOutRoot(primArg(1));
}

primFun(primNLPrint) {			/* print list without evaluation   */
    unwind(primArg(2));
    out = NIL;
    if (whnfHead==nameCons && whnfArgs==2) {
	outCh(',');
	outPr(nameNPrint,MIN_PREC,pushed(0));
	outLPr(nameNLPrint,pushed(1));
    }
    else if (whnfHead==nameNil && whnfArgs==0)
	outCh(']');
    else {
	outStr("] ++ ");
	outPr(nameNPrint,FUN_PREC-1,primArg(2));
    }
    updOutRoot(primArg(1));
}

primFun(primSPrint) {			/* evaluate and print string	   */
    Cell temp = evalWithNoError(primArg(2));
    out       = NIL;
    if (nonNull(temp)) {
	push(temp);
	outStr("\" ++ ");
	outException(top());
    }
    else if (whnfHead==nameCons && whnfArgs==2) {
	temp = evalWithNoError(top());	/* primArg(4), primArg(3) contain  */
	out  = NIL;			/* the head and tail of list, resp */
	if (nonNull(temp)) {
	    push(temp);
	    outStr("\" ++ [");
	    outException(top());
	    outLPr(nameLPrint,primArg(3));
	}
	else if (isChar(whnfHead) && whnfArgs==0) {
	    outSCh(charOf(whnfHead));
	    outLPr(nameSPrint,primArg(3));
	}
	else {
	    outStr("\" ++ [");
	    outBadRedex(primArg(4));
	    outLPr(nameLPrint,primArg(3));
	}
    }
    else if (whnfHead==nameNil && whnfArgs==0)
	outCh('"');
    else {
	outStr("\" ++ ");
	outBadRedex(primArg(2));
    }
    updOutRoot(primArg(1));
}

primFun(primNSPrint) {			/* print string without eval	   */
    unwind(primArg(2));
    out = NIL;
    if (whnfHead==nameCons && whnfArgs==2) {
	unwind(pushed(0));
	if (isChar(whnfHead) && whnfArgs==0) {
	    outSCh(charOf(whnfHead));
	    outLPr(nameNSPrint,primArg(3));
	}
	else {
	    outStr("\" ++ [");
	    outPr(nameNPrint,MIN_PREC,primArg(4));
	    outLPr(nameNLPrint,primArg(3));
	}
    }
    else if (whnfHead==nameNil && whnfArgs==0)
	outCh('"');
    else {
	outStr("\" ++ ");
	outPr(nameNPrint,FUN_PREC-1,primArg(2));
    }
    updOutRoot(primArg(1));
}

/* --------------------------------------------------------------------------
 * Auxiliary functions for printer(s):
 * ------------------------------------------------------------------------*/

static Void local outName(nm)  /* output nm using parent field if possible */
Name nm; {
    Cell p = name(nm).parent;
    switch (whatIs(p)) {
#ifdef VERBOSE_PRINT
	case INSTANCE : outStr("inst");
			outStr(textToStr(cclass(inst(p).c).text));
			outCh('_');
			break;

	case CLASS    : outStr(textToStr(cclass(p).text));
			outCh('_');
			break;

	case TYCON    : outStr(textToStr(tycon(p).text));
			outCh('_');
			break;
#endif

	case NAME     : outName(p);
			outCh('_');
			break;
    }
    outStr(textToStr(name(nm).text));
}

static Void local outVar(nm)		/* output nm as function symbol	   */
Name nm; {
    String s = textToStr(name(nm).text);
    if ((isascii(*s) && isalpha(*s)) || *s=='_' || *s=='[' || *s=='(')
	outName(nm);
    else {
	outCh('(');
	outName(nm);
	outCh(')');
    }
}

static Void local outOp(nm)		/* output nm as operator symbol	   */
Name nm; {
    String s = textToStr(name(nm).text);
    if (isascii(s[0]) && isalpha(s[0])) {
	outCh('`');
	outName(nm);
	outCh('`');
    }
    else
	outName(nm);
}

static Void local outStr(s)		/* output string s		   */
String s; {
    while (*s)
	outCh(ExtractChar(s));
}

static Void local outPr(pr,d,e)		/* output expr e with printer pr,  */
Name pr;				/* precedence d			   */
Int  d;
Cell e; {
    out           = ap(NIL,out);
    fst(out)      = ap(NIL,e);
    fst(fst(out)) = ap(pr,mkInt(d));
}

static Void local outLPr(pr,xs)		/* output list xs with printer pr  */
Name pr;
Cell xs; {
    out      = ap(NIL,out);
    fst(out) = ap(pr,xs);
}

static Void local outException(ex)	/* Produce expr to print exception */
Cell ex; {
    outCh('{');
    if (isAp(ex) && fun(ex)==nameErrorCall) {
	outStr("error ");
	outPr(nameNPrint,FUN_PREC,arg(ex));
    } else {
	outStr("throw ");
	outPr(nameNPrint,FUN_PREC,ex);
    }
    outCh('}');
}

static Void local outBadRedex(rx)	/* Produce expr to print bad redex */
Cell rx; {
    outCh('{');
    outPr(nameNPrint,MIN_PREC,rx);
    outCh('}');
}

static Cell local printDException(ex)	/* Produce expression for exception*/
Cell ex; {				/* with special handling	   */
    if (isAp(ex) && fun(ex)==nameErrorCall)/* of {error str} exceptions    */
	return arg(ex);
    else
	return printException(ex,nameNil);
}

static Cell local printException(ex,rs)	/* produce expression for exception*/
Cell ex, rs; {
   out = NIL;
   outException(ex);
   return revOnto(out,rs);
}

Void abandon(what,ex)			/* abandon computation		   */
String what;
Cell   ex; {
    push(printDException(ex));
    out   = NIL;
    outCh('\n');
    outStr(what);
    outStr(" error: ");
    top() = revOnto(out,top());
    out   = NIL;
    outputString(errorStream);
    errAbort();
}

/*-------------------------------------------------------------------------*/


/* --------------------------------------------------------------------------
 * Evaluate name, obtaining a C string from a Hugs string:
 * ------------------------------------------------------------------------*/

#if FILENAME_MAX < 1024
# define MAX_STRING 1024
#else
# define MAX_STRING FILENAME_MAX
#endif

String evalName(es)                     /* evaluate es :: [Char] and save  */
Cell es; {                              /* in char array... return ptr to  */
    static char buffer[MAX_STRING+1];   /* string or 0, if error occurs    */
    char	*bp = buffer;
    StackPtr    saveSp = sp;

    eval(es);
    while (whnfHead==nameCons && bp<=buffer+MAX_STRING-MAX_CHAR_ENCODING) {
	eval(pop());
	AddChar(charOf(whnfHead), bp);
	eval(pop());
    }
    if (whnfHead==nameNil) {
	*bp = '\0';
	return buffer;
    }
    sp = saveSp;                        /* stack pointer must be the same  */
    return 0;                           /* as it was on entry              */
}

/* --------------------------------------------------------------------------
 * Top-level printing mechanism:
 * ------------------------------------------------------------------------*/

Void outputString(fp)                   /* Evaluate string on top of stack */
FILE *fp; {                             /* and print it on fp              */
    StackPtr origSp = sp;
    for (;;) {
	Cell temp = evalWithNoError(pop());
	if (nonNull(temp)) {
	    sp = origSp;
	    top()  = printException((top()=temp),nameNil);
	}
	else if (whnfHead==nameCons) {
	    if (nonNull(temp=evalWithNoError(pop()))) {
		sp = origSp;
		onto(temp);
		pushed(1) = printException(pushed(0),pushed(1));
		drop();
	    }
	    else {
		FPutChar(charOf(whnfHead),fp);
		fflush(fp);
	    }
	}
	else
	    return;
    }
}

/* --------------------------------------------------------------------------
 * IO monad implementation
 * ------------------------------------------------------------------------*/

#if IO_MONAD
/* --------------------------------------------------------------------------
 * Implementation of the Haskell IO monad.
 *
 * The primitives below implement the standard IO monad for Haskell 1.3
 * using a continuation passing monad for sequencing.  The primitives are
 * believed to give a reasonably good implementation of the semantics
 * specified by the Haskell 1.3 report.  There are also some additional
 * primitives, particularly for dealing with IOError and Handle values
 * that are not included in the prelude, but are used by standard libraries.
 *
 * The Hugs 98 system is Copyright (c) Mark P Jones, Alastair Reid, the
 * Yale Haskell Group, and the OGI School of Science & Engineering at OHSU,
 * 1994-2003, All rights reserved.  It is distributed as free software under
 * the license in the file "License", which is included in the distribution.
 *
 * $RCSfile: iomonad.c,v $
 * $Revision: 1.101 $
 * $Date: 2006/05/03 09:10:40 $
 * ------------------------------------------------------------------------*/
 
Name nameIORun;			        /* run IO code                     */
Name nameIOBind;		        /* bind IO code                    */
Name namePutStr;		        /* Prelude.putStr                  */

static Name namePass;			/* auxiliary:: \f b a -> f a b     */
#if IO_HANDLES
static Name nameHreader;	        /* auxiliary function		   */
#endif
static Cell hugsProgName;		/* value of getProgName            */
static Cell hugsArgs;			/* value of getArgs                */

#if IO_HANDLES
static Void   local throwErrno     (String,Bool,Int,Cell *);
static String local toIOErrorDescr (int,Bool);
static Name   local toIOError      (int);
static Int    local newHandle      (Cell *,String);
static String local modeString     (Int,Bool);
static Cell   local openHandle     (StackPtr,Cell *,Int,Bool,String);
static Cell   local openFdHandle   (StackPtr,Int,Int,Bool,String);
static Char   local hGetChar       (Int,String);
static Void   local hPutChar       (Char,Int,String);
static Void   local setRWState     (Int,Int);
static Void   local checkOpen      (Int,String);
static Void   local checkReadable  (Int,String);
static Void   local checkWritable  (Int,String);
#endif

#if IO_HANDLES
# if WANT_FIXED_SIZE_TABLES
#  define MAX_HANDLES NUM_HANDLES
# else
#  define MAX_HANDLES num_handles
# endif
#endif

extern Void local pushString       (String);

/* --------------------------------------------------------------------------
 * IO monad control:
 * ------------------------------------------------------------------------*/

static Void iomonadControl (Int);
static Void iomonadControl(what)
Int what; {
    switch (what) {
	case INSTALL : 
		       setCurrModule(modulePrelude);
#define pFun(n,s,t)    addPrim(0,n=newName(findText(s),NIL),t,modulePrelude,NIL)
		       pFun(namePass,	 "_pass",    "passIO");
#if    IO_HANDLES
		       pFun(nameHreader, "_hreader", "hreader");
#endif
#undef pFun
#define predef(nm,str) nm=newName(findText(str),NIL); name(nm).defn=PREDEFINED
		       predef(nameIORun,    "hugsIORun");
		       predef(nameIOBind,   "primbindIO");
		       predef(namePutStr,   "putStr");
#undef predef
		       break;

	case MARK    : mark(hugsProgName);
		       mark(hugsArgs);
		       break;
    }
}

PROTO_PRIM(primReturnIO);
PROTO_PRIM(primBindIO);
PROTO_PRIM(primPass);

PROTO_PRIM(primGC);
PROTO_PRIM(primGetEnv);
PROTO_PRIM(primSystem);
PROTO_PRIM(primGetRandomSeed);

PROTO_PRIM(primGetProgName);
PROTO_PRIM(primGetArgs);
PROTO_PRIM(primSetProgName);
PROTO_PRIM(primSetArgs);

#if IO_HANDLES
static Void local fopenPrim   (StackPtr,Bool,String);
static int local getIOMode    (Cell);

PROTO_PRIM(primHGetChar);
PROTO_PRIM(primHPutChar);
PROTO_PRIM(primHPutStr);
PROTO_PRIM(primHreader);
PROTO_PRIM(primHContents);
PROTO_PRIM(primOpenFile);
PROTO_PRIM(primOpenBinaryFile);
PROTO_PRIM(primStdin);
PROTO_PRIM(primStdout);
PROTO_PRIM(primStderr);
PROTO_PRIM(primOpenFd);
PROTO_PRIM(primHandleToFd);
PROTO_PRIM(primHIsEOF);
PROTO_PRIM(primHFlush);
PROTO_PRIM(primHClose);
PROTO_PRIM(primHGetPosn);
PROTO_PRIM(primHSetPosn);
PROTO_PRIM(primHSetBuffering);
PROTO_PRIM(primHGetBuffering);
PROTO_PRIM(primHSeek);
PROTO_PRIM(primHLookAhead);
PROTO_PRIM(primHIsOpen);
PROTO_PRIM(primHIsClosed);
PROTO_PRIM(primHIsReadable);
PROTO_PRIM(primHIsWritable);
PROTO_PRIM(primHIsSeekable);
PROTO_PRIM(primHFileSize);
PROTO_PRIM(primHWaitForInput);
PROTO_PRIM(primEqHandle);
PROTO_PRIM(primGetHandleNumber);

PROTO_PRIM(primHSetBinaryMode);
PROTO_PRIM(primHPutBuf);
PROTO_PRIM(primHGetBuf);

PROTO_PRIM(primHIsTerminalDevice);
PROTO_PRIM(primHGetEcho);
PROTO_PRIM(primHSetEcho);

PROTO_PRIM(primIOEql);
PROTO_PRIM(primIOHash);
#endif

#if IO_REFS
PROTO_PRIM(primNewRef);
PROTO_PRIM(primDerefRef);
PROTO_PRIM(primAssignRef);
PROTO_PRIM(primEqRef);
#endif

PROTO_PRIM(primMakeSP);
PROTO_PRIM(primDerefSP);
PROTO_PRIM(primFreeSP);
PROTO_PRIM(primCastSPToP);
PROTO_PRIM(primCastPToSP);

PROTO_PRIM(primNewFP);
PROTO_PRIM(primAddFPF);
PROTO_PRIM(primAddFPFEnv);
PROTO_PRIM(primWriteFP);
PROTO_PRIM(primEqFP);
PROTO_PRIM(primTouchFP);
PROTO_PRIM(primFPToP);

#if GC_WEAKPTRS
PROTO_PRIM(primWeakPtrEq);
PROTO_PRIM(primMkWeak);
PROTO_PRIM(primDeRefWeak);
PROTO_PRIM(primReplaceFinalizer);
PROTO_PRIM(primFinalize);
PROTO_PRIM(primRunFinalizer);
PROTO_PRIM(primFinalizerWaiting);
#endif

#if STABLE_NAMES
PROTO_PRIM(primMakeSN);
PROTO_PRIM(primDerefSN);
PROTO_PRIM(primHashSN);
PROTO_PRIM(primEqSN);
#endif

#ifdef HSCRIPT
PROTO_PRIM(primGetCurrentScript);
#endif

#ifdef DOTNET
/* These primops are remnants from the first attempt at
 * providing .NET interop for Haskell / Hugs. They've been
 * mostly superceeded by the integration of .NET interop
 * with the Haskell FFI, but we'll keep these primops around
 * for a little bit longer.
 */
EXT_PROTO_PRIM(primCreateObject);
EXT_PROTO_PRIM(primInvokeMethod);
EXT_PROTO_PRIM(primInvokeStaticMethod);
EXT_PROTO_PRIM(primNewString);
EXT_PROTO_PRIM(primToHsString);
EXT_PROTO_PRIM(primNewArgArray);
EXT_PROTO_PRIM(primSetArg);
EXT_PROTO_PRIM(primGetArg);
EXT_PROTO_PRIM(primGetField);
EXT_PROTO_PRIM(primSetField);
EXT_PROTO_PRIM(primGetStaticField);
EXT_PROTO_PRIM(primSetStaticField);
EXT_PROTO_PRIM(primIsNullPtr);
EXT_PROTO_PRIM(primMkPrimVector);
#endif

static struct primitive iomonadPrimTable[] = {
  {"primretIO",		1+IOArity, primReturnIO},
  {"primbindIO",	2+IOArity, primBindIO},
  {"passIO",		2+IOArity, primPass},

  {"primGC",	        0+IOArity, primGC},
  {"getEnv",	        1+IOArity, primGetEnv},
  {"primSystem",	1+IOArity, primSystem},
  {"getRandomSeed",	0+IOArity, primGetRandomSeed},

  {"primGetProgName",   0+IOArity, primGetProgName},
  {"primGetArgs",       0+IOArity, primGetArgs},
  {"primSetProgName",   1+IOArity, primSetProgName},
  {"primSetArgs",       1+IOArity, primSetArgs},

#if IO_HANDLES
  {"hGetChar",		1+IOArity, primHGetChar},
  {"hPutChar",		2+IOArity, primHPutChar},
  {"hPutStr",		2+IOArity, primHPutStr},
  {"hreader",		1, primHreader},
  {"hGetContents",	1+IOArity, primHContents},
  {"openFile",          2+IOArity, primOpenFile},
  {"openBinaryFile",    2+IOArity, primOpenBinaryFile},
  {"openFd",            4+IOArity, primOpenFd},
  {"handleToFd",	1+IOArity, primHandleToFd},
  {"stdin",		0, primStdin},
  {"stdout",		0, primStdout},
  {"stderr",		0, primStderr},
  {"hIsEOF",		1+IOArity, primHIsEOF},
  {"hFlush",		1+IOArity, primHFlush},
  {"hClose",		1+IOArity, primHClose},
  {"hGetPosnPrim",	1+IOArity, primHGetPosn},
  {"hSetPosnPrim",	2+IOArity, primHSetPosn},
  {"hSetBuff",          3+IOArity, primHSetBuffering},
  {"hGetBuff",          1+IOArity, primHGetBuffering},
  {"hSeekPrim",         3+IOArity, primHSeek},
  {"hLookAhead",        1+IOArity, primHLookAhead},
  {"hIsOpen",		1+IOArity, primHIsOpen},
  {"hIsClosed",		1+IOArity, primHIsClosed},
  {"hIsReadable",	1+IOArity, primHIsReadable},
  {"hIsWritable",	1+IOArity, primHIsWritable},
  {"hIsSeekable",       1+IOArity, primHIsSeekable},
  {"hFileSize",         1+IOArity, primHFileSize},
  {"hWaitForInput",     2+IOArity, primHWaitForInput},
  {"primEqHandle",	2, primEqHandle},
  {"primGetHandleNumber", 1, primGetHandleNumber},
  {"hSetBinaryMode",	2+IOArity, primHSetBinaryMode},
  {"hPutBuf",		3+IOArity, primHPutBuf},
  {"hGetBuf",		3+IOArity, primHGetBuf},
  {"hIsTerminalDevice",	1+IOArity, primHIsTerminalDevice},
  {"hGetEcho",		1+IOArity, primHGetEcho},
  {"hSetEcho",		2+IOArity, primHSetEcho},
#endif

#if IO_REFS
  {"newRef",            1+IOArity, primNewRef},
  {"getRef",		1+IOArity, primDerefRef},
  {"setRef",		2+IOArity, primAssignRef},
  {"eqRef",		2, primEqRef},
#endif

  {"makeStablePtr",	1+IOArity, primMakeSP},
  {"deRefStablePtr",	1+IOArity, primDerefSP},
  {"freeStablePtr",	1+IOArity, primFreeSP},
  {"castStablePtrToPtr",1, primCastSPToP},
  {"castPtrToStablePtr",1, primCastPToSP},

  {"writeForeignObj",	2+IOArity, primWriteFP},
  {"eqForeignObj",	2, primEqFP},

  {"newForeignPtr_",	1+IOArity, primNewFP},
  {"addForeignPtrFinalizer", 2+IOArity, primAddFPF},
  {"addForeignPtrFinalizerEnv", 3+IOArity, primAddFPFEnv},
  {"touchForeignPtr",	1+IOArity, primTouchFP},
  {"unsafeForeignPtrToPtr", 1, primFPToP},

#if GC_WEAKPTRS
  {"weakPtrEq",		2, primWeakPtrEq},
  {"mkWeak",		3+IOArity, primMkWeak},
  {"deRefWeak",		1+IOArity, primDeRefWeak},
  {"replaceFinalizer",	2+IOArity, primReplaceFinalizer},
  {"finalize",		1+IOArity, primFinalize},
  {"runFinalizer",	0+IOArity, primRunFinalizer},
  {"finalizerWaiting",	0+IOArity, primFinalizerWaiting},
#endif

#if STABLE_NAMES
  {"makeStableName",	1+IOArity, primMakeSN},
  {"deRefStableName",	1, primDerefSN},
  {"hashStableName",	1, primHashSN},
  {"eqStableName",	2, primEqSN},
#endif
  
#ifdef HSCRIPT
  {"getCurrentScript",  0+IOArity, primGetCurrentScript},
#endif

#ifdef DOTNET
  {"createObject",       2+IOArity, primCreateObject},
  {"invokeMethod",       3+IOArity, primInvokeMethod},
  {"invokeStaticMethod", 2+IOArity, primInvokeStaticMethod},
  {"newString",          1+IOArity, primNewString},
  {"toString",           1+IOArity, primToHsString},
  {"newArgArray",        1+IOArity, primNewArgArray},
  {"setArrayArg",        3+IOArity, primSetArg},
  {"getArrayArg",        2+IOArity, primGetArg},
  {"getField",           2+IOArity, primGetField},
  {"setField",           3+IOArity, primSetField},
  {"getStaticField",     2+IOArity, primGetStaticField},
  {"setStaticField",     3+IOArity, primSetStaticField},
  {"isNullPtr",          1+IOArity, primIsNullPtr},
  {"mkPrimVector",       2+IOArity, primMkPrimVector},
#endif
  {"IOEql",              2+IOArity, primIOEql},
  {"IOHash",             1+IOArity, primIOHash},

  {0,			0, 0}
};

static struct primInfo iomonadPrims = { iomonadControl, iomonadPrimTable, 0 };

/* --------------------------------------------------------------------------
 * The monad combinators:
 * ------------------------------------------------------------------------*/

primFun(primReturnIO) {			/* IO monad unit		   */
    IOReturn(IOArg(1));
}

primFun(primBindIO) {			/* IO monad bind		   */
    push(ap(namePass,primArg(2)));	/* bind 3 2 1 = 3 (pass 2 1)       */
    toparg(primArg(1));
    updapRoot(primArg(3),top());
}

primFun(primPass) {			/* Auxiliary function		   */
    push(ap(primArg(3),primArg(1)));	/* pass 3 2 1 = 3 1 2		   */
    updapRoot(top(),primArg(2));
}

/* --------------------------------------------------------------------------
 * Handle operations:
 * ------------------------------------------------------------------------*/

#if IO_HANDLES

static
Int local newHandle(sCell,loc) /* return a free Handle or throw an IOError */
Cell   *sCell;
String loc; {
    Int i;

    for (i=0; i<(Int)MAX_HANDLES && nonNull(handles[i].hcell); ++i)
	;                                       /* Search for unused handle*/
    if (i>=(Int)MAX_HANDLES) {                  /* If at first we don't    */
	garbageCollect();                       /* succeed, garbage collect*/
	for (i=0; i<(Int)MAX_HANDLES && nonNull(handles[i].hcell); ++i)
	    ;                                   /* and try again ...       */
#if !WANT_FIXED_SIZE_TABLES
	if (i >= (Int)MAX_HANDLES) {
	    Int j;
	    growDynTable(dynTabHandles);
	    handles = (struct strHandle*)(dynTabHandles->data);
	    num_handles = dynTabHandles->maxIdx;
	    /* Nil out the new entries in the table */
	    for (j=i; j < (Int)num_handles; j++) {
		handles[j].hcell = NIL;
	    }
	}
#endif
    }

    if (i>=(Int)MAX_HANDLES) {                  /* ... before we give up   */
	IOFail(mkIOError(NULL,
			 nameIllegal,
			 loc,
			 "too many handles open",
			 sCell));
    }

    return i;
}

static
String local modeString(hmode,binary) /* return mode string for f(d)open */
Int  hmode;
Bool binary; {
    if (binary) {
	return (hmode&HAPPEND)    ? "ab"  :
	       (hmode&HWRITE)     ? "wb"  :
	       (hmode&HREADWRITE) ? "rb+" :
	       (hmode&HREAD)      ? "rb"  : (String)0;
    } else {
	return (hmode&HAPPEND)    ? "a"   :
	       (hmode&HWRITE)     ? "w"   :
	       (hmode&HREADWRITE) ? "r+"  :
	       (hmode&HREAD)      ? "r"   : (String)0;
    }
}

static
Cell local openHandle(root,sCell,hmode,binary,loc) /* open handle to file named s in  */
StackPtr root;
Cell   *sCell;                                     /* the specified hmode  */
Int    hmode;
Bool   binary;
String loc; {
    Int i;
    String s = evalName(*sCell);
    String stmode = modeString(hmode,binary);

    /* openHandle() either returns a Handle or throws an IOError. */

    if (!s) {				/* check for valid name		   */
	IOFail(mkIOError(NULL,
			 nameIllegal,
			 loc,
			 "illegal file name",
			 NULL));
    }

    i = newHandle(sCell,loc);

    if (!stmode) {
	IOFail(mkIOError(NULL,
			 nameIllegal,
			 loc,
			 "illegal mode",
			 NULL));
    }
    /* prepare to open file    */
    handles[i].hfp = fopen(s,stmode);
    if (hmode==HREADWRITE && handles[i].hfp==NULL) /* try to create it */
	handles[i].hfp = fopen(s, binary ? "wb+" : "w+");
    if (!handles[i].hfp)
	throwErrno(loc, TRUE, NO_HANDLE, sCell);

    handles[i].hmode = hmode;
    handles[i].hbufMode = HUNKNOWN_BUFFERING;
    handles[i].hbufSize = (-1);
    setRWState(i, RW_NEUTRAL);
#if CHAR_ENCODING
    handles[i].hBinaryMode = binary;
    handles[i].hLookAhead = -1;
#endif
    return (handles[i].hcell = ap(HANDCELL,i));
}

static
Cell local openFdHandle(root,fd,hmode,binary,loc) /* open handle to file desc fd in  */
StackPtr root;
Int    fd;					  /* the specified hmode  */
Int    hmode;
Bool   binary;
String loc; {
    Int i = newHandle(NIL,loc);
    String stmode = modeString(hmode,binary);

    /* openFdHandle() either returns a Handle or throws an IOError. */

    if (!stmode) {
	IOFail(mkIOError(NULL,
			 nameIllegal,
			 loc,
			 "illegal mode",
			 NULL));
    }
    if (!(handles[i].hfp=fdopen(fd,stmode)))
	throwErrno(loc, TRUE, NO_HANDLE, NULL);

    handles[i].hmode = hmode;
    handles[i].hbufMode = HANDLE_NOTBUFFERED;
    handles[i].hbufSize = 0;
    setRWState(i, RW_NEUTRAL);
#if CHAR_ENCODING
    handles[i].hBinaryMode = binary;
    handles[i].hLookAhead = -1;
#endif
    return (handles[i].hcell = ap(HANDCELL,i));
}

static Char local hGetChar(Int h, String fname) {
    Int c;
#if CHAR_ENCODING
    if (handles[h].hLookAhead>=0) {
	c = handles[h].hLookAhead;
	handles[h].hLookAhead = -1;
    } else if (handles[h].hBinaryMode) {
	c = getc(handles[h].hfp);
    } else {
	c = FGetChar(handles[h].hfp);
    }
#else /* !CHAR_ENCODING */
    c = FGetChar(handles[h].hfp);
#endif
    if (c==EOF && !feof(handles[h].hfp))
	throwErrno(fname, TRUE, h, NULL);
#if CHAR_ENCODING
    else if (c==BAD_CHAR) {
	IOFail(mkIOError(&handles[h].hcell,
			 nameProtocolError,
			 fname,
			 "input contains non-character data - use binary I/O for binary data",
			 NULL));
    }
#endif /* CHAR_ENCODING */
    return c;
}

static Void local hPutChar(Char c, Int h, String fname) {
    Int retval;
#if CHAR_ENCODING
    retval = handles[h].hBinaryMode ? putc(c, handles[h].hfp) :
				      FPutChar(c, handles[h].hfp);
#else
    retval = FPutChar(c, handles[h].hfp);
#endif
    if (retval == EOF)
	throwErrno(fname, TRUE, h, NULL);
#if FLUSHEVERY
    if (h <= 2) { /* Only flush the standard handles */
	fflush(handles[h].hfp);
    }
#endif
}

/* If the stream is read-write, set the state, otherwise do nothing */
static Void local setRWState(Int h, Int newState) {
    if (handles[h].hmode&HREADWRITE) {
	/* ANSI C requires repositioning between writes and reads */
	if (newState==RW_READING) {
	    if (handles[h].hRWState==RW_WRITING) {
		fflush(handles[h].hfp);
		fseek(handles[h].hfp, 0L, SEEK_CUR);
	    }
	} else if (newState==RW_WRITING) {
	    if (handles[h].hRWState==RW_READING)
		fseek(handles[h].hfp, 0L, SEEK_CUR);
	}
	handles[h].hRWState = newState;
    }
}

/* ensure that the handle is neither closed nor semi-closed */
static Void local checkOpen(Int h, String fname) {
    Int mode = handles[h].hmode;
    if (mode==HCLOSED)
	IOFail(mkIOError(&handles[h].hcell,
			 nameIllegal,
		         fname,
			 "handle is closed",
			 NULL));
    if (mode==HSEMICLOSED)
	IOFail(mkIOError(&handles[h].hcell,
			 nameIllegal,
		         fname,
			 "handle is semi-closed",
			 NULL));
}

/* ensure that the handle is readable */
static Void local checkReadable(Int h, String fname) {
    checkOpen(h, fname);
    if (!(handles[h].hmode&(HREAD|HREADWRITE)))
	IOFail(mkIOError(&handles[h].hcell,
			 nameIllegal,
			 fname,
			 "handle is not readable",
			 NULL));
}

/* ensure that the handle is writable */
static Void local checkWritable(Int h, String fname) {
    checkOpen(h, fname);
    if (!(handles[h].hmode&(HWRITE|HAPPEND|HREADWRITE)))
	IOFail(mkIOError(&handles[h].hcell,
			 nameIllegal,
			 fname,
			 "handle is not writeable",
			 NULL));
}

/* --------------------------------------------------------------------------
 * Building strings:
 * ------------------------------------------------------------------------*/

Void pushString(s)       /* push pointer to string onto stack */
String s; {
    if (*s == '\0')
	push(nameNil);
    else {
	Cell l = ap(consChar(ExtractChar(s)),NIL);
	push(l);
	while (*s) {
	    snd(l) = ap(consChar(ExtractChar(s)),NIL);
	    l = snd(l);
	}
	snd(l) = nameNil;
    }
}

/* Helper function for constructing IOErrors (see Prelude defn of
 * IOError for explanation of what the the individual arguments
 * do.
 */
  
Cell
mkIOError(mbH, kind, loc, desc, mbF)
Cell   *mbH;	/* a Handle or NULL */
Name   kind;	/* an IOErrorType */
String loc;
String desc;
Cell   *mbF;	/* a FilePath or NULL */
{
    Cell str;
    push(nameIOError);
    toparg(mbH==NULL ? nameNothing : ap(nameJust,*mbH));
    toparg(kind);
    pushString(loc); str = pop(); toparg(str);
    pushString(desc); str = pop(); toparg(str);
    toparg(mbF==NULL ? nameNothing : ap(nameJust,*mbF));
    return pop();
}


#endif
/* --------------------------------------------------------------------------
 * IO Errors (more defined for file ops)
 * ------------------------------------------------------------------------*/

static Void local throwErrno(String fname, Bool isFile, Int h, Cell *mbF)
{
    IOFail(mkIOError(h == NO_HANDLE ? NULL : &handles[h].hcell,
		     toIOError(errno),
		     fname,
		     toIOErrorDescr(errno, isFile),
		     mbF));
}

/*
 * Map a libc error code to an IOError
 */
static Name local toIOError(errc)
int errc;
{
    switch(errc) {

    case EEXIST:
	return nameAlreadyExists;
    case ENOENT:
    case ENOTDIR:
	return nameDoesNotExist;
    case EPERM:
    case EACCES:
	return namePermDenied;
    case ENOSPC:
    case EFBIG:
	return nameIsFull;
    default:
	return nameIllegal;
    }
}

/*
 * Map a libc error code to an IOError descriptive string
 */
static String local toIOErrorDescr(errc,isFile)
int   errc;
Bool  isFile;
{
    switch(errc) {

    case EEXIST:
	return (isFile ? "file already exists" : "directory already exists");
    case ENOENT:
    case ENOTDIR:
	return (isFile ? "file does not exist" : "directory does not exist");
    case EPERM:
    case EACCES:
	return ""; /* No need to replicate the info conveyed by the IOErrorKind */
    case ENOSPC:
    case EFBIG:
	return "device is full";
    default:
	return "";
    }
}

/* --------------------------------------------------------------------------
 * Misc.
 * ------------------------------------------------------------------------*/

primFun(primGC) {			/* force a GC right now            */
    garbageCollect();
    IOReturn(nameUnit);
}

#if BIGNUMS
#include <time.h>

primFun(primGetRandomSeed) {		/* generate a random seed          */
    IOReturn(bigInt(clock()));
}

#else

primFun(primGetRandomSeed) {		/* generate a random seed          */
    ERRMSG(0) "getRandomSeed is not implemented on this architecture"
    EEND;
}

#endif
				     
primFun(primGetEnv) {                 /* primGetEnv :: String -> IO String */
    String s = evalName(IOArg(1));    /* Eval name	                   */
    String r;
    if (!s) {			      /* check for valid name		   */
	IOFail(mkIOError(NULL,
			 nameIllegal,
		         "System.getEnv",
			 "illegal environment variable name",
			 &IOArg(1)));
    }
    if ((r = getenv(s))==0) {    
	IOFail(mkIOError(NULL,
			 nameDoesNotExist,
		         "System.getEnv",
			 "environment variable not found",
			 &IOArg(1)));
    }
    pushString(r);
    IOReturn(pop());
}

#include <sys/types.h>
# include <sys/wait.h>
#ifndef WEXITSTATUS
/* If it's not defined, return it verbatim. */
# define WEXITSTATUS(stat_val) (stat_val)
#endif

primFun(primSystem) {                   /* primSystem :: String -> IO Int  */
    String s = evalName(IOArg(1));	/* Eval name	                   */
    Int r;
    if (!s) {				/* check for valid string          */
	IOFail(mkIOError(NULL,
			 nameIllegal,
		         "System.system",
			 "illegal system command string",
			 &IOArg(1)));
    }
    r = shellEsc(s, TRUE/*synchronous*/, TRUE/*use shell*/);
    IOReturn(mkInt(WEXITSTATUS(r)));
}

Void setHugsArgs(argc,argv)
Int    argc;
String argv[]; {
    int i;
    Cell str;

    hugsArgs = nameNil;
    pushString(argv[0]);
    hugsProgName = pop();
    for (i=argc-1; i>0; i--) {
	pushString(argv[i]);
	str = pop();
	hugsArgs = ap2(nameCons, str, hugsArgs);
    }
}

primFun(primGetProgName) {              /* primGetProgName :: IO String    */
    IOReturn(hugsProgName);
}

primFun(primGetArgs) {                  /* primGetArgs :: IO [String]      */
    IOReturn(hugsArgs);
}

primFun(primSetProgName) {              /* primSetProgName :: String -> IO () */
    hugsProgName = IOArg(1);
    IOReturn(nameUnit);
}

primFun(primSetArgs) {                  /* primSetArgs :: [String] -> IO () */
    hugsArgs = IOArg(1);
    IOReturn(nameUnit);
}

/* --------------------------------------------------------------------------
 * File IO
 * ------------------------------------------------------------------------*/

#if IO_HANDLES

#define HandleArg(nm,offset)  \
    eval(primArg(offset));    \
    nm = intValOf(whnfHead)

#define IOBoolResult(e)  \
    IOReturn((e)?nameTrue:nameFalse)

primFun(primHGetChar) {			/* Read character from handle	   */
    Int h;
    Int c;
    HandleArg(h,1+IOArity);

    checkReadable(h, "IO.hGetChar");
    setRWState(h, RW_READING);
    c = hGetChar(h, "IO.hGetChar");
    if (c==EOF) {
	setRWState(h, RW_NEUTRAL);
	IOFail(mkIOError(&handles[h].hcell,
			 nameEOFErr,
			 "IO.hGetChar",
			 "end of file",
			 NULL));
    }
    IOReturn(mkChar(c));
}

primFun(primHPutChar) {			/* print character on handle	   */
    Char c = 0;
    Int  h;
    HandleArg(h,2+IOArity);
    CharArg(c,1+IOArity);

    checkWritable(h, "IO.hPutChar");
    setRWState(h, RW_WRITING);
    hPutChar(c, h, "IO.hPutChar");
    IOReturn(nameUnit);
}

primFun(primHPutStr) {			/* print string on handle	   */
    Int h;
    HandleArg(h,2+IOArity);
    push(primArg(1+IOArity));
    primArg(1+IOArity) = NIL;

    checkWritable(h, "IO.hPutStr");
    setRWState(h, RW_WRITING);
    blackHoleRoot();
    eval(pop());
    while (whnfHead==nameCons) {
	eval(pop());
	hPutChar(charOf(whnfHead),h,"IO.hPutStr");
	eval(pop());
    }
#if !FLUSHEVERY
    if (h <= 2) { /* Only flush the standard handles */
	fflush(handles[h].hfp);
    }
#endif
    IOReturn(nameUnit);
}

primFun(primHreader) {			/* read String from a handle 	   */
    Int h;                              /* Handle -> String                */
    HandleArg(h,1);
    if (handles[h].hmode&HSEMICLOSED) {	/* read requires semi-closed handle*/
	Int c = hGetChar(h, "IO.getContents");
	if (c!=EOF && c>=0 && c<=MAXCHARVAL) {
	    updapRoot(consChar(c),ap(nameHreader,primArg(1)));
	    return;
	}
	clearerr(handles[h].hfp);
    }
    updateRoot(nameNil);
}

primFun(primHContents) {		/* hGetContents :: Handle -> IO Str*/
    Int h;
    HandleArg(h,1+IOArity);

    checkReadable(h, "IO.hGetContents");  /* must have readable handle     */
    setRWState(h, RW_READING);
    handles[h].hmode = HSEMICLOSED; /* semi-close handle		   */
    IOReturn(ap(nameHreader,IOArg(1)));
}

static int local getIOMode(mode)	/* From IOMode to internal form    */
Cell mode; {
    Int    m = HCLOSED;

    eval(mode);				/* Eval IOMode			   */
    if (isName(whnfHead) && isCfun(whnfHead))
	switch (cfunOf(whnfHead)) {	/* we have to use numeric consts   */
	    case 1 : m = HREAD;		/* here to avoid the need to put   */
		     break;		/* IOMode in startup environment   */
	    case 2 : m = HWRITE;
		     break;
	    case 3 : m = HAPPEND;
		     break;
	    case 4 : m = HREADWRITE;
		     break;
	}
    return m;
}

static Void local fopenPrim(root,binary,loc)/* Auxiliary function for          */
StackPtr root;                              /* opening a file                  */
Bool     binary;
String   loc; {
    Int    m;

    m = getIOMode(IOArg(1));
    if (m==HCLOSED) {			/* Only accept legal modes	   */
	IOFail(mkIOError(NULL,
			 nameIllegal,
			 loc,
			 "unknown handle mode",
			 &IOArg(2)));
    }
    IOReturn(openHandle(root,&IOArg(2),m,binary,loc));
}

primFun(primOpenFile) {			/* open handle to a text file	   */
    fopenPrim(root,FALSE,"IO.openFile");
}

primFun(primOpenBinaryFile) {		/* open handle to a binary file	   */
    fopenPrim(root,TRUE,"System.IO.openBinaryFile");
}

primFun(primStdin) {			/* Standard input handle	   */
    push(handles[HSTDIN].hcell);
}

primFun(primStdout) {			/* Standard output handle	   */
    push(handles[HSTDOUT].hcell);
}

primFun(primStderr) {			/* Standard error handle	   */
    push(handles[HSTDERR].hcell);
}

primFun(primOpenFd) {			/* open handle to file descriptor. */
    Int  m;                             /* :: Int{-Fd-} -> Bool -> IOMode -> Bool -> IO Handle */
    Int  fd;
    Bool binary;
    Bool isSock;

    IntArg(fd,4+IOArity);
    BoolArg(isSock,3+IOArity);
    BoolArg(binary,1+IOArity);

    m = getIOMode(IOArg(2));
    if (m==HCLOSED) {			/* Only accept legal modes	   */
        IOFail(mkIOError(NULL,
		         nameIllegal,
		         "openFd",
		         "unknown handle mode",
		         NULL));
    }

    IOReturn(openFdHandle(root,fd,m,binary,"openFd"));
}

/* Extract the file descriptor from a Handle, discarding the Handle */
primFun(primHandleToFd) {
    Int h;
    HandleArg(h,1+IOArity);
    if (IS_STANDARD_HANDLE(h)) {
        IOFail(mkIOError(NULL,
			 nameIllegal,
		         "System.Posix.IO.handleToFd",
		         "invalid handle",
			 NULL));
    }
    checkOpen(h, "System.Posix.IO.handleToFd");
    {
        Int fd = dup(fileno(handles[h].hfp));
        fclose(handles[h].hfp);
        handles[h].hfp   = 0;
        handles[h].hmode = HCLOSED;
        IOReturn(mkInt(fd));
    }
}

primFun(primHIsEOF) {	/* Test for end of file on handle  */
                        /* :: Handle -> IO Bool */
    Int h;
    FILE* fp;
    Bool isEOF;
    HandleArg(h,1+IOArity);
    checkReadable(h, "IO.hIsEOF");
    fp = handles[h].hfp;
    isEOF = feof(fp);
    if (isEOF) { /* If the EOF flag is already signalled,
		    peeking at the next char isn't likely
		    to produce a different outcome! */
	IOBoolResult(isEOF);
#if CHAR_ENCODING
    } else if (handles[h].hLookAhead>=0) {
	IOReturn(nameFalse);
#endif
    } else {
	Int c;

	setRWState(h, RW_READING);
	c = fgetc(fp);
	isEOF = feof(fp);
	if (isEOF)
	    setRWState(h, RW_NEUTRAL);
	/* Put the char back and clear any flags. */
	ungetc(c,fp);
	clearerr(fp);
	
	IOBoolResult(isEOF);
    }
}

primFun(primHFlush) {			/* Flush handle			   */
    Int h;
    HandleArg(h,1+IOArity);
    checkWritable(h, "IO.hFlush");
    fflush(handles[h].hfp);
    IOReturn(nameUnit);
}

primFun(primHClose) {			/* Close handle                   */
    Int h;
    HandleArg(h,1+IOArity);

    /* Disallow closing any of the standard handles */
    if (!IS_STANDARD_HANDLE(h) && handles[h].hmode!=HCLOSED) {
	if (h>HSTDERR && handles[h].hfp)
	    fclose(handles[h].hfp);
	handles[h].hfp   = 0;
	handles[h].hmode = HCLOSED;
    }
    /* closing an already closed handle is the identity
       (i.e., not an error.) */
    IOReturn(nameUnit);
}

primFun(primHGetPosn) {			/* Get file position               */
    Int h;
    long pos;
    HandleArg(h,1+IOArity);
    checkOpen(h, "IO.hGetPosn");
    pos = ftell(handles[h].hfp);
    IOReturn(mkInt((Int)pos));
}

primFun(primHSetPosn) {			/* Set file position               */
    long   pos = 0;
    Int    h;
    HandleArg(h,2+IOArity);
    IntArg(pos,1+IOArity);
    checkOpen(h, "IO.hSetPosn");
    setRWState(h, RW_NEUTRAL);
    fflush(handles[h].hfp);
    if (fseek(handles[h].hfp,pos,SEEK_SET) == 0) {
	IOReturn(nameUnit);
    }
}

primFun(primHSeek) {	/* Seek to new file posn */
                        /* :: Handle -> Int -> Int -> IO () */
    Int h;
    Int sMode;
    Int off;

    HandleArg(h,3+IOArity);
    IntArg(sMode, 2+IOArity);
    IntArg(off, 1+IOArity);

    if (sMode == 0) 
	sMode = SEEK_SET;
    else if (sMode == 1)
	sMode = SEEK_CUR;
    else
	sMode = SEEK_END;

    checkOpen(h, "IO.hSeek");
    if (fseek(handles[h].hfp,off,sMode) != 0)
	throwErrno("IO.hSeek", TRUE, h, NULL);

    setRWState(h, RW_NEUTRAL);

    IOReturn(nameUnit);
}

primFun(primHLookAhead) { /* Peek at the next char */
                          /* :: Handle -> IO Char  */
    Int h;
    Int c;
    
    HandleArg(h,1+IOArity);

    checkReadable(h, "IO.hLookAhead");
#if CHAR_ENCODING
    if (handles[h].hLookAhead>=0) {
	IOReturn(mkChar(handles[h].hLookAhead));
	return;
    }
#endif
    setRWState(h, RW_READING);
    if (feof(handles[h].hfp) ||
	(c = hGetChar(h, "IO.hLookAhead")) == EOF) {
	setRWState(h, RW_NEUTRAL);
	IOFail(mkIOError(&handles[h].hcell,
			 nameEOFErr,
			 "IO.hLookAhead",
			 "end of file",
			 NULL));
    }
#if CHAR_ENCODING
    if (handles[h].hBinaryMode)
	ungetc(c, handles[h].hfp);
    else
	handles[h].hLookAhead = c;
#else
    ungetc(c, handles[h].hfp);
#endif
    IOReturn(mkChar(c));
}

primFun(primHSetBuffering) {	/* Change a Handle's buffering */
                                /* :: Handle -> Int -> Int -> IO () */
    Int h;
    Int ty;
    Int sz;
    int rc;
    HandleArg(h,3+IOArity);
    IntArg(ty,2+IOArity);
    IntArg(sz,1+IOArity);

    checkOpen(h, "IO.hSetBuffering");
    switch(ty) {
    case 0:
	ty = _IONBF;
	handles[h].hbufMode = HANDLE_NOTBUFFERED;
	handles[h].hbufSize = 0;
	break;
    case 1:
	ty = _IOLBF;
	sz = BUFSIZ;
	handles[h].hbufMode = HANDLE_LINEBUFFERED;
	handles[h].hbufSize = 0;
	break;
    case 2:
	ty = _IOFBF;
	handles[h].hbufMode = HANDLE_BLOCKBUFFERED;
	if (sz == 0) {
	    sz=BUFSIZ;
	}
	handles[h].hbufSize = sz;
	break;
    default:
	IOFail(mkIOError(&handles[h].hcell,
			 nameIllegal,
			 "IO.hSetBuffering",
			 "illegal buffer mode",
			 NULL));
    }

    /* Change the buffering mode; setvbuf() flushes the old buffer. */
    /* Let setvbuf() allocate the buffer for us. */
    rc = setvbuf(handles[h].hfp, NULL, ty, sz);
    if (rc != 0)
	throwErrno("IO.hSetBuffering", TRUE, h, NULL);
    if ((handles[h].hmode&(HWRITE|HAPPEND|HREADWRITE)) &&
	isatty(fileno(handles[h].hfp)))
	setBuffTerminal(fileno(handles[h].hfp), ty!=0);
    IOReturn(nameUnit);
}

primFun(primHGetBuffering) {	/* Return buffering info of a handle. */
                                /*  Handle :: IO (Int,Int)            */
    Int h;
    HandleArg(h,1+IOArity);

    checkOpen(h, "IO.hGetBuffering");
    if (handles[h].hbufMode == HUNKNOWN_BUFFERING) {
	/* figure out buffer mode and size. */
	if (isatty (fileno(handles[h].hfp)) ) {
	    /* TTY connected handles are normally linebuffered. */
	    handles[h].hbufMode =
		(handles[h].hmode&(HWRITE|HAPPEND|HREADWRITE))==0 ||
		    getBuffTerminal(fileno(handles[h].hfp)) ?
		    HANDLE_LINEBUFFERED : HANDLE_NOTBUFFERED;
	    handles[h].hbufSize = 0;
	} else {
	    /* ..if not, block buffered. */
	    handles[h].hbufMode = HANDLE_BLOCKBUFFERED;
	    handles[h].hbufSize = BUFSIZ;
	}
    }
    IOReturn(ap(ap(mkTuple(2),mkInt((Int)handles[h].hbufMode)),
		mkInt((Int)handles[h].hbufSize)));
}

primFun(primHIsOpen) {			/* Test is handle open             */
    Int h;
    HandleArg(h,1+IOArity);
    IOBoolResult(handles[h].hmode!=HCLOSED 
		 && handles[h].hmode!=HSEMICLOSED);
}

primFun(primHIsClosed) {		/* Test is handle closed           */
    Int h;
    HandleArg(h,1+IOArity);
    IOBoolResult(handles[h].hmode==HCLOSED);
}

primFun(primHIsReadable) {		/* Test is handle readable         */
    Int h;
    HandleArg(h,1+IOArity);
    IOBoolResult(handles[h].hmode&(HREAD|HREADWRITE));
}

primFun(primHIsWritable) {		/* Test is handle writable         */
    Int h;
    HandleArg(h,1+IOArity);
    IOBoolResult(handles[h].hmode&(HWRITE|HREADWRITE|HAPPEND));
}

#if defined(IS_WINDOWS) && !defined(S_ISREG)
#define S_ISREG(x)  ((x) & _S_IFREG)
#endif

primFun(primHIsSeekable) {		/* Test if handle is writable   */
    Int h;
    Bool okHandle;
    struct stat sb;

    HandleArg(h,1+IOArity);

    okHandle = (handles[h].hmode&(HREAD|HWRITE|HREADWRITE|HAPPEND));
    if (okHandle && (fstat(fileno(handles[h].hfp), &sb) == 0)) {
	okHandle = S_ISREG(sb.st_mode);
    }
    IOBoolResult(okHandle);
}

primFun(primHFileSize) {  /* If handle points to a regular file,
			     return the size of the file   */
                          /* :: Handle -> IO Integer       */
    Int h;
    struct stat sb;

    HandleArg(h,1+IOArity);

    checkOpen(h, "IO.hFileSize");
    if (handles[h].hmode&(HWRITE|HREADWRITE|HAPPEND))
	fflush(handles[h].hfp);
    if (fstat(fileno(handles[h].hfp), &sb) != 0 || !S_ISREG(sb.st_mode)) {
	IOFail(mkIOError(&handles[h].hcell,
			 nameIllegal,
			 "IO.hFileSize",
			 "not a regular file",
			 NULL));
    }
    IOReturn(bigWord(sb.st_size));
  }

primFun(primEqHandle) {			/* Test for handle equality        */
    Int h1, h2;
    HandleArg(h1,1);
    HandleArg(h2,2);
    BoolResult(h1==h2);
}

primFun(primGetHandleNumber) {
    Int h;
    HandleArg(h,1);
    IntResult(h);
}

primFun(primHSetBinaryMode) {
    Int h;
    Bool binary;
    HandleArg(h, 2+IOArity);
    BoolArg(binary, 1+IOArity);

    checkOpen(h, "System.IO.hSetBinaryMode");

#if CHAR_ENCODING
    handles[h].hBinaryMode = binary;
#endif
    IOReturn(nameUnit);
}

primFun(primHPutBuf) {			/* write binary data from a buffer   */
    Int h, size;
    Pointer buf;

    HandleArg(h, 3+IOArity);
    PtrArg(buf, 2+IOArity);
    IntArg(size, 1+IOArity);

    /* argument checks */
    checkWritable(h, "System.IO.hPutBuf");
    if (size < 0) {
	IOFail(mkIOError(&handles[h].hcell,
			 nameIllegal,
			 "System.IO.hPutBuf",
			 "illegal buffer size",
			 NULL));
    }
#if CHAR_ENCODING
    if (!handles[h].hBinaryMode) {
	IOFail(mkIOError(&handles[h].hcell,
			 nameIllegal,
			 "System.IO.hPutBuf",
			 "not a binary handle",
			 NULL));
    }
#endif
    setRWState(h, RW_WRITING);

    errno = 0;
    while (size > 0) {
	size -= (Int)fwrite(buf, 1, size, handles[h].hfp);
	if (errno < 0)
	    throwErrno("System.IO.hPutBuf", TRUE, h, NULL);
    }

    IOReturn(nameUnit);
}

primFun(primHGetBuf) {			/* read binary data into a buffer   */
    Int h, size, numRead;
    Pointer buf;

    HandleArg(h, 3+IOArity);
    PtrArg(buf, 2+IOArity);
    IntArg(size, 1+IOArity);

    /* argument checks */
    checkReadable(h, "System.IO.hGetBuf");
    if (size < 0) {
	IOFail(mkIOError(&handles[h].hcell,
			 nameIllegal,
			 "System.IO.hGetBuf",
			 "illegal buffer size",
			 NULL));
    }
#if CHAR_ENCODING
    if (!handles[h].hBinaryMode) {
	IOFail(mkIOError(&handles[h].hcell,
			 nameIllegal,
			 "System.IO.hGetBuf",
			 "not a binary handle",
			 NULL));
    }
#endif
    setRWState(h, RW_READING);

    numRead = (Int)fread(buf, 1, size, handles[h].hfp);
    if (numRead < size && ferror(handles[h].hfp))
	throwErrno("System.IO.hGetBuf", TRUE, h, NULL);
    IOReturn(mkInt(numRead));
}

primFun(primHWaitForInput) { /* Check whether a character can be read
				from a handle within x msecs */
                             /* :: Handle -> Int -> IO Bool */
    Int h;
    Int msecs;
    
    HandleArg(h,2+IOArity);
    IntArg(msecs,1+IOArity);
    
    checkReadable(h, "IO.hWaitForInput");
    {
	/* Implementation is a rip-off of GHC's inputReady.c */
	int maxfd, fd;
	int ready;
	fd_set rfd;
	struct timeval tv;
	
	FD_ZERO(&rfd);
	fd = fileno(handles[h].hfp);
	FD_SET(fd, &rfd);
	
	maxfd = fd + 1;
	tv.tv_sec  = msecs / 1000;
	tv.tv_usec = msecs % 1000;
	
	while ( (ready = select(maxfd, &rfd, NULL, NULL, &tv)) < 0 ) {
	    if (errno != EINTR) {
		IOFail(mkIOError(&handles[h].hcell,
				 nameIllegal,
				 "IO.hWaitForInput",
				 "input waiting terminated by signal",
				 NULL));
	    }
	}
	IOBoolResult(ready > 0);
    }
}

primFun(primHIsTerminalDevice) { /* Does the handle refer to a terminal? */
    Int h;
    HandleArg(h, 1+IOArity);
    checkOpen(h, "System.IO.hIsTerminalDevice");
    IOBoolResult(isatty(fileno(handles[h].hfp)));
}

primFun(primHGetEcho) {
    Int h;
    Int fd;
    HandleArg(h, 1+IOArity);
    checkOpen(h, "System.IO.hGetEcho");
    fd = fileno(handles[h].hfp);
    IOBoolResult(isatty(fd) && getEchoTerminal(fd));
}

primFun(primHSetEcho) {
    Int h;
    Bool echo;
    Int fd;
    HandleArg(h, 2+IOArity);
    BoolArg(echo, 1+IOArity);
    checkOpen(h, "System.IO.hSetEcho");
    fd = fileno(handles[h].hfp);
    if (isatty(fd))
	setEchoTerminal(fd, echo);
}

#endif /* IO_HANDLES */

/* --------------------------------------------------------------------------
 * Mutable variables
 * ------------------------------------------------------------------------*/

#if IO_REFS

#if CHECK_TAGS
#define checkRef() if (MUTVAR != whatIs(whnfHead)) internal("Ref expected")
#else
#define checkRef() /* do nothing */
#endif

primFun(primNewRef) {			/* a -> IO (Ref a)		   */
    IOReturn(ap(MUTVAR,IOArg(1)));
}

primFun(primDerefRef) {			/* Ref a -> IO a		   */
    eval(pop());
    checkRef();
    IOReturn(snd(whnfHead));
}

primFun(primAssignRef) {		/* Ref a -> a -> IO ()		   */
    eval(IOArg(2));
    checkRef();
    snd(whnfHead) = IOArg(1);
    IOReturn(nameUnit);
}

primFun(primEqRef) {			/* Ref a -> Ref a -> Bool	   */
    eval(primArg(2));
    checkRef();
    push(whnfHead);
    eval(primArg(1));
    checkRef();
    updateRoot(pop()==whnfHead ? nameTrue : nameFalse);
}
#endif

/* --------------------------------------------------------------------------
 * Stable Pointers
 * ------------------------------------------------------------------------*/

#if CHECK_TAGS
#define checkSP() checkInt()
#else
#define checkSP() /* do nothing */
#endif

#define SPArg(nm,offset)                          \
    eval(primArg(offset));                        \
    checkSP();                                    \
    nm = (HugsStablePtr)whnfInt

/* nm should be a variable in which result is stored.
   If you use an expression, reevaluation might occur */
#define SPResult(nm)                              \
   updateRoot(mkInt((Int)(nm)))


primFun(primMakeSP) {			/* a -> IO (StablePtr a)	   */
    HugsStablePtr sp = mkStablePtr(IOArg(1));
    if (sp == 0) {
        IOFail(mkIOError(NULL,
			 nameIsFull,
			 "Foreign.makeStablePtr",
		         "too many StablePtrs",
			 NULL));
    }
    IOReturn(mkInt(sp));
}

primFun(primDerefSP) {			/* StablePtr a -> IO a   	   */
    HugsStablePtr x;
    SPArg(x,1+IOArity);
    
    IOReturn(derefStablePtr(x));
}

primFun(primFreeSP) {			/* StablePtr a -> IO ()   	   */
    HugsStablePtr x;
    SPArg(x,1+IOArity);
    freeStablePtr(x);
    IOReturn(nameUnit);
}

primFun(primCastSPToP) {		/* StablePtr a -> Ptr ()   	   */
    HugsStablePtr x;
    SPArg(x,1);
    PtrResult((Pointer)x);
}

primFun(primCastPToSP) {		/* Ptr () -> StablePtr a   	   */
    Pointer x;
    PtrArg(x,1);
    SPResult((HsStablePtr)x);
}


    
/* --------------------------------------------------------------------------
 * Foreign Objects
 * ------------------------------------------------------------------------*/

#if CHECK_TAGS
#define checkForeign() if (MPCELL != whatIs(whnfHead)) internal("ForeignObj expected")
#else
#define checkForeign() /* do nothing */
#endif

primFun(primNewFP) { /* Ptr a -> IO (ForeignPtr a) */
    Pointer addr = 0;
    eval(IOArg(1));
    addr = ptrOf(whnfHead);
    IOReturn(newMallocPtr(addr));
}

primFun(primAddFPF) { /* FunPtr (Ptr a -> IO ()) -> ForeignPtr a -> IO () */
    int mp;
    eval(IOArg(1));
    mp = mpOf(whnfHead);
    eval(IOArg(2));
    mallocPtrs[mp].finalizers = cons(whnfHead, mallocPtrs[mp].finalizers);
    IOReturn(nameUnit);
}

primFun(primAddFPFEnv) {	 /* FunPtr (Ptr env -> Ptr a -> IO ()) ->  */
    int mp;			 /*	Ptr Env -> ForeignPtr a -> IO ()   */
    eval(IOArg(1));
    mp = mpOf(whnfHead);
    eval(IOArg(2));
    push(whnfHead);
    eval(IOArg(3));
    mallocPtrs[mp].finalizers =
	cons(pair(whnfHead, pop()), mallocPtrs[mp].finalizers);
    IOReturn(nameUnit);
}

primFun(primWriteFP) {		/* ForeignPtr a -> Ptr a -> IO ()	   */
    Cell mp = NIL;
    eval(IOArg(2));
    checkForeign();
    mp = whnfHead;
    eval(IOArg(1));
    derefMP(mp) = ptrOf(whnfHead);
    IOReturn(nameUnit);
}

primFun(primEqFP) {		/* ForeignPtr a -> ForeignPtr a -> Bool    */
    eval(primArg(2));
    checkForeign();
    push(whnfHead);
    eval(primArg(1));
    checkForeign();
    updateRoot(pop()==whnfHead ? nameTrue : nameFalse);
}

primFun(primTouchFP) {		/* ForeignPtr a -> IO ()	           */
    eval(IOArg(1));
    checkForeign();
    IOReturn(nameUnit);
}

primFun(primFPToP) {		/* ForeignPtr a -> Ptr a                   */
    eval(primArg(1));
    checkForeign();
    PtrResult(derefMP(whnfHead));
}

#if STABLE_NAMES
/* --------------------------------------------------------------------------
 * Stable Names
 * ------------------------------------------------------------------------*/

primFun(primMakeSN) {		/* a -> IO (StableName a)		   */
    IOReturn(ap(STABLENAME,IOArg(1)));
}

primFun(primDerefSN) {		/* StableName a -> a			   */
    eval(primArg(1));
    updateRoot(snd(whnfHead));
}

primFun(primHashSN) {		/* StableName a -> Int			   */
    eval(primArg(1));
    updateRoot(mkInt(whnfHead));
}
primFun(primEqSN) {		/* StableName a -> StableName a -> Bool	   */
    eval(primArg(2));
    push(whnfHead);
    eval(primArg(1));
    updateRoot(pop()==whnfHead ? nameTrue : nameFalse);
}
#endif

#if GC_WEAKPTRS
/* --------------------------------------------------------------------------
 * Weak Pointers
 * ------------------------------------------------------------------------*/

#if CHECK_TAGS
#define checkWeak() if(WEAKCELL!=whatIs(whnfHead)) internal("weakPtr expected");
#else
#define checkWeak() /* do nothing */
#endif

primFun(primWeakPtrEq) {		/* Weak a -> Weak a -> Bool	   */
    eval(primArg(2));
    push(whnfHead);
    eval(primArg(1));
    updateRoot(pop()==whnfHead ? nameTrue : nameFalse);
}

primFun(primMkWeak) {			/* k -> v -> Maybe (IO ())	   */
    Cell w = NIL;			/*		    -> IO (Weak v) */
    eval(IOArg(1));
    if (whnfHead==nameJust) {		/* Look for finalizer		   */
	w = pop();
    }
    w		     = ap(NIL,ap(NIL,ap(NIL,w)));
    fst(snd(w))      = IOArg(3);
    fst(snd(snd(w))) = IOArg(2);
    liveWeakPtrs     = cons(w,liveWeakPtrs);
    fst(w)           = WEAKFIN;
    IOReturn(w);
}

primFun(primDeRefWeak) {		/* Weak v -> IO (Maybe v)	   */
    eval(IOArg(1));
    if (whatIs(whnfHead)!=WEAKFIN) {
	internal("primDeRefWeak");
    }
    if (nonNull(snd(whnfHead))) {
	IOReturn(ap(nameJust,fst(snd(snd(whnfHead)))));
    } else {
	IOReturn(nameNothing);
    }
}

primFun(primReplaceFinalizer) {		/* Weak v -> Maybe (IO ())	   */
					/*	-> IO (Maybe (IO ()))	   */
    eval(IOArg(1));			/* Grab new finalizer ...	   */
    if (whnfHead!=nameJust) {
	push(NIL);
    }
    eval(IOArg(2));			/* Get weak pointer ...		   */
    if (whatIs(whnfHead)!=WEAKFIN) {
	internal("primReplaceFinalizer");
    } else if (nonNull(snd(whnfHead))) {/* ... and replace finalizer	   */
	Cell oldfin = snd(snd(snd(whnfHead)));
	snd(snd(snd(whnfHead))) = pop();
	if (nonNull(oldfin)) {
	    IOReturn(ap(nameJust,oldfin));
	}
    }
    IOReturn(nameNothing);
}

primFun(primFinalize) {			/* Weak v -> IO ()		   */
    eval(IOArg(1));			/* Bring weak pointer to an early  */
    if (whatIs(whnfHead)!=WEAKFIN) {	/* end ...			   */
	internal("primFinalize");
    } else if (nonNull(snd(whnfHead))) {
	Cell wp = whnfHead;
	Cell vf = snd(snd(wp));
	if (isPair(vf)) {
	    if (nonNull(snd(vf))) {
		fst(vf)    = snd(vf);
		snd(vf)    = finalizers;
		finalizers = vf;
	    }
	    fst(snd(wp)) = NIL;
	    snd(snd(wp)) = NIL;
	    snd(wp)      = NIL;
	}
	liveWeakPtrs = removeCell(wp,liveWeakPtrs);
    }
    IOReturn(nameUnit);
}

primFun(primRunFinalizer) {		/* IO ()			   */
    if (isNull(finalizers)) {
	IOReturn(nameUnit);
    } else {
	updapRoot(ap(hd(finalizers),primArg(2)),primArg(1));
	finalizers = tl(finalizers);
	return;
    }
}

primFun(primFinalizerWaiting) {		/* IO Boolean			   */
  IOBoolResult(!isNull(finalizers));
}
#endif /* GC_WEAKPTRS */


#if HSCRIPT
#if EMBEDDED
extern void* getCurrentScript(void);

primFun(primGetCurrentScript) {  /* IO Int */
    IOReturn( mkInt( (int)getCurrentScript() ) );
}

#else
 
primFun(primGetCurrentScript) {  /* IO Int */
    IOReturn( mkInt( 0 ) );
}

#endif /* EMBEDDED */
#endif /* HSCRIPT */

/* --------------------------------------------------------------------------
 * Primitives for implementing disposable memo functions
 * Byron Cook -- byron@cse.ogi.edu
 *
 * IOEql :: Eval a => a -> a -> IO Bool 
 *   if argument is an Int or Char
 *   then use ==
 *   else use pointer identity
 *
 * IOHash :: Eval a => a -> IO Int
 *   if a is an Int or Char
 *   then use value cast as an Int
 *   else use pointer identity
 *
 * (Earlier versions made Float a special case too - but that's not very
 *  portable since it assumes that sizeof(FloatPro) == sizeof(Int).)
 * ------------------------------------------------------------------------*/

primFun(primIOEql) {		    /* :: Eval a => a -> a -> ST Mem Bool */
    Cell x = IOArg(1);
    Cell y = IOArg(2);
    eval(x);
    eval(y);
    x = followInd(IOArg(1));
    y = followInd(IOArg(2));

    if (whatIs(x) == whatIs(y)) {
	switch (whatIs(x)) {
	    case INTCELL   : IOBoolResult(intOf(x)==intOf(y));
	    case CHARCELL  : IOBoolResult(charOf(x)==charOf(y));
	    /* deliberate fall through to end */
	}
    }
    IOBoolResult(x==y);
}

primFun(primIOHash) {                      /* :: Eval a => a -> ST Mem Int */
    Cell x = IOArg(1);
    eval(x);
    x = followInd(IOArg(1)); 

    switch(whatIs(x)) {
	case INTCELL   : IOBoolResult(x); 
	case CHARCELL  : IOBoolResult(mkInt(charOf(x)));
    }
    IOBoolResult(mkInt((Int)x));
}

/*-------------------------------------------------------------------------*/
#endif

/* --------------------------------------------------------------------------
 * Time and CPUTime module implementations
 * ------------------------------------------------------------------------*/

#if TIME_MODULE
/*
 * Primitives needed to implement the Haskell 98 Time & CPUTime modules.
 *
 * This file has to be included by builtin.c, and won't compile on its own.
 */
 

/* --------------------------------------------------------------------------
 * Time/CPUTime control:
 * ------------------------------------------------------------------------*/

static Void timeControl (Int);

static Void timeControl(what)
Int what; {
}

/* --------------------------------------------------------------------------
 * Time/CPUTime primitive table:
 * ------------------------------------------------------------------------*/

PROTO_PRIM(primClockTicks);
PROTO_PRIM(primGetCPUUsage);

PROTO_PRIM(primGetClockTime);
PROTO_PRIM(primGetCalTime);
PROTO_PRIM(primMkTime);

static struct primitive timePrimTable[] = {
  /* CPUTime primitives */
  {"clockTicks",           0, primClockTicks},
  {"getCPUUsage",          0+IOArity, primGetCPUUsage},
  /* Time primitives */
  {"getClockTimePrim",     0+IOArity, primGetClockTime},        
  {"toCalTimePrim",        2+IOArity, primGetCalTime},        
  {"toClockTimePrim",      7+IOArity, primMkTime},
  {0,			0, 0}
};

static struct primInfo timePrims = { timeControl, timePrimTable, 0 };

/* --------------------------------------------------------------------------
 * Time primitives:
 * ------------------------------------------------------------------------*/

primFun(primGetClockTime) { /* :: IO (Int,Int) */
  struct timeval tv;
  int rc;
  
  rc = gettimeofday(&tv,NULL);
  
  if (rc == -1)
    throwErrno("Time.getClockTime", TRUE, NO_HANDLE, NULL);
  IOReturn(ap(ap(mkTuple(2), mkInt(tv.tv_sec)),mkInt(tv.tv_usec)));
}

primFun(primGetCalTime) { /* Int   -> Int -> IO (.....) */
                          /* isUTC    secs */
                          /* isUTC => convert time to UTC, o/wise local time */

  Int isUTC;
  time_t secs;
  struct tm* tm;
  char* zoneNm = NULL;
  Int utcOff;
  Cell zoneStr = NIL;

  IntArg(isUTC,2+IOArity);
  IntArg(secs,1+IOArity);
  
  if (isUTC) {
    tm=gmtime(&secs);
  } else {
    tm=localtime(&secs);
  }
  
  /* Warning - ugliness. */
  zoneNm = (char*)tm->tm_zone;

  utcOff = tm->tm_gmtoff;

  pushString(zoneNm);
  zoneStr = pop();

  IOReturn(ap(ap(ap(ap(ap(ap(ap(ap(ap(ap(ap(mkTuple(11),mkInt(tm->tm_sec)),
					 mkInt(tm->tm_min)),
				      mkInt(tm->tm_hour)),
				   mkInt(tm->tm_mday)),
				mkInt(tm->tm_mon)),
			     mkInt(tm->tm_year)),
			  mkInt(tm->tm_wday)),
		       mkInt(tm->tm_yday)),
		    mkInt(tm->tm_isdst)),
		 zoneStr),
	      mkInt(utcOff)));

}

primFun(primMkTime) { /* Int{-year-}  -> Int{-month-} -> Int{-day-} ->
                         Int{-hour-}  -> Int{-mins-}  -> Int{-sec-} ->
			 Int{-tz offset-} -> IO Int{-secs since Epoch-} */
  Int year, month, day;
  Int hour,min,sec;
  Int tz;

  struct tm tm;
  time_t t;
  
  IntArg(year,7+IOArity);
  IntArg(month,6+IOArity);
  IntArg(day,5+IOArity);
  IntArg(hour,4+IOArity);
  IntArg(min,3+IOArity);
  IntArg(sec,2+IOArity);
  IntArg(tz,1+IOArity);
  
  tm.tm_year = year;
  tm.tm_mon  = month;
  tm.tm_mday = day;
  tm.tm_hour = hour;
  tm.tm_min  = min;
  tm.tm_sec  = sec;
  /* The OpenGroup spec suggests that setting tm_isdst to a neg. value,
     makes mktime() try to figure this out on its own. */
  tm.tm_isdst = -1;
  
  t = mktime(&tm);
  
  if (t ==(time_t)-1)
    throwErrno("Time.toClockTime", TRUE, NO_HANDLE, NULL);

  /* mktime() assumes that the given time was local, but we might have
     been passed an UTC cal. time, so we now have to add the UTC
     offset, that is, the difference between toClockTime's UTC offset and
     the UTC offset returned by mktime().
  */
  tz = -tz;
  tz += tm.tm_gmtoff;
  
  IOReturn(mkInt(t+tz));
}

/* --------------------------------------------------------------------------
 * CPUTime primitives:
 * ------------------------------------------------------------------------*/

#ifdef CLK_TCK
CAFInt(primClockTicks, CLK_TCK)
#elif defined(CLOCKS_PER_SEC)
CAFInt(primClockTicks, CLOCKS_PER_SEC)
#else
CAFInt(primClockTicks, sysconf(_SC_CLK_TCK))
#endif

/* 
 * The code for grabbing the process times has been lifted from GHC.
 * Don't feel too bad about that, since I wrote and maintained it. 
 */
primFun(primGetCPUUsage) { /* IO (Int,Int,Int,Int) */
  int userSec, userNSec;
  int sysSec,  sysNSec;
    struct rusage t;

    getrusage(RUSAGE_SELF, &t);
    userSec  = t.ru_utime.tv_sec;
    userNSec = 1000 * t.ru_utime.tv_usec;
    sysSec   = t.ru_stime.tv_sec;
    sysNSec  = 1000 * t.ru_stime.tv_usec;

    IOReturn(ap(ap(ap(ap( mkTuple(4), mkInt(userSec)),
		      mkInt(userNSec)),
		   mkInt(sysSec)),
		mkInt(sysNSec)));
}

#endif

/* --------------------------------------------------------------------------
 * Directory module implementation
 * ------------------------------------------------------------------------*/

#if DIRECTORY_MODULE
/*
 * Primitives needed to implement the Haskell 98 Directory module.
 *
 * This file has to be included by builtin.c, and won't compile on its own.
 */
 

/* --------------------------------------------------------------------------
 * Directory control:
 * ------------------------------------------------------------------------*/

static Void dirControl (Int);

static Void dirControl(what)
Int what; {
}

/* --------------------------------------------------------------------------
 * Directory primitive table:
 * ------------------------------------------------------------------------*/

PROTO_PRIM(primCreateDirectory);
PROTO_PRIM(primRemoveDirectory);
PROTO_PRIM(primRemoveFile);
PROTO_PRIM(primRenameDirectory);
PROTO_PRIM(primRenameFile);
PROTO_PRIM(primGetDirectory);
PROTO_PRIM(primSetDirectory);
PROTO_PRIM(primFileExist);
PROTO_PRIM(primDirExist);
PROTO_PRIM(primGetPermissions);
PROTO_PRIM(primSetPermissions);
PROTO_PRIM(primGetDirContents);
PROTO_PRIM(primGetModTime);

static struct primitive dirPrimTable[] = {
  {"createDirectory",      1+IOArity, primCreateDirectory},
  {"removeDirectory",      1+IOArity, primRemoveDirectory},
  {"removeFile",           1+IOArity, primRemoveFile},
  {"renameDirectory",      2+IOArity, primRenameDirectory},
  {"renameFile",           2+IOArity, primRenameFile},
  {"getCurrentDirectory",  0+IOArity, primGetDirectory},
  {"setCurrentDirectory",  1+IOArity, primSetDirectory},
  {"doesFileExist",        1+IOArity, primFileExist},
  {"doesDirectoryExist",   1+IOArity, primDirExist},
  {"getPerms",             1+IOArity, primGetPermissions},
  {"setPerms",             5+IOArity, primSetPermissions},
  {"getDirContents",       1+IOArity, primGetDirContents},
  {"getModTime",           1+IOArity, primGetModTime},

  {0,			0, 0}
};

static struct primInfo dirPrims = { dirControl, dirPrimTable, 0 };

static	Bool	local	isDirectory	(String);

#define ToBool(v) ( (v) ? nameTrue : nameFalse)

#ifdef _MSC_VER
/* If not provided, define em. */
#ifndef R_OK
#define R_OK 04
#endif
#ifndef W_OK
#define W_OK 02
#endif
#ifndef X_OK
#define X_OK 06
#endif
#endif

/* MSVC6 doesn't define these helper macros in <sys/stat.h> there
 * might be other platforms too, so... The assumption here is that
 * S_ISDIR() and friends indeed are CPP macros - if that's not the
 * case, please adjust the conditional below to suit your platform
 * (and feed back the tweak you make.)
 */
#if !defined(S_ISDIR)
#define S_ISDIR(st_mode)  ((st_mode & S_IFMT) == S_IFDIR)
#endif

#if !defined(S_ISREG)
#define S_ISREG(st_mode)  ((st_mode & S_IFMT) == S_IFREG)
#endif

/* --------------------------------------------------------------------------
 * Directory primitives:
 * ------------------------------------------------------------------------*/

primFun(primCreateDirectory) { /* create a directory, :: String -> IO ()   */
  int rc;
  String s = evalName(IOArg(1));
  
  if (!s) {
    IOFail(mkIOError(NULL,
		     nameIllegal,
		     "Directory.createDirectory",
		     "illegal directory name",
		     &IOArg(1)));
  }
  
   rc = mkdir(s,0777);

   if (rc != 0)
      throwErrno("Directory.createDirectory", FALSE, NO_HANDLE, &IOArg(1));
  IOReturn(nameUnit);
}

primFun(primRemoveDirectory) { /* remove a directory	   */
  int rc;
  String s = evalName(IOArg(1));
  
  if (!s) {
    IOFail(mkIOError(NULL,
		     nameIllegal,
		     "Directory.removeDirectory",
		     "illegal directory name",
		     &IOArg(1)));
  }
  
   rc = rmdir(s);

   if (rc != 0)
      throwErrno("Directory.removeDirectory", FALSE, NO_HANDLE, &IOArg(1));
  IOReturn(nameUnit);
}

primFun(primRemoveFile) { /* remove a file	   */
  int rc;
  String s = evalName(IOArg(1));
  
  if (!s) {
    IOFail(mkIOError(NULL,
		     nameIllegal,
		     "Directory.removeFile",
		     "illegal file name",
		     &IOArg(1)));
  }
  
   rc = unlink(s);

  if (rc != 0)
    throwErrno("Directory.removeFile", TRUE, NO_HANDLE, &IOArg(1));
  IOReturn(nameUnit);
}

/* Pair of macros for creating temporary strings */
# define ALLOC_STRING(x) (String)alloca(sizeof(char)*(x + 1))
# define FREE_STRING(x)

primFun(primRenameDirectory) { /* rename a directory	   */
  int rc;
  String tmpStr;
  String to;
  String from;

  tmpStr = evalName(IOArg(1));
  if (!tmpStr) {
    IOFail(mkIOError(NULL,
		     nameIllegal,
		     "Directory.renameDirectory",
		     "illegal directory name",
		     &IOArg(1)));
  }
  to = ALLOC_STRING(strlen(tmpStr));
  strcpy(to, tmpStr);

  from = evalName(IOArg(2));
  if (!from) {
    FREE_STRING(to);
    IOFail(mkIOError(NULL,
		     nameIllegal,
		     "Directory.renameDirectory",
		     "illegal directory name",
		     &IOArg(2)));
  }

  rc = rename(from,to);

  FREE_STRING(to);

  if (rc != 0)
    throwErrno("Directory.renameDirectory", FALSE, NO_HANDLE, &IOArg(1));
  IOReturn(nameUnit);
}

primFun(primRenameFile) { /* rename a file	   */
  int rc;
  String tmpStr;
  String to;
  String from;

  tmpStr = evalName(IOArg(1));
  if (!tmpStr) {
    IOFail(mkIOError(NULL,
		     nameIllegal,
		     "Directory.renameFile",
		     "illegal file name",
		     &IOArg(1)));
  }
  to = ALLOC_STRING(strlen(tmpStr));
  strcpy(to, tmpStr);

  from = evalName(IOArg(2));
  if (!from) {
    FREE_STRING(to);
    IOFail(mkIOError(NULL,
		     nameIllegal,
		     "Directory.renameFile",
		     "illegal file name",
		     &IOArg(2)));
  }

  if (isDirectory(from)) {
    FREE_STRING(to);
    IOFail(mkIOError(NULL,
		     namePermDenied,
		     "Directory.renameFile",
		     "is a directory",
		     &IOArg(2)));
  }

  if (isDirectory(to)) {
    FREE_STRING(to);
    IOFail(mkIOError(NULL,
		     namePermDenied,
		     "Directory.renameFile",
		     "is a directory",
		     &IOArg(1)));
  }

  rc = rename(from,to);
  
  FREE_STRING(to);

  if (rc != 0)
    throwErrno("Directory.renameFile", TRUE, NO_HANDLE, &IOArg(1));
  IOReturn(nameUnit);
}


primFun(primGetDirectory) { /* IO String - get current directory. */
  char buffer[FILENAME_MAX+1];
  if ((char*)(getcwd(buffer,FILENAME_MAX)) == (char*)NULL)
    throwErrno("Directory.getCurrentDirectory", FALSE, NO_HANDLE, NULL);
  pushString(buffer);
  IOReturn(pop());
}

primFun(primSetDirectory) { /* String -> IO () - set current directory. */
  int rc;
  String s = evalName(IOArg(1));

  if (!s) {
    IOFail(mkIOError(NULL,
		     nameIllegal,
		     "Directory.setCurrentDirectory",
		     "illegal directory name",
		     &IOArg(1)));
  }
  
   rc = chdir(s);

   if (rc != 0)
      throwErrno("Directory.setCurrentDirectory", FALSE, NO_HANDLE, &IOArg(1));
   IOReturn(nameUnit);
}

primFun(primFileExist) { /* FilePath -> IO Bool - check to see if file exists. */
  int rc;
  String s = evalName(IOArg(1));
  struct stat st;

  if (!s) {
    IOFail(mkIOError(NULL,
		     nameIllegal,
		     "Directory.doesFileExist",
		     "illegal file name",
		     &IOArg(1)));
  }
  
  rc = stat(s, &st);
  
  IOBoolResult(rc == 0 && !S_ISDIR(st.st_mode) );
}

primFun(primDirExist) { /* FilePath -> IO Bool - check to see if directory exists. */
  String s = evalName(IOArg(1));

  if (!s) {
    IOFail(mkIOError(NULL,
		     nameIllegal,
		     "Directory.doesDirectoryExist",
		     "illegal directory name",
		     &IOArg(1)));
  }
  IOBoolResult(isDirectory(s));
}

static Bool local isDirectory(s)
String s; {
  int rc;
  struct stat st;
#ifdef _WIN32
  /* For whatever reason, stat()ing a directory name
   * like "foo/" returns an error, while both "foo" and "foo/."
   * is fine. We want them all to be treated equal.
   */
  int len = strlen(s);
  while (len > 0 && 
	 (s[len-1] == '/' || s[len-1] == '\\')) {
      s[len-1] = '\0';
      len--;
  }
#endif
  rc = stat(s, &st);
  return (rc==0 && S_ISDIR(st.st_mode));
}

primFun(primGetPermissions) { /* FilePath -> IO (Bool,Bool,Bool,Bool) */
  int rc;
  String s = evalName(IOArg(1));
  struct stat st;
  int isR, isW, isX;
  

#if __MWERKS__ && macintosh
    IOFail(mkIOError(NULL,
		     nameIllegal,
		     "Directory.primGetPermissions",
		     "operation not supported",
		     &IOArg(1)));
#else
  if (!s) {
    IOFail(mkIOError(NULL,
		     nameIllegal,
		     "Directory.getPermissions",
		     "illegal file name",
		     &IOArg(1)));
  }
  
  isR = access(s, R_OK);
  isW = access(s, W_OK);
  isX = access(s, X_OK);
  rc = stat(s, &st);
  
  if (rc != 0)
    throwErrno("Directory.getPermissions", FALSE, NO_HANDLE, &IOArg(1));
  IOReturn(ap(ap(ap(ap( mkTuple(4),
			ToBool(isR == 0)),
		    ToBool(isW == 0)),
		 ToBool(isX == 0 && !S_ISDIR(st.st_mode))),
	      ToBool(isX == 0 && S_ISDIR(st.st_mode))));
#endif
}

#define EVAL_BOOL(x,y) \
   eval(y);\
   if (whnfHead==nameTrue) { \
      x = TRUE; \
   } else if (whnfHead==nameFalse) { \
      x = FALSE; \
   } else { \
      IOFail(mkIOError(NULL, \
	     nameIllegal, \
	     "Directory.setPermissions", \
	     "illegal flag", \
	     NULL)); \
   }

#ifdef _MSC_VER
#define READ_FLAG   S_IREAD
#define WRITE_FLAG  S_IWRITE
#define EXEC_FLAG   S_IEXEC
#else
#define READ_FLAG   S_IRUSR
#define WRITE_FLAG  S_IWUSR
#define EXEC_FLAG   S_IXUSR
#endif

#define SET_CHMOD_FLAG(x,y)  (x ? y : 0)

primFun(primSetPermissions) { /* FilePath -> Bool -> Bool -> Bool -> Bool -> IO () */
  int rc;
  String str;
  struct stat st;

  Bool   r;
  Bool   w;
  Bool   e;
  Bool   s;
  
#if __MWERKS__ && macintosh
    IOFail(mkIOError(NULL,
		     nameIllegal,
		     "Directory.primSetPermissions",
		     "operation not supported",
		     &IOArg(1)));
#else
  EVAL_BOOL(s, IOArg(1));
  EVAL_BOOL(e, IOArg(2));
  EVAL_BOOL(w, IOArg(3));
  EVAL_BOOL(r, IOArg(4));
  
  str = evalName(IOArg(5));
  
  if (!str) {
    IOFail(mkIOError(NULL,
		     nameIllegal,
		     "Directory.setPermissions",
		     "illegal file name",
		     &IOArg(5)));
  }

  rc = stat(str, &st);
  if (rc == 0)
    rc = chmod(str,
	       (st.st_mode & ~(READ_FLAG|WRITE_FLAG|EXEC_FLAG)) |
	       SET_CHMOD_FLAG(r, READ_FLAG)  |
	       SET_CHMOD_FLAG(w, WRITE_FLAG) |
	       SET_CHMOD_FLAG(e||s, EXEC_FLAG));
	     
  if (rc != 0)
    throwErrno("Directory.setPermissions", TRUE, NO_HANDLE, &IOArg(5));
  IOReturn(nameUnit);
  
#endif
}

/* Pedantically remove these local defs. */
#undef READ_FLAG
#undef WRITE_FLAG
#undef EXEC_FLAG
#undef SET_CHMOD_FLAG

primFun(primGetDirContents) { /* FilePath -> IO [FilePath] */
  /* opendir() / readdir() implementation. */
  DIR* dir;
  struct dirent* pDir;
  Cell ls;
  String fName = evalName(IOArg(1));
  
  if (!fName) {
    IOFail(mkIOError(NULL,
		     nameIllegal,
		     "Directory.getDirectoryContents",
		     "illegal file name",
		     &IOArg(1)));
  }
  
  dir = opendir(fName);
  
  if (dir == NULL)
    throwErrno("Directory.getDirectoryContents", FALSE, NO_HANDLE, &IOArg(1));

  ls = nameNil;
  
  /* To ensure that the test below doesn't
     succeed just because the impl of readdir()
     'forgot' to reset 'errno', do it ourselves. */
  errno = 0;
  
  while ( (pDir = readdir(dir)) ) {
    Cell n;
    push(ls);
    pushString(pDir->d_name);
    n = ap(nameCons, pop());
    ls = ap(n, pop());
  }
  
  if (errno != 0) {
    int rc = errno;
    closedir(dir);
    errno = rc;
    throwErrno("Directory.getDirectoryContents", FALSE, NO_HANDLE, &IOArg(1));
  }

  closedir(dir);
  IOReturn(ls);
}

primFun(primGetModTime) { /* FilePath -> IO Int{-time_t-} - get the mod. time of the file/directory. */
  int rc;
  String s = evalName(IOArg(1));
  struct stat st;

  if (!s) {
    IOFail(mkIOError(NULL,
		     nameIllegal,
		     "Directory.getModificationTime",
		     "illegal file name",
		     &IOArg(1)));
  }
  
  rc = stat(s, &st);
  
  if (rc < 0)
    throwErrno("Directory.getModificationTime", TRUE, NO_HANDLE, &IOArg(1));
  IOReturn(mkInt(st.st_mtime));
}
#endif

/* --------------------------------------------------------------------------
 * Error catching primitives
 * (not standard Haskell but jolly useful)
 * ------------------------------------------------------------------------*/

#if INTERNAL_PRIMS
/* --------------------------------------------------------------------------
 * Implementation of primitives which provide access to internal Hugs
 * data structures and representations from within Haskell.
 *
 * Despite appearances, the only primitives which break referential
 * transparency are the pointer equality primitives.
 *
 * The Hugs 98 system is Copyright (c) Mark P Jones, Alastair Reid, the
 * Yale Haskell Group, and the OGI School of Science & Engineering at OHSU,
 * 1994-2003, All rights reserved.  It is distributed as free software under
 * the license in the file "License", which is included in the distribution.
 *
 * $RCSfile: interns.c,v $
 * $Revision: 1.14 $
 * $Date: 2003/11/19 21:49:48 $
 * ------------------------------------------------------------------------*/
 
/* --------------------------------------------------------------------------
 * internalPrims control
 * ------------------------------------------------------------------------*/

static Void linkInternals (Void);

static Name nameLeft;			/* standard data constructors      */
static Name nameRight;			

static Void local internalControl (Int);
static Void local internalControl(what)
Int what; {
    switch (what) {
	case INSTALL : 
#define predef(nm,str) nm=newName(findText(str),NIL); name(nm).defn=PREDEFINED
		       setCurrModule(modulePrelude);
		       predef(nameLeft,       "Left");
		       predef(nameRight,      "Right");
#undef predef
		       break;

	case RESET :   linkInternals();
		       break;
    }
}

PROTO_PRIM(primGetCell);
PROTO_PRIM(primCellPtrEq);
PROTO_PRIM(primCatchError2);
PROTO_PRIM(primClassifyCell);
PROTO_PRIM(primNameInfo);
PROTO_PRIM(primNameEq);
PROTO_PRIM(primNameString);

#if BYTECODE_PRIMS
PROTO_PRIM(primNameCode);
PROTO_PRIM(primIntAt);
PROTO_PRIM(primFloatAt);
PROTO_PRIM(primDoubleAt);
PROTO_PRIM(primCellAt);
PROTO_PRIM(primNameAt);
PROTO_PRIM(primBytecodeAt);
PROTO_PRIM(primTextAt);
PROTO_PRIM(primAddrAt);
#endif /* BYTECODE_PRIMS */

static struct primitive internalPrimTable[] = {
  {"getCell",       1, primGetCell},
  {"cellPtrEq",     2, primCellPtrEq},
  {"catchError2",   1, primCatchError2},
  {"classifyCell",  2+IOArity, primClassifyCell},
  {"nameString",    1, primNameString},
  {"nameInfo",      1, primNameInfo},
  {"nameEq",        2, primNameEq},

#if BYTECODE_PRIMS
  {"nameCode",      1, primNameCode},
  {"intAt",         1, primIntAt},
  {"floatAt",       1, primFloatAt},
  {"doubleAt",      1, primDoubleAt},
  {"cellAt",        1, primCellAt},
  {"nameAt",        1, primNameAt},
  {"bytecodeAt",    1, primBytecodeAt},
  {"textAt",        1, primTextAt},
  {"addrAt",        1, primAddrAt},
#endif /* BYTECODE_PRIMS */

  {0,		    0, 0}
};

static struct primInfo internalPrims = {internalControl, internalPrimTable, 0};

/* --------------------------------------------------------------------------
 * linkInternals
 * ------------------------------------------------------------------------*/

static Module moduleInternals;

static Name nameHugsApply;            /* data CellKind = Apply Cell [Cell] */
static Name nameHugsFun;              /*    	       | Fun   Name        */
static Name nameHugsCon;              /*    	       | Con   Name        */
static Name nameHugsTuple;            /*    	       | Tuple Int         */
static Name nameHugsInt;              /*    	       | Int   Int         */
static Name nameHugsInteger;          /*    	       | Integer Integer   */
static Name nameHugsFloat;            /*    	       | Float Float       */
static Name nameHugsDouble;           /*    	       | Double Double     */
static Name nameHugsChar;             /*    	       | Char  Char        */
static Name nameHugsPrim;             /*    	       | Prim  String      */
static Name nameHugsError;            /*    	       | Error Cell        */

static Void linkInternals() {
    String internLib = "Hugs.Internals";
    moduleInternals = findModule(findText(internLib));
    if (nonNull(moduleInternals)) {
	setCurrModule(moduleInternals);
	nameHugsApply   = findName(findText("Apply"));
	nameHugsFun     = findName(findText("Fun"));
	nameHugsCon     = findName(findText("Con"));
	nameHugsTuple   = findName(findText("Tuple"));
	nameHugsInt     = findName(findText("Int"));
	nameHugsInteger = findName(findText("Integer"));
	nameHugsFloat   = findName(findText("Float"));
	nameHugsDouble  = findName(findText("Double"));
	nameHugsChar    = findName(findText("Char"));
	nameHugsPrim    = findName(findText("Prim"));
	nameHugsError   = findName(findText("Error"));
     
	if (isNull(nameHugsApply)   
	    || isNull(nameHugsFun)     
	    || isNull(nameHugsCon)     
	    || isNull(nameHugsTuple)   
	    || isNull(nameHugsInt)     
	    || isNull(nameHugsInteger) 
	    || isNull(nameHugsFloat)   
	    || isNull(nameHugsDouble)   
	    || isNull(nameHugsChar)    
	    || isNull(nameHugsPrim)    
	    || isNull(nameHugsError)
	    ) {
		fatal("module HugsInternals doesn't define CellKind correctly");
	}
    }
}

/* --------------------------------------------------------------------------
 * Operations on Cells
 * ------------------------------------------------------------------------*/

/* Cells have to be boxed (using the HUGSOBJECT tag) so that we can        */
/* evaluate a Cell expression such as "fst (cell1, cell2)" without         */
/* evaluating the thunk inside the Cell.                                   */

#define isHugsObject(a)     (isPair(a) && fst(a)==HUGSOBJECT)
#define checkHugsObject(x)  \
  if (!isHugsObject(x)) internal("HugsObject expected")

#define HugsObjectArg(nm, offset)                  \
   eval(primArg(offset));                          \
   checkHugsObject(whnfHead);                      \
   nm = snd(whnfHead);

primFun(primGetCell) {                 /* Trivial coercion                 */
    updapRoot(HUGSOBJECT,primArg(1));  /*  :: a -> Cell                    */
}


/* --------------------------------------------------------------------------
 * Pointer equality on Cells
 * ------------------------------------------------------------------------*/

primFun(primCellPtrEq) {	       /* Unsafe pointer equality test     */
    Cell x = 0;
    Cell y = 0;
    HugsObjectArg(x,1);
    push(x);
    HugsObjectArg(y,2);
    x = pop();

    /* We chase indirection chains since that's easy and gives more
     * accurate results.  
     * Don't do this if we use this function during error recovery -
     * it might be possible for an infinite loop to occur.
     */
    while (isPair(x) && INDIRECT == fst(x))
	x = snd(x);
    while (isPair(y) && INDIRECT == fst(y))
	y = snd(y);
    updateRoot( (x==y) ? nameTrue : nameFalse );
}

/* --------------------------------------------------------------------------
 * Error catching - evaluate a Cell and return an error redex or its value
 * ------------------------------------------------------------------------*/

primFun(primCatchError2) {	       /* Error catching  primitive        */
				       /*  :: a -> Either Cell a           */
    Cell err = NIL;
    err = evalWithNoError(primArg(1)); 
    if (isNull(err)) {
	updapRoot(nameRight, primArg(1));
    } else {
	updapRoot(nameLeft, ap(HUGSOBJECT, err));
    }
}

/* --------------------------------------------------------------------------
 * Examine an object either with or without evaluation.
 *
 * This primitive lets Haskell programs examine values of any datatype
 * (probably to print them).  It is wired into the IO monad to (narrowly)
 * avoid breaking referential transparency.
 * ------------------------------------------------------------------------*/

/* --------------------------------------------------------------------------
 * Note: we can't force the object _outside_ classify for two reasons: 
 *									     
 * 1) Writing "strict classifyCell x" only causes x to be evaluated to 
 *    WHNF.  Since x is a Cell, this doesn't achieve very much.	     
 *									     
 * 2) We can overcome this by adding an operation 			     
 *									     
 *      "forceCell :: Cell -> Cell" 			     
 *									     
 *    and writing "classifyObject (forceObject x)" but, if "x" is a CAF, 
 *    then "x" will be updated but the indirection via "x" will remain  
 *    in place.								     
 * ------------------------------------------------------------------------*/

primFun(primClassifyCell) {          /* classifyCell                       */
    Bool gc = consGC;                /*  :: Bool -> Cell -> IO CellKind    */
    Bool strict;
    Cell result = NIL;
    BoolArg(strict,2+IOArity);
    if (strict) {
	Cell temp;
	eval(IOArg(1)); /* object */
	checkHugsObject(whnfHead);
	whnfHead = snd(whnfHead);
	if (nonNull(temp = evalWithNoError(whnfHead))) {
	    IOReturn(ap(nameHugsError,ap(HUGSOBJECT,temp)));
	}
    } else {
	eval(IOArg(1)); /* object */
	checkHugsObject(whnfHead);
	whnfHead = snd(whnfHead);
	unwind(whnfHead);
    }
    consGC = TRUE;
    if (whnfArgs > 0) {
	Cell args = nameNil;
	while (whnfArgs-- > 0) 
	    args = ap(ap(nameCons,
			 ap(HUGSOBJECT, pushed(whnfArgs))),
			 args);
	result = ap(ap(nameHugsApply, 
		       ap(HUGSOBJECT,whnfHead)), 
		       args);
    } else {
	switch (whatIs(whnfHead)) {

	    case NAME     : result = ap((isCfun(whnfHead) ? nameHugsCon 
							  : nameHugsFun), 
					mkInt(whnfHead));
			    break;

	    case TUPLE    : result = ap(nameHugsTuple,mkInt(tupleOf(whnfHead)));
			    break;

#if BIGNUMS
	    case ZERONUM  : 
	    case POSNUM   : 
	    case NEGNUM   : 
			    result = ap(nameHugsInteger, whnfHead);
			    break;
#endif

	    case INTCELL  : result = ap(nameHugsInt, whnfHead);
			    break;

	    case CHARCELL : result = ap(nameHugsChar, whnfHead);
			    break;

	    case FLOATCELL: result = ap(nameHugsFloat, whnfHead);
			    break;

	    case DOUBLECELL: result = ap(nameHugsDouble, whnfHead);
			    break;

#if IO_HANDLES
	    case HANDCELL : result = ap(nameHugsPrim, mkStr(findText("{handle}")));
			    break;
#endif

#if IO_REFS
	    case MUTVAR   : result = ap(nameHugsPrim, mkStr(findText("{ref}")));
			    break;
#endif

#if HASKELL_ARRAYS
	    case ARRAY    : result = ap(nameHugsPrim, mkStr(findText("{primitive array}")));
			    break;
#endif

	    case HUGSOBJECT : 
			    /* ToDo: fix so that debugger can examine itself */
			    result = ap(nameHugsPrim, mkStr(findText("{Cell ...}")));
			    break;

	    case INSTANCE : result = ap(nameHugsPrim, mkStr(findText("{instance}")));
			    break;
#if OBSERVATIONS
            case OBSERVE  : result = ap(nameHugsPrim, mkStr(findText("{observe marker}")));
                            break;
#endif
	    default       : internal("Error in graph2");
			    break;
	}
    }
    consGC = gc;
    IOReturn(result);
}

/* --------------------------------------------------------------------------
 * Operations on Names
 * ------------------------------------------------------------------------*/

primFun(primNameString) {              /* Get string of a name             */
  Text t;                              /*  :: Name -> String              */
  Name nm;
  IntArg(nm,1);
  t = name(nm).text;
  if (0 <= t && t < NUM_TEXT) {
      /* ToDo: replace updateRoot(mkStr(..)) with updapRoot(STRCELL,..) */
      updateRoot(mkStr(t));
  } else { 
      /* If textToStr generates result on the fly, we build the entire
       * string now.
       */
      pushString(textToStr(t));
      updateRoot(pop());
  }      
}

primFun(primNameInfo) {                /* Get arity and fixity of a Name   */
  Name nm;                             /*  :: Name -> (Int,Int,Char)       */
  Syntax syn = 0;
  Char assoc = 0;  
  IntArg(nm,1);
  syn = syntaxOf(nm);
  switch (assocOf(syn)) {
  case APPLIC    : assoc = 'A'; break;
  case LEFT_ASS  : assoc = 'L'; break;
  case RIGHT_ASS : assoc = 'R'; break;
  case NON_ASS   : assoc = 'N'; break;
  }
  updapRoot(ap(ap(mkTuple(3),
	    mkInt(name(nm).arity)),
	    mkInt(precOf(syn))),
	    mkChar(assoc));
}

primFun(primNameEq) {                  /* Compare names                    */
  Name nm1;                            /*  :: Name -> Name -> Bool         */
  Name nm2;                    
  IntArg(nm1,2);
  IntArg(nm2,1);
  BoolResult(nm1==nm2);
}

#if BYTECODE_PRIMS

primFun(primNameCode) {                /* Obtain address of first bytecode */
  Name nm;                             /*  :: Name -> Addr                 */
  IntArg(nm,1);
  IntResult(name(nm).code);
}

#endif /* BYTECODE_PRIMS */

/* --------------------------------------------------------------------------
 * Operations on Addresses/Bytecodes
 * ------------------------------------------------------------------------*/

#if BYTECODE_PRIMS

primFun(primIntAt) {                  /* Obtain Int at address             */
  Addr m;                             /*  :: Addr -> Int                   */
  IntArg(m,1);
  IntResult(IntAt(m));
}

primFun(primFloatAt) {                /* Obtain float at address           */
  Addr m;                             /*  :: Addr -> Cell                  */
  IntArg(m,1);
  FloatResult(FloatAt(m));
}

primFun(primDoubleAt) {               /* Obtain float at address           */
  Addr m;                             /*  :: Addr -> Cell                  */
  IntArg(m,1);
  DoubleResult(DoubleAt(m));
}

primFun(primCellAt) {                 /* Obtain cell at address            */
  Addr m;                             /*  :: Addr -> Cell                  */
  IntArg(m,1);
  updapRoot(HUGSOBJECT,CellAt(m));
}

primFun(primNameAt) {                 /* Obtain Name at address            */
  Addr m;                             /*  :: Addr -> Name                  */
  IntArg(m,1);
  IntResult(CellAt(m));
}

primFun(primBytecodeAt) {             /* Obtain bytecode at address        */
  Addr m;                             /*  :: Addr -> Bytecode              */
  IntArg(m,1);
  IntResult(InstrAt(m));
}

primFun(primTextAt) {                 /* Obtain string at address          */
  Addr m;                             /*  :: Addr -> String                */
  IntArg(m,1);
  updapRoot(STRCELL,TextAt(m));
}

primFun(primAddrAt) {                 /* Obtain address at address         */
  Addr m;                             /*  :: Addr -> Addr                  */
  IntArg(m,1);
  IntResult(AddrAt(m));
}

#endif /* BYTECODE_PRIMS */

/*-------------------------------------------------------------------------*/
#endif

/* --------------------------------------------------------------------------
 * Array primitives:
 * ------------------------------------------------------------------------*/

#if HASKELL_ARRAYS
/* --------------------------------------------------------------------------
 * Haskell array primitives.
 *
 * The Hugs 98 system is Copyright (c) Mark P Jones, Alastair Reid, the
 * Yale Haskell Group, and the OGI School of Science & Engineering at OHSU,
 * 1994-2003, All rights reserved.  It is distributed as free software under
 * the license in the file "License", which is included in the distribution.
 *
 * $RCSfile: array.c,v $
 * $Revision: 1.10 $
 * $Date: 2004/01/06 10:02:51 $
 * ------------------------------------------------------------------------*/

static Name nameEltUndef;		/* undefined element in array	   */

PROTO_PRIM(primArray);
PROTO_PRIM(primUpdate);
PROTO_PRIM(primAccum);
PROTO_PRIM(primAccumArray);
PROTO_PRIM(primAmap);
PROTO_PRIM(primSubscript);
PROTO_PRIM(primBounds);
PROTO_PRIM(primElems);
PROTO_PRIM(primEltUndef);
#if IO_MONAD
PROTO_PRIM(primIONewArr);
PROTO_PRIM(primIOReadArr);
PROTO_PRIM(primIOWriteArr);
PROTO_PRIM(primIOFreeze);
PROTO_PRIM(primIOArrEq);
#endif

static struct primitive arrayPrimTable[] = {
  {"primArray",		3, primArray},
  {"primUpdate",	2, primUpdate},
  {"primAccum",		3, primAccum},
  {"primAccumArray",	5, primAccumArray},
  {"primAmap",		2, primAmap},
  {"primSubscript",	2, primSubscript},
  {"primBounds",	1, primBounds},
  {"primElems",		1, primElems},
  {"eltUndef",		0, primEltUndef},
#if IO_MONAD
  {"IONewArr",		3+IOArity, primIONewArr},
  {"IOReadArr",		2+IOArity, primIOReadArr},
  {"IOWriteArr",	3+IOArity, primIOWriteArr},
  {"IOFreeze",		1+IOArity, primIOFreeze},
  {"IOBounds",	        1, primBounds},
  {"IOArrEq",	        2, primIOArrEq},
#endif
  {0,			0, 0}
};

static Void outOfBounds (void);
static void outOfBounds() {
    throwException(ap(nameArrayException, ap(nameIndexOutOfBounds, nameNil)));
}

/* --------------------------------------------------------------------------
 * Array control:
 * ------------------------------------------------------------------------*/

static Void arrayControl (Int);
static Void arrayControl(what)
Int what; {
    switch (what) {
	case INSTALL : 
		       setCurrModule(modulePrelude);
#define pFun(n,s,t)    addPrim(0,n=newName(findText(s),NIL),t,modulePrelude,NIL)
		       pFun(nameEltUndef,  "_undefined_array_element",
							"eltUndef");
#undef pFun
		       break;
    }
}

static struct primInfo arrayPrims = { arrayControl, arrayPrimTable, 0 };

/*-------------------------------------------------------------------------*/

/* The implementation of arrays is heavily parameterized to allow the
 * use of different implementations.  Non-conservative GC is also an
 * important goal, which is also why so much of this was originally done
 * using macros rather than procedures.  As it happens, this probably could
 * have been avoided, but there don't seem to be sufficiently good reasons
 * to warrant changing it.
 *
 * The result, however, is a torture-test for the C preprocessor!
 *
 * A description of the various `parameters' follows:
 *
 * Primitives that build a new array use the macro:
 * declArr;		Allocate slot on stack to hold a freshly created
 *			array that will be seen by the garbage collector.
 *			The value of the array can subsequently be referred
 *			to using the `arr' macro.  The declArr macro also
 *			declares a local Int variable, alen, to hold the
 *			length of the array.
 *
 * There are four methods for creating a new array, all of which return
 * the intermediate array in arr and its length in alen:
 *
 * aNewSet(b,s,v);	Allocate new array with bounds b and size s.
 *			Data elements set to v.
 * aNewNil(b,s);	Equivalent to aNewSet(b,s,NIL), treated separately
 *			because it is possible to use more efficient code
 *			for this special case in some implementations.
 * aNewCopy(a);		Builds an exact copy of array a, which can then be
 *			modified destructively, without changing a.
 *			Note that this forces evaluation of a.
 * aNewLike(a,v);	Builds an array of the same size and bounds as a
 *			with each element initialized to v.
 *			Note that this forces evaluation of a.
 *
 * All four of these methods are implemented using macros; the b, r, a
 * parameters are integers, identifying particular primArg(x) slots.
 * The v parameters should be constants, unmovable by GC, or primArg(x)
 * references that can be safely modified during GC.
 *
 * Other functions are:
 *
 * aEvalModel(a);	Evaluate model array primArg(a), and overwrite it
 *			on stack with an indirection free pointer to the
 *			resulting array.
 * aAssocs(as,p);	Move list of assocs -- (offset,value) pairs -- from
 *			primArg(as) (which is NIL'd to prevent space leak)
 *			to top of stack and evaluate, in sequence, until all
 *			assocs have been processed.  For each pair, we
 *			run procedure p with the offset in whnfInt and the
 *			associated value in top(), to be popped before p is
 *			done.
 * aSetElt;		To be used with aAssocs: if arr[whnfInt] is NIL,
 *			set it to top(), otherwise set to undefined.
 * aAddElt(f);		To be used with aAssocs: replace whnfInt element e
 *			of arr with ap(ap(primArg(f),e),top())
 * aNullElts;		Set any null elements in arr to nameEltUndef.
 * aCopyNull(a);	Replace any null elements in arr with corresponding
 *			values in array primArg(a).
 * aMapElts(f);		Replace every element e in arr with ap(primArg(f),e).
 * aGetElt(a);		Push value of whnfInt'th element of primArg(a).
 * aPutElt(a,v);	Put v into whnfInt'th slot of primArg(a).
 * aElems(a);		Evaluate array at primArg(a), and return its list of
 *			elements on top of stack in reverse order, backed onto
 *			NIL (ready for revOnto(top(),nameNil)).
 * aBounds()		Extract bounds from arr.
 * aGetBounds(a)	Extract bounds from primArg(a).
 *
 * There is no guarantee that the representation used for arr will be the
 * same as for any other array.  The following methods do however ensure
 * that the standard representation is used when a value is finally returned:
 *
 * updarrRoot();	Updates root of redex with array represented by arr.
 *			(Should also reset arr to avoid space leaks.)
 * aRetForIO();		Update root to return an array from IO monad;
 *			i.e. pass arr to the continuation.
 */

#define declArr		StackPtr arrPos=sp+1; Int alen; push(NIL)
#define arr		stack(arrPos)

#define aNewNil(b,s)	aNewSet(b,s,NIL)
#define aNewSet(b,s,v)	{   Int i;				\
			    eval(primArg(s));			\
			    alen = (whnfInt>0)?whnfInt:0;	\
			    for (arr=NIL, i=alen; i>0; i--)	\
				arr = ap(v,arr);		\
			    arr = ap(primArg(b),arr);		\
			}
#define aNewCopy(a)	{   Cell es = snd(primArg(a));		\
			    for (arr=ap(hd(es),NIL), alen=0;	\
				 nonNull(es=tl(es)); ++alen)	\
				arr = ap(hd(es),arr);		\
			    arr = rev(arr);			\
			}
#define aNewLike(a,v)	{   Cell es = snd(primArg(a));		\
			    for (arr=ap(hd(es),NIL), alen=0;	\
				 nonNull(es=tl(es)); ++alen)	\
				arr = ap(v,arr);		\
			    arr = rev(arr);			\
			}
#define aEvalModel(a)	eval(primArg(a)); primArg(a)=whnfHead
#define aSetElt		{   List us = snd(arr);			\
			    for (; 0<whnfInt--; us=tl(us));	\
			    hd(us) = isNull(hd(us))?top():nameEltUndef;\
			    drop();				\
			}
#define aAddElt(f)	{   List us = snd(arr);			\
			    for (; 0<whnfInt--; us=tl(us));	\
			    hd(us) = ap(primArg(f),hd(us));	\
			    hd(us) = ap(hd(us),pop());		\
			}
#define aNullElts	{   List us = snd(arr);			\
			    for (; nonNull(us); us=tl(us))	\
				if (isNull(hd(us)))		\
				    hd(us) = nameEltUndef;	\
			}
#define aCopyNull(a)	{   List us = snd(snd(primArg(a)));	\
			    List vs = snd(arr);			\
			    for (; nonNull(vs); vs=tl(vs), us=tl(us))\
				if (isNull(hd(vs)))		\
				    hd(vs) = hd(us);		\
			}
#define aMapElts(f)	{   List us = snd(arr);			\
			    for (; nonNull(us); us=tl(us))	\
				hd(us) = ap(primArg(f),hd(us));	\
			}
#define aGetElt(a)	{   List es = snd(snd(primArg(a)));	\
			    while (0<whnfInt--)			\
				es = tl(es);			\
			    push(hd(es));			\
			}
#define aPutElt(a,v)	{   List es = snd(snd(primArg(a)));	\
			    while (0<whnfInt--)			\
				es = tl(es);			\
			    hd(es) = v;				\
			}
#define aElems(a)	{   List us;				\
			    eval(primArg(a));			\
			    us = snd(snd(primArg(a)));		\
			    chkStack(2); onto(NIL); onto(NIL);	\
			    for(; nonNull(us); us=tl(us)) {	\
				top()     = ap(nameCons,hd(us));\
				pushed(1) = ap(top(),pushed(1));\
			    }					\
			    drop();				\
			}
#define aBounds()	fst(arr)
#define aGetBounds(a)	fst(snd(primArg(a)))
#define updarrRoot()	updapRoot(ARRAY,arr); arr=NIL
#define aRetForIO()	arr = ap(ARRAY,arr);			\
			updapRoot(primArg(1),arr);		\
			arr = NIL;

/* The implementation of aAssocs(as,p) should be independent of the
 * representation for arrays:
 */
#define aAssocs(as,p)	push(primArg(as)); primArg(as)=NIL;	\
			eval(pop());				\
			while (whnfHead==nameCons) {		\
			    eval(pop());			\
			    eval(top());			\
			    if (whnfInt<0 || whnfInt>=alen)	\
				outOfBounds();			\
			    drop(); p; eval(pop());		\
			}

/* Finally, we come to the implementation of the Haskell array primitives: */

primFun(primArray) {			/* :: (a,a)			   */
    declArr;				/*    -> Int			   */
    aNewNil(3,2);			/*	 -> [(Int,b)]		   */
    aAssocs(1,aSetElt);			/*	    -> Array a b	   */
    aNullElts;
    updarrRoot();
}

primFun(primUpdate) {			/* :: [(Int,b)]			   */
    declArr;				/*    -> Array a b		   */
    aEvalModel(1);			/*       -> Array a b		   */
    aNewLike(1,NIL);
    aAssocs(2,aSetElt);
    aCopyNull(1);
    updarrRoot();
}

primFun(primAccum) {			/* :: [(Int,c)] -> Array a b	   */
    declArr;				/*    -> (b -> c -> b)		   */
    aEvalModel(2);			/*	 -> Array a b		   */
    aNewCopy(2);
    aAssocs(3,aAddElt(1));
    updarrRoot();
}

primFun(primAccumArray) {		/* :: (a,a) -> Int		   */
    declArr;				/*    -> (b -> c -> b) -> b	   */
    aNewSet(5,4,primArg(2));		/*	 -> [(Int,c)]		   */
    aAssocs(1,aAddElt(3));		/*	    -> Array a b	   */
    updarrRoot();
}

primFun(primAmap) {			/* :: (a -> b)			   */
    declArr;				/*    -> Array c a		   */
    aEvalModel(1);			/*       -> Array c b		   */
    aNewCopy(1);
    aMapElts(2);
    updarrRoot();
}

primFun(primSubscript) {		/* :: Array a b -> Int -> b        */
    aEvalModel(2);
    eval(primArg(1));
    aGetElt(2);
    updateRoot(top());
}

primFun(primBounds) {			/* :: Array a b -> (a,a)	   */
    aEvalModel(1);
    updateRoot(aGetBounds(1));
}

primFun(primElems) {			/* :: Array a b -> [b]		   */
    aEvalModel(1);
    aElems(1);
    updateRoot(revOnto(top(),nameNil));
}

primFun(primEltUndef) {
    throwException(ap(nameArrayException, ap(nameUndefinedElement, nameNil)));
}

#if IO_MONAD
primFun(primIONewArr) {			/* :: (a,a)			   */
    declArr;				/*    -> Int			   */
    aNewSet(3+IOArity,2+IOArity,IOArg(1));/*	 -> b			   */
    aRetForIO();			/*	    -> IO (IOArray a b)	   */
}

primFun(primIOReadArr) {		/* :: IOArray a b -> Int -> IO b   */
    aEvalModel(2+IOArity);
    eval(primArg(1+IOArity));
    aGetElt(2+IOArity);
    IOReturn(top());
}

primFun(primIOWriteArr) {		/* :: IOArray a b -> Int -> b	   */
    aEvalModel(3+IOArity);		/*    -> IO ()			   */
    eval(primArg(2+IOArity));
    aPutElt(3+IOArity,IOArg(1));
    IOReturn(nameUnit);
}

primFun(primIOFreeze) {			/* :: IOArray a b		   */
    declArr;				/*    -> IO (Array a b)		   */
    aEvalModel(1+IOArity);
    aNewCopy(1+IOArity);
    aRetForIO();
}

primFun(primIOArrEq) {		        /* :: IOArray a b                 */
    aEvalModel(1);                      /*    -> IOArray a b -> Bool      */
    aEvalModel(2);
    BoolResult(primArg(1)==primArg(2));
}

#endif /* IO_MONAD */

/* Retire macros used in the implementation of arrays -------------------- */

#undef aNewSet
#undef aNewNil
#undef aNewCopy
#undef aNewLike
#undef aEvalModel
#undef aAssocs
#undef aSetElt
#undef aAddElt
#undef aNullElts
#undef aCopyNull
#undef aMapElts
#undef aGetElt
#undef aPutElt
#undef aElems
#undef aBounds
#undef aGetBounds
#undef updarrRoot
#undef aRetForIO

/*-------------------------------------------------------------------------*/
#endif

/* --------------------------------------------------------------------------
 * Char primitives:
 * ------------------------------------------------------------------------*/

CAFChar(primMaxChar,MAXCHARVAL)

Char2Bool(primIsUpper,isUpper(x))
Char2Bool(primIsLower,isLower(x))
Char2Bool(primIsAlpha,isAlpha(x))
Char2Bool(primIsAlphaNum,isAlphaNum(x))
Char2Bool(primIsPrint,isPrint(x))

Char2Char(primToLower,toLower(x))
Char2Char(primToUpper,toUpper(x))
Char2Char(primToTitle,toTitle(x))

Char2Int(primUniGenCat,uni_gencat(x))

/* --------------------------------------------------------------------------
 * Extensible records: (Gaster and Jones, 1996)
 * ------------------------------------------------------------------------*/

#if TREX
primFun(primRecExt) {                   /* :: Int -> a -> Rec ? -> Rec ?   */
    Int  n;
    Cell b = NIL;
    Cell r;
    eval(primArg(3));
    n = whnfInt;
    eval(primArg(1));
    for (r=arg(whnfHead); n>0; n--) {
	b = cons(fun(r),b);
	r = arg(r);
    }
    b = cons(primArg(2),b);
    updapRoot(RECORD,revOnto(b,r));
}

primFun(primRecBrk) {                   /* :: Int -> Rec ? -> (?, Rec ?)   */
    Int  n;
    Cell b;
    Cell r;
    eval(primArg(2));
    n = whnfInt;
    eval(primArg(1));
    b = cons(RECORD,NIL);
    for (r=arg(whnfHead); n>0; n--) {
	b = cons(fun(r),b);
	r = arg(r);
    }
    pushed(1) = revOnto(b,arg(r));
    pushed(0) = ap(mkTuple(2),fun(r));
    updapRoot(pushed(0),pushed(1));
}

primFun(primRecSel) {                   /* :: Int -> Rec ? -> ?            */
    Int  n;
    Cell r;
    eval(primArg(2));
    n = whnfInt;
    eval(primArg(1));
    for (r=arg(whnfHead); n>0; n--)
	r = arg(r);
    updateRoot(fun(r));
}

/* recShw :: primArg(5) Label l       ->
 *           primArg(4) ShowD a       ->
 *           primArg(3) Lacks_l r     ->
 *           primArg(2) ShowRecRowD r ->
 *           primArg(1) Rec (l::a|r)  -> [(String,ShowS)]
 * recShw l d e f r
 *    = case recBrk e r of
 *        (v,s) -> insertField l (showsPrec d 0 v) (showRecRow f s)
 */

primFun(primRecShw) {
    push(nameRecBrk);
    toparg(primArg(3));
    toparg(primArg(1));
    eval(pop());
    primArg(2) = ap(nameShowRecRow,primArg(2));
    primArg(4) = ap(nameShowsPrec,primArg(4));
    primArg(4) = ap(primArg(4),mkInt(0));
    primArg(5) = ap(nameInsFld,primArg(5));
    pushed(1)  = ap(primArg(2),pushed(1));
    pushed(0)  = ap(primArg(4),pushed(0));
    pushed(0)  = ap(primArg(5),pushed(0));
    updapRoot(pushed(0),pushed(1));
}

/* recEq :: primArg(6) Label l       ->
 *	    primArg(5) EqD a         ->
 *	    primArg(4) Lacks_x r     ->
 *	    primArg(3) EqRecRowD r   ->
 *	    primArg(2) Rec (l::a|r)  ->
 *	    primArg(1) Rec (l::a|r)  -> [(String,Bool)]
 * reqEq l eqa e eqr r1 r2
 *    = case recBrk e r1 of
 *	  (v,s1) -> case recBrk e r2 of
 *		      (w,s2) -> insertField l ((==) eqa v w)
 *						      (eqRecRow eqr s1 s2)
 */

primFun(primRecEq) {
    push(nameRecBrk);
    toparg(primArg(4));
    toparg(primArg(2));
    eval(pop());
    push(nameRecBrk);
    toparg(primArg(4));
    toparg(primArg(1));
    eval(pop());
    primArg(3) = ap(nameEqRecRow,primArg(3));
    primArg(3) = ap(primArg(3),pushed(3));
    primArg(3) = ap(primArg(3),pushed(1));
    primArg(5) = ap(nameEq,primArg(5));
    primArg(5) = ap(primArg(5),pushed(2));
    primArg(5) = ap(primArg(5),pushed(0));
    primArg(6) = ap(nameInsFld,primArg(6));
    primArg(6) = ap(primArg(6),primArg(5));
    updapRoot(primArg(6),primArg(3));
}
#endif

/* --------------------------------------------------------------------------
 * Auxilliary functions
 * ------------------------------------------------------------------------*/

static Cell local followInd(c)    /* follow chain of indirections and CAFs */
Cell c; {
    do {
	switch (whatIs(c)) {
	case INDIRECT : c = snd(c);
		break;
#if OBSERVATIONS
        case OBSERVE  : c = markedExpr(c);
                break;
#endif
	case NAME     : if (isCfun(c)
		|| name(c).arity != 0 
		|| isNull(name(c).defn)) {
		return c;
		}
		c = name(c).defn;
		break;
	default       : return c;
    }
    allowBreak();
    } while (1);
}
		   
/* --------------------------------------------------------------------------
 * Pointer equality
 * ------------------------------------------------------------------------*/

/* Pointer equality tests break referential transparency.
 * However, they can be useful in implementing referentially transparent
 * functions such as lazy memo-tables.
 *
 *   foo = cache sin
 *
 *   cache :: (a -> b) -> (a -> b)
 *   cache f = \x -> unsafePerformIO (check x)
 *    where
 *     ref = unsafePerformIO (newRef (error "cache", error "cache"))
 *     check x = derefRef ref >>= \ (x',a) ->
 *               if x `unsafePtrEq` x' then
 *                 return a
 *               else
 *                 let a = f x in
 *                 assignRef ref (x, a) >>
 *                 return a
 */

primFun(primUnsafePtrEq) {		 /* Unsafe pointer equality test     */
    Cell x = followInd(primArg(2));
    Cell y = followInd(primArg(1));
    updateRoot( (x==y) ? nameTrue : nameFalse );
}

/* Companion function for use when debugging uses of unsafePtrEq.
 * Converts a heap pointer to an Int so you can look at it.
 * I don't think there's any way of using this function that 
 * doesn't break the semantics - debugging use only.
 */
primFun(primUnsafePtrToInt) {
    updateRoot(mkInt(followInd(primArg(1))));
}

/*---------------------------------------------------------------------------
 * GreenCard entry points
 *
 * GreenCard generated code accesses Hugs data structures and functions 
 * (only) via these functions (which are stored in the virtual function
 * table hugsAPI4).
 *-------------------------------------------------------------------------*/

static void           getUnit        (void);
static HsInt          getInt         (void);
static HsWord         getWord        (void);
static HsAddr         getAddr        (void);
static char           getChar4       (void);
static HsChar         getChar        (void);
static HugsForeign    getForeign     (void);
static HsBool         getBool        (void);
static HsInt8         getInt8        (void);
static HsInt16        getInt16       (void);
static HsInt32        getInt32       (void);
static HsInt64        getInt64       (void);
static HsWord8        getWord8       (void);
static HsWord16       getWord16      (void);
static HsWord32       getWord32      (void);
static HsWord64       getWord64      (void);
static HsPtr          getPtr         (void);
static HsFunPtr       getFunPtr      (void);
static HsForeignPtr   getForeignPtr  (void);	      
static HsStablePtr    getStablePtr4  (void);
static HsFloat        getFloat       (void);
static HsDouble       getDouble      (void);

static HugsStablePtr  getStablePtr   (void);

static void           putInt         (HsInt);
static void           putWord        (HsWord);
static void           putAddr        (HsAddr);
static void           putChar4       (char);
static void           putChar        (HsChar);
static void           putForeign     (HugsForeign, void (*)(void *));
static void           putStablePtr4  (HsStablePtr);
static void           putBool        (HsBool);
static void           putInt8        (HsInt8);
static void           putInt16       (HsInt16);
static void           putInt32       (HsInt32);
static void           putInt64       (HsInt64);
static void           putWord8       (HsWord8);
static void           putWord16      (HsWord16);
static void           putWord32      (HsWord32);
static void           putWord64      (HsWord64);
static void           putPtr         (HsPtr);
static void           putFunPtr      (HsFunPtr);
static void           putForeignPtr  (HsForeignPtr);
static void           putFloat       (HsFloat);
static void           putDouble      (HsDouble);

static void           freeStablePtr4 (HsStablePtr);

	      
static void           returnIO       (HugsStackPtr, int);
static void           returnId       (HugsStackPtr, int);
static int            runIO          (int);
static void           apMany         (int);

static void           getUnit()      { eval(pop()); }
static HsInt          getInt()       { eval(pop()); checkInt();   return whnfInt; }
static HsWord         getWord()      { eval(pop()); checkWord();  return (unsigned int) whnfInt; }
static HsAddr         getAddr()      { eval(pop()); checkPtr();   return ptrOf(whnfHead); }
static char           getChar4()     { eval(pop()); checkChar();  return charOf(whnfHead); }
static HsChar         getChar()      { eval(pop()); checkChar();  return charOf(whnfHead); }
static HugsForeign    getForeign()   { eval(pop()); return derefMP(whnfHead); }
static HsBool         getBool()      { eval(pop()); checkBool();  return (whnfHead == nameTrue); }
static HsInt8         getInt8()      { eval(pop()); checkInt();   return whnfInt; } 
static HsInt16        getInt16()     { eval(pop()); checkInt();   return whnfInt; }
static HsInt32        getInt32()     { eval(pop()); checkInt();   return whnfInt; }
static HsInt64        getInt64()     { eval(pop()); return int64FromParts(intOf(fst(snd(whnfHead))), intOf(snd(snd(whnfHead)))); }
static HsWord8        getWord8()     { eval(pop()); checkWord();  return (unsigned int) whnfInt; } 
static HsWord16       getWord16()    { eval(pop()); checkWord();  return (unsigned int) whnfInt; } 
static HsWord32       getWord32()    { eval(pop()); checkWord();  return (unsigned int) whnfInt; } 
static HsWord64       getWord64()    { eval(pop()); return int64FromParts(intOf(fst(snd(whnfHead))), intOf(snd(snd(whnfHead)))); }
static HsPtr          getPtr()       { eval(pop()); checkPtr();   return ptrOf(whnfHead); }
static HsFunPtr       getFunPtr()    { eval(pop()); checkPtr();   return (HsFunPtr)ptrOf(whnfHead); }

static HsForeignPtr   getForeignPtr() {
    ERRMSG(0) "getForeignPtr: not implemented in Hugs"
    EEND;
    return 0;
}

static HugsStablePtr  getStablePtr() { 
    Cell c = mkStablePtr(pop());
    if (isNull(c)) {
	ERRMSG(0) "Stable pointer table full"
	EEND;
    }
    return c;
}

static HugsStablePtr  lookupName (String, String);
static HugsStablePtr  lookupName(q,n)
String q;
String n; { 
    Name nm = findQualFun(findText(q), findText(n));
    Cell c;
    
    if (isNull(nm)) {
	ERRMSG(0) "Can't find qualified name '%s.%s'", q, n
	EEND;
    }
    c = mkStablePtr(nm);
    if (isNull(c)) {
	ERRMSG(0) "Stable pointer table full"
	EEND;
    }
    return c;
}

static void putInt (HsInt  x) { push(mkInt(x)); }
static void putWord(HsWord x) { push(mkInt((int)x)); }
static void putAddr(HsAddr x) { push(mkPtr(x)); }
static void putChar4(char x) { push(mkChar(x)); }
static void putChar(HsChar x) { push(mkChar(x)); }
static void putForeign(HugsForeign x, void (*f)(HugsForeign)) { push(mkMallocPtr(x,f)); }
static void putStablePtr   (HugsStablePtr x) { push(derefStablePtr(x)); }
static void putBool        (HsBool x)        { push(x?nameTrue:nameFalse); }
                                            
static void putInt8 (HsInt8  x) { push(mkInt(x)); }
static void putInt16(HsInt16 x) { push(mkInt(x)); }
static void putInt32(HsInt32 x) { push(mkInt(x)); }
static void putInt64(HsInt64 x) { push(pair(I64CELL,pair(mkInt(part1Int64(x)),mkInt(part2Int64(x))))); }
static void putWord8 (HsWord8  x) { push(mkInt((int)x)); }
static void putWord16(HsWord16 x) { push(mkInt((int)x)); }
static void putWord32(HsWord32 x) { push(mkInt((int)x)); }
static void putWord64(HsWord64 x) { push(pair(I64CELL,pair(mkInt(part1Int64(x)),mkInt(part2Int64(x))))); }
static void putPtr   (HsPtr    x) { push(mkPtr(x)); }
static void putFunPtr(HsFunPtr x) { push(mkPtr((Pointer)x)); }

static void putStablePtr4(HsStablePtr   x) {
    push((HugsStablePtr)x);
}

static HsStablePtr getStablePtr4(void) { 
    HugsStablePtr x = pop();
    return (HsStablePtr)x;
}

static Void freeStablePtr4(HsStablePtr x) {
    if (x) freeStablePtr((HugsStablePtr)x);
}

static HsFloat        getFloat()     { eval(pop()); checkFloat(); return whnfFloat; }
static HsDouble       getDouble()    { eval(pop()); checkDouble(); return whnfDouble; }

static void putFloat(HsFloat x) {
  push(mkFloat(x));
}

static void putDouble(HsDouble x) {
  push(mkDouble(x));
}

static void putForeignPtr(HsForeignPtr x) {
    ERRMSG(0) "putForeignPtr: not implemented in Hugs"
    EEND;
}

static void returnIO(root,n) /* return in IO monad */
HugsStackPtr root;
int          n; {
    /* There should be n return values on the top of the stack */
    if (n == 0) {
	push(nameUnit);
    } else if (n == 1) {
       /* do nothing */
    } else {
    int i;
    push(mkTuple(n));
    for(i=0; i<n; ++i) {
	pushed(1) = ap(pushed(0),pushed(1));
	drop();
    }
    }
    IOReturn(top());
}

static void returnId(root,n) /* return in identity monad */
HugsStackPtr root;
int          n; {
    /* There should be n return values on the top of the stack */
    if (n == 0) {
	push(nameUnit);
    } else if (n == 1) {
        /* do nothing, already there. */
    } else {
	int i;
	push(mkTuple(n));
	for(i=0; i<n; ++i) {
	    pushed(1) = ap(pushed(0),pushed(1));
	    drop();
	}
    }
    /*
     * Note: have to be a bit careful when returning, since we could
     * be returning from evaluating a CAF. In the non-CAF case,
     * 'root' points to the object we've entered, so we can just
     * go ahead and update it. No problem there.
     *
     * For CAFs, the evaluator leaves 'root' pointing at the Name
     * of the CAF, so updating it directly is not going to work.
     * Instead, we just leave the result at the top of the stack & 
     * let the evaluator do what it has always done; clean up & 
     * update the 'defn' field inside the CAF Name.
     * 
     *  [6/01 --sof]
     */
    if ( isPair(stack(root))
#if 0 || DEBUG   
	 || (whatIs(stack(root)) == AP)
#endif
	 ) {
      updateRoot(top());
    }
}

static int runIO(n)
int n; {
    /* stack = argn : ... : arg1 : fun : rest */
    StackPtr old_sp   = sp - n - 1;
    Cell     temp     = NIL;
    Int i;
    /* build application node */
    for(i=n-1; i >= 0; --i) {
	pushed(n) = ap(pushed(n), pushed(i));
    }
    sp -= n;

    /* evaluate it - should have type IO a */
    temp = evalWithNoError(ap(nameIORun,pop()));
    if (nonNull(temp)) {
	ERRMSG(0) "runIO: uncaught error"
	EEND;
    }
    if (sp != old_sp+1) {
	ERRMSG(0) "runIO: unbalanced stack (%d)", sp-old_sp
	EEND;
    }
    if (whnfHead == nameRight) {
	return 0;
    } else if (whnfHead != nameLeft) { /* Called "exit" */
	ERRMSG(0) "runIO: bad return value"
	EEND;
    }
    return 1;
}    

static void apMany(n)
int n; {
    /* stack = argn : ... : arg1 : fun : rest */
    Int i;
    /* build application node */
    for(i=n-1; i >= 0; --i) {
	pushed(n) = ap(pushed(n), pushed(i));
    }
    sp -= n;
    /* stack = ap(...(ap(fun,arg1),...),argn) : rest */
}

static int runId (int);

static int runId(n)
int n; {
    apMany(n);
    top() = ap(nameReturnIO,top());
    return runIO(0);
}

/* This allocates a small object and writes some machine code into it.  
 *
 * The code generated by the generated code is equivalent to:
 *  
 *   rty f(ty1 a1, ... tym am) {
 *     return (*app)(s,a1, ... am);
 *   }
 *
 * Where s is a stable pointer (an int).
 *
 * But, because this is machine code, we can do it without knowing
 * anything about the argument types.  This works because the C
 * calling convention on most machines has the stack looking something
 * like this (where leftmost = top of stack)
 *
 *   ret_addr : a1 : ... am : rest of stack
 *
 * If this is not the case for some architecture/calling convention,
 * the easiest thing to do might be to make the stable pointer be
 * the last argument (if that is easily found) or to put it in a callee
 * saves register - and then adapt the apply function generated by 
 * implementForeignExport.  
 *
 * thunk->{next,prev}: a doubly linked list (makes deletion easy)
 * thunk->stable:      the stable pointer (makes deletion easy)
 * 
 * At the end of execution, we run down the list freeing every thunk
 * that the user did not explicitly deallocate.
 */

struct thunk_data {
    struct thunk_data* next;
    struct thunk_data* prev;
    HugsStablePtr      stable;
    char               code[1];
};

static void* mkThunk      (void (*)(void), HugsStablePtr);
static void freeThunkAux  (struct thunk_data*);
static void freeAllThunks (void);

static struct thunk_data* foreignThunks = 0;

static void* mkThunk(void (*app)(void), HugsStablePtr s) {
    ERRMSG(0) "Foreign import wrapper is not supported"
    EEND;
}

static void freeThunkAux(struct thunk_data* thunk) {
    freeStablePtr(thunk->stable);
    if (thunk->prev) {
        assert(foreignThunks != thunk);
        thunk->prev->next = thunk->next;
    } else {        
        assert(foreignThunks == thunk);
        foreignThunks = thunk->next;
    }        
    if (thunk->next) {
        thunk->next->prev = thunk->prev;
    }
    free(thunk);
}

static void freeAllThunks(void) {
    while (foreignThunks) {
        freeThunkAux(foreignThunks);
    }
}

/* This frees the object allocated by mkThunk.
 * [A useful debugging mode would not deallocate the thunk but would
 *  overwrite the thunk with code which prints an error message.]
 */
static void freeHaskellFunctionPtr(void* t) {
    struct thunk_data* thunk = (struct thunk_data*)((char*)t - (char*)&((struct thunk_data*)0)->code);
    freeThunkAux(thunk);
}

primFun(primFreeHFunPtr) {		/*  :: FunPtr a -> IO ()           */
    Pointer x;
    PtrArg(x,1+IOArity);
    freeHaskellFunctionPtr(x);
    IOReturn(nameUnit);
}

HugsAPI4* hugsAPI4() { /* build virtual function table */
    static HugsAPI4 api;
    static Bool initialised = FALSE;

    if (!initialised) {

        /* evaluate next argument */
      	api.getInt        = getInt;
	api.getWord       = getWord;
	api.getAddr       = getAddr;
	api.getFloat      = getFloat;
	api.getDouble     = getDouble;
	api.getChar       = getChar4;
	api.getForeign    = getForeign;
	api.getStablePtr  = getStablePtr;

	/* push part of result   */
	api.putInt        = putInt;
	api.putWord       = putWord;
	api.putAddr       = putAddr;
	api.putFloat      = putFloat;
	api.putDouble     = putDouble;
	api.putChar       = putChar4;
	api.putForeign    = putForeign;
	api.putStablePtr  = putStablePtr;

	/* return n values in IO monad or Id monad */
	api.returnIO      = returnIO;
        api.returnId      = returnId;
	api.runIO         = runIO;

	/* free a stable pointer */	    			 
	api.freeStablePtr = freeStablePtr;

	/* register the prim table */	    			 
	api.registerPrims = registerPrims;

	/* garbage collect */
	api.garbageCollect = garbageCollect;

	/* API3 additions follow */
        api.lookupName    = lookupName;
        api.ap            = apMany;
        api.getUnit       = getUnit;
        api.mkThunk       = mkThunk;
        api.freeThunk     = freeHaskellFunctionPtr;
	api.getBool       = getBool;
	api.putBool       = putBool;

	/* API4 additions follow */
        api.getInt8       = getInt8;
        api.getInt16      = getInt16;
        api.getInt32      = getInt32;
        api.getInt64      = getInt64;
        api.getWord8      = getWord8;
        api.getWord16     = getWord16;
        api.getWord32     = getWord32;
        api.getWord64     = getWord64;
        api.getPtr        = getPtr;
        api.getFunPtr     = getFunPtr;
        api.getForeignPtr = getForeignPtr;

        api.putInt8       = putInt8;
        api.putInt16      = putInt16;
        api.putInt32      = putInt32;
        api.putInt64      = putInt64;
        api.putWord8      = putWord8;
        api.putWord16     = putWord16;
        api.putWord32     = putWord32;
        api.putWord64     = putWord64;
        api.putPtr        = putPtr;
        api.putFunPtr     = putFunPtr;
        api.putForeignPtr = putForeignPtr;

	api.makeStablePtr4 = getStablePtr;
	api.derefStablePtr4= putStablePtr;

	api.putStablePtr4 = putStablePtr4;
	api.getStablePtr4 = getStablePtr4;
	api.freeStablePtr4 = freeStablePtr4;
			  
        api.runId         = runId;
    }
    return &api;
}

HugsAPI5* hugsAPI5() { /* build virtual function table */
    static HugsAPI5 api;
    static Bool initialised = FALSE;
    if (!initialised) {
	api.getBool       = getBool;
      	api.getInt        = getInt;
	api.getWord       = getWord;
	api.getAddr       = getAddr;
	api.getFloat      = getFloat;
	api.getDouble     = getDouble;
	api.getChar       = getChar;
	api.getForeign    = getForeign;
        api.getInt8       = getInt8;
        api.getInt16      = getInt16;
        api.getInt32      = getInt32;
        api.getInt64      = getInt64;
        api.getWord8      = getWord8;
        api.getWord16     = getWord16;
        api.getWord32     = getWord32;
        api.getWord64     = getWord64;
        api.getPtr        = getPtr;
        api.getFunPtr     = getFunPtr;
        api.getForeignPtr = getForeignPtr;
	api.getStablePtr4 = getStablePtr4;

	api.putBool       = putBool;
	api.putInt        = putInt;
	api.putWord       = putWord;
	api.putAddr       = putAddr;
	api.putFloat      = putFloat;
	api.putDouble     = putDouble;
	api.putChar       = putChar;
	api.putForeign    = putForeign;
        api.putInt8       = putInt8;
        api.putInt16      = putInt16;
        api.putInt32      = putInt32;
        api.putInt64      = putInt64;
        api.putWord8      = putWord8;
        api.putWord16     = putWord16;
        api.putWord32     = putWord32;
        api.putWord64     = putWord64;
        api.putPtr        = putPtr;
        api.putFunPtr     = putFunPtr;
        api.putForeignPtr = putForeignPtr;
	api.putStablePtr4 = putStablePtr4;
			  
	api.returnIO      = returnIO;
	api.runIO         = runIO;
        api.returnId      = returnId;
        api.runId         = runId;

	api.registerPrims = registerPrims;

        api.lookupName    = lookupName;
        api.ap            = apMany;
        api.getUnit       = getUnit;
        api.mkThunk       = mkThunk;
        api.freeThunk     = freeHaskellFunctionPtr;

	api.makeStablePtr4 = getStablePtr;
	api.derefStablePtr4= putStablePtr;
	api.freeStablePtr4 = freeStablePtr4;
    }
    return &api;
}

void hs_perform_gc(void)
{
    garbageCollect();
}

void hs_free_stable_ptr(HsStablePtr sp)
{
    freeStablePtr4(sp);
}

void hs_free_fun_ptr(HsFunPtr fp)
{
    freeHaskellFunctionPtr((void *)fp);
}

/* --------------------------------------------------------------------------
 * Built-in control:
 * ------------------------------------------------------------------------*/

/* Dummy entry */
static Void builtinControl (Int);
static Void builtinControl(what)
Int what; {
}
static struct primInfo builtinPrims = { builtinControl, builtinPrimTable, 0 };

Void builtIn(what)
Int what; {
    switch (what) {
	case INSTALL : 

		       registerPrims(&builtinPrims);
		       registerPrims(&printerPrims);
#if HASKELL_ARRAYS
		       registerPrims(&arrayPrims);
#endif
#if BIGNUMS
		       registerPrims(&bignumPrims);
#endif
#if IO_MONAD
		       registerPrims(&iomonadPrims);
#endif
#if TIME_MODULE
		       registerPrims(&timePrims);
#endif
#if DIRECTORY_MODULE
		       registerPrims(&dirPrims);
#endif
#if INTERNAL_PRIMS
		       registerPrims(&internalPrims);
#endif
		       setCurrModule(modulePrelude);
#define pFun(n,s,t)    addPrim(0,n=newName(findText(s),NIL),t,modulePrelude,NIL)
		       pFun(nameFatbar,    "_FATBAR",  "fatbar");
		       pFun(nameFail,      "_FAIL",    "fail");
		       pFun(nameIf,        "_IF",      "if");
		       pFun(nameSel,       "_SEL",     "sel");

		       pFun(nameConCmp,    "_concmp",  "conCmp");
		       pFun(nameEnRange,   "_range",   "enRange");
		       pFun(nameEnIndex,   "_index",   "enIndex");
		       pFun(nameEnInRng,   "_inRange", "enInRng");
		       pFun(nameEnToEn,    "_toEnum",  "enToEn");
		       pFun(nameEnFrEn,    "_frEnum",  "enFrEn");
		       pFun(nameEnFrom,    "_from",    "enFrom");
		       pFun(nameEnFrTo,    "_fromTo",  "enFrTo");
		       pFun(nameEnFrTh,    "_fromThen","enFrTh");

		       pFun(namePrimThrow, "_throw",    "primThrowException");
		       pFun(nameBlackHole, "_Gc Black Hole",    "gcBhole");
		       pFun(nameInd,	   "_indirect", "dictIndirect");
		       name(nameInd).number = DFUNNAME;
#if    TREX
		       pFun(nameRecExt,	   "_recExt",	"recExt");
		       pFun(nameRecBrk,	   "_recBrk",	"recBrk");
		       pFun(nameRecSel,	   "_recSel",	"recSel");
		       pFun(nameRecShw,	   "_recShw",	"recShw");
		       pFun(nameRecEq,	   "_recEq",	"recEq");
		       pFun(nameAddEv,	   "_addEv",	"primPlusInt");
		       name(nameAddEv).number = DFUNNAME;
#endif
#undef pFun
#define predef(nm,str) nm=newName(findText(str),NIL); name(nm).defn=PREDEFINED
		       predef(nameNegate,       "negate");
		       predef(nameFlip,         "flip");
		       predef(nameFrom,         "enumFrom");
		       predef(nameFromThen,     "enumFromThen");
		       predef(nameFromTo,       "enumFromTo");
		       predef(nameFromThenTo,   "enumFromThenTo");
		       predef(nameFst,		"fst");
		       predef(nameSnd,		"snd");
		       predef(nameAnd,          "&&");
		       predef(nameOr,           "||");
		       predef(nameId,           "id");
		       predef(nameOtherwise,    "otherwise");
		       predef(nameComp,         ".");
		       predef(nameApp,          "++");
		       predef(nameShowField,    "showField");
		       predef(nameShowParen,    "showParen");
		       predef(nameReadField,    "readField");
		       predef(nameReadParen,    "readParen");
		       predef(nameLex,          "lex");
		       predef(nameRangeSize,    "rangeSize");
		       predef(nameCompAux,      "primCompAux");
		       predef(namePmInt,        "primPmInt");
		       predef(namePmInteger,    "primPmInteger");
		       predef(namePmFlt,        "primPmFlt");
		       predef(nameReturnIO,     "primretIO");
#if NPLUSK
		       predef(namePmNpk,        "primPmNpk");
		       predef(namePmSub,        "primPmSub");
#endif
		       predef(nameRationalToDouble, "rationalToDouble");
		       predef(nameRationalToFloat,  "rationalToFloat");
#if SHORT_CIRCUIT_COERCIONS
		       predef(nameFloatToRational,  "floatToRational");
		       predef(nameDoubleToRational, "doubleToRational");
		       predef(nameDoubleToRatio,    "doubleToRatio");
		       predef(nameIntToRatio,       "intToRatio");
		       predef(nameIntToFloat,       "primIntToFloat");
		       predef(nameIntToDouble,      "primIntToDouble");
		       predef(nameDoubleToFloat,    "primDoubleToFloat");
		       predef(nameFloatToDouble,    "primFloatToDouble");
#endif
#undef  predef
		       break;

        case RESET   : freeAllThunks();
                       break;
    }
}

/*-------------------------------------------------------------------------*/

