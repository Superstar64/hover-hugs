/* --------------------------------------------------------------------------
 * Connections between components of the Hugs system
 *
 * The Hugs 98 system is Copyright (c) Mark P Jones, Alastair Reid, the
 * Yale Haskell Group, and the OGI School of Science & Engineering at OHSU,
 * 1994-2003, All rights reserved.  It is distributed as free software under
 * the license in the file "License", which is included in the distribution.
 *
 * $RCSfile: connect.h,v $
 * $Revision: 1.92 $
 * $Date: 2005/12/02 12:42:27 $
 * ------------------------------------------------------------------------*/
#ifndef __CONNECT_H__
#define __CONNECT_H__

#include "prelude.h"
#include "storage.h"

/* --------------------------------------------------------------------------
 * Standard data:
 * ------------------------------------------------------------------------*/

extern Bool   haskell98;		/* TRUE => Haskell 98 compatibility*/
extern Module modulePrelude;
extern Module moduleUserPrelude;

/* --------------------------------------------------------------------------
 * Primitive constructor functions 
 * ------------------------------------------------------------------------*/

extern Name  nameFalse, nameTrue;
extern Name  nameNil,   nameCons;
extern Name  nameJust,	nameNothing;
extern Name  nameLeft,	nameRight;
extern Name  nameUnit;

extern Name  nameLT,      nameEQ;
extern Name  nameGT;
extern Name  nameFst,     nameSnd;      /* standard combinators		   */
extern Name  nameId,	  nameOtherwise;
extern Name  nameNegate,  nameFlip;	/* primitives reqd for parsing	   */
extern Name  nameFrom,    nameFromThen;
extern Name  nameFromTo,  nameFromThenTo;
extern Name  nameFatbar,  nameFail;	/* primitives reqd for translation */
extern Name  nameIf,	  nameSel;
extern Name  nameCompAux;
extern Name  namePmInt,	  namePmFlt;	/* primitives for pattern matching */
extern Name  namePmInteger;
extern Name  namePmNpk,	  namePmSub;	/* primitives for (n+k) patterns   */
extern Name  nameBlackHole;		/* For GC-detected black hole	   */
extern Name  nameInd;			/* For dict indirection		   */
extern Name  nameAnd,	  nameOr;	/* For optimisation of && and ||   */
extern Name  nameFromInt, nameFromDouble;/*coercion of numerics		   */
extern Name  nameFromInteger;
extern Name  nameEq,	  nameCompare;	/* names used for deriving	   */
extern Name  nameMinBnd,  nameMaxBnd;
extern Name  nameIndex,	  nameInRange;
extern Name  nameRange;
extern Name  nameLe,      nameGt;
extern Name  nameShowsPrec, nameReadsPrec;
extern Name  nameMult,	  namePlus;
extern Name  nameConCmp,  nameEnRange;
extern Name  nameEnIndex, nameEnInRng;
extern Name  nameEnToEn,  nameEnFrEn;
extern Name  nameEnFrom,  nameEnFrTh;
extern Name  nameEnFrTo;
extern Name  nameComp,	  nameApp;	/* composition and append	   */
extern Name  nameShowField;		/* display single field		   */
extern Name  nameShowParen;		/* wrap with parens		   */
extern Name  nameReadField;		/* read single field		   */
extern Name  nameReadParen;             /* unwrap from parens              */
extern Name  nameLex;                   /* lexer                           */
extern Name  nameRangeSize;		/* calculate size of index range   */
extern Class classMonad;		/* Monads			   */
extern Name  nameReturn, nameBind;	/* for translating do / monad comps*/
extern Name  nameThen;                  /* for translating do / monad comps*/
extern Name  nameMFail;
extern Name  nameListMonad;		/* builder function for List Monad */

extern Class classMonadRec;		/* Recursive monads		   */
extern Name  nameMFix;			/* for translating mdo-notation	   */

extern Name  namePrint;			/* printing primitive		   */
extern Name  nameNPrint;		/* internal printer                */

extern Type   typeIO;		        /* For the IO monad, IO 	   */
extern Type   typeProgIO;		/* For the IO monad, IO ()	   */
extern Name   nameIORun;	        /* IO monad executor		   */
extern Name   nameIOBind;	        /* IO bind executor		   */
extern Name   namePutStr;	        /* Prelude.putStr                  */

extern Name   nameIOError, nameUserErr; /* primitives required for IOError */
extern Name   namePermDenied;
extern Name   nameAlreadyExists, nameAlreadyInUse, nameDoesNotExist, nameIsFull;
extern Name   nameIllegal;

#define IOArity		1		/* arity of IO actions             */

extern Name   nameEOFErr;
extern Name   nameProtocolError;


extern Name   namePrimThrow;

extern Name   nameArithException;
extern Name   nameArrayException;
extern Name   nameErrorCall;
extern Name   nameIOException;
extern Name   nameNoMethodError;
extern Name   nameNonTermination;
extern Name   namePatternMatchFail;
extern Name   nameRecConError;
extern Name   nameRecSelError;
extern Name   nameRecUpdError;

extern Name   nameOverflow;
extern Name   nameDivideByZero;
extern Name   nameIndexOutOfBounds;
extern Name   nameUndefinedElement;

extern Text  textCCall;                 /* ffi tokens                      */
extern Text  textSafe;
extern Text  textUnsafe;
extern Text  textThreadsafe;
extern Text  textExport;
#if STDCALL_SUPPORTED
extern Text  textStdcall;
#endif

extern Text  textPrelude;
extern Text  textUserPrelude;
extern Text  textNum;			/* used to process default decls   */
extern Text  textPlus;			/* Used to recognise n+k patterns  */
extern Name  nameNoRec;			/* The empty record		   */
extern Type  typeNoRow;			/* The empty row		   */
extern Type  typeRec;			/* Record formation		   */
extern Kind  extKind;			/* Kind of extension, *->row->row  */
extern Name  nameRecExt;		/* Extend a record		   */
extern Name  nameRecBrk;		/* Break a record		   */
extern Name  nameAddEv;			/* Addition of evidence values	   */
extern Name  nameRecSel;		/* Select a record		   */
extern Name  nameRecShw;		/* Show a record		   */
extern Name  nameShowRecRow;		/* Used to output rows		   */
extern Name  nameRecEq;			/* Compare records		   */
extern Name  nameEqRecRow;		/* Used to compare rows		   */
extern Name  nameInsFld;		/* Field insertion routine	   */
extern Name  nameShowRecRowCls;         /* Trex.ShowRecRow class           */
extern Name  nameEqRecRowCls;           /* Trex.EqRecRow class             */

extern String versionString;		/* String containing version name  */
extern String scriptFile;               /* Name of current script (if any) */

extern Type  typeArrow;			/* Builtin type constructors	   */
extern Type  typeList;
extern Type  typeUnit;

extern Type  typeInt;                          
extern Type  typeInt8;                          
extern Type  typeInt16;                          
extern Type  typeInt32;                          
extern Type  typeInt64;                          
extern Type  typeWord;
extern Type  typeWord8;
extern Type  typeWord16;
extern Type  typeWord32;
extern Type  typeWord64;
extern Type  typeFunPtr;
extern Type  typePtr;
extern Type  typeAddr;
extern Type  typeFloat;
extern Type  typeDouble;
extern Type  typeChar;
extern Type  typeForeignP;
extern Type  typeForeign;
extern Type  typeStable;
extern Type  typeBool;
extern Type  typeString;

#define fn(from,to)  ap(ap(typeArrow,from),to)	/* make type: from -> to   */

extern List  stdDefaults;		/* List of standard default types  */

extern Class classEq;			/* `standard' classes		   */
extern Class classOrd;
extern Class classShow;
extern Class classRead;
extern Class classIx;
extern Class classEnum;
extern Class classBounded;

extern Class classReal;			/* `numeric' classes		   */
extern Class classIntegral;
extern Class classRealFrac;
extern Class classRealFloat;
extern Class classFractional;
extern Class classFloating;
extern Class classNum;

extern Cell  *CStackBase;		/* pointer to base of C stack	   */

extern List  tyconDefns;		/* list of type constructor defns  */
extern List  typeInDefns;		/* list of synonym restrictions	   */
extern List  valDefns;			/* list of value definitions       */
extern List  classDefns;		/* list of class definitions       */
extern List  instDefns;			/* list of instance definitions    */
extern List  selDefns;			/* list of selector lists	   */
extern List  genDefns;			/* list of generated defns	   */
extern List  primDefns;			/* list of primitive definitions   */
extern List  unqualImports;		/* unqualified import list         */
extern List  defaultDefns;		/* default definitions (if any)	   */
extern Int   defaultLine;		/* line in which default defs occur*/
extern List  evalDefaults;		/* defaults for evaluator	   */
extern Cell  inputExpr;			/* evaluator input expression      */
extern Cell  inputContext;		/* evaluator input expression      */
extern Addr  inputCode;			/* Code for compiled input expr    */

extern Int   whnfArgs;		 	/* number of args of term in whnf  */
extern Cell  whnfHead;		 	/* head of term in whnf            */
extern Int   whnfInt;		 	/* integer value of term in whnf   */
extern Float whnfFloat;		 	/* float value of term in whnf	   */
extern Double whnfDouble;	 	/* double value of term in whnf	   */
extern Long  numReductions;		/* number of reductions used       */
extern Long  numCells;			/* number of cells allocated       */
extern Int   numGcs;			/* number of garbage collections   */
extern Bool  broken;			/* indicates interrupt received    */
extern Bool  preludeLoaded;		/* TRUE => prelude has been loaded */


/* --------------------------------------------------------------------------
 * Function prototypes etc...
 * ------------------------------------------------------------------------*/
#define RESET   1		/* reset subsystem                         */
#define MARK    2		/* mark parts of graph in use by subsystem */
#define INSTALL 3		/* install subsystem (executed once only)  */
#define EXIT	4		/* Take action immediately before exit()   */
#define BREAK   5		/* Take action after program break	   */

/* hugs.c exports: */
extern  Void   shutdownHugs     (Void);
extern  Void   promptForInput   (String);
/* The next three are required only by winhugs */
extern  Bool   doCommand        (Void);
extern  Void   runEditor        (Void);
extern  Module findEvalModule   (Void);

extern  Void   storage          (Int);

extern  Bool   startsQual        (Char);
extern  Void   input             (Int);
extern  Void   consoleInput      (String);
extern  Void   stringInput       (String);
extern  Bool   parseScript       (String,Long);
extern  Void   parseScriptString (String);
extern  Void   parseExp          (Void);
#if EXPLAIN_INSTANCE_RESOLUTION
extern  Void   parseContext     (Void);
#endif
extern  String readFilename     (Void);
extern  String readLine		(Void);
extern  Bool   isModuleId	(String);
extern  Syntax defaultSyntax    (Text);
extern  Syntax syntaxOf		(Name);
extern  String unlexChar        (Char,Char);
extern  Void   printString	(String);
extern  Char   getStrChr	(String *);

extern  Void   substitution	(Int);

extern  Void   staticAnalysis   (Int);
extern  Void   startModule      (Cell);
extern  Void   setExportList    (List);
extern  Void   setExports       (List);
extern  Void   tyconDefn	(Int,Cell,Cell,Cell);
extern  Void   setTypeIns	(List);
extern  Void   clearTypeIns	(Void);
extern  Type   fullExpand	(Type);
extern  Bool   isAmbiguous	(Type);
extern  Void   ambigError	(Int,String,Cell,Type);
extern  Void   classDefn	(Int,Cell,List,List);
extern  Void   instDefn		(Int,Cell,Cell);
extern  Void   addTupInst	(Class,Int);
extern  Bool   hasIOResultType  (Type);
extern  Inst   addRecShowInst	(Class,Ext);
extern  Inst   addRecEqInst	(Class,Ext);
extern Text    zipName		(Int);
extern  List   oclose		(List,List);
extern  List   zonkTyvarsIn	(Type,List);
extern  Type   zonkTyvar	(Int);
extern  Type   zonkType		(Type,Int);
extern  Void   primDefn		(Cell,List,Cell);
extern  Void   defaultDefn	(Int,List);
extern  Void   checkExp		(Void);
#if EXPLAIN_INSTANCE_RESOLUTION
extern  Void   checkContext	(Void);
#endif
extern  Void   checkDefns	(Void);
extern  Void   h98CheckInferredType	(Int,Cell,Type);
extern  Void   h98DoesntSupport	(Int,String);
extern  Cell   depExpr          (Int,Cell);

extern  Void   foreignImport    (Cell,Cell,Cell,Cell,Cell,Type);
extern  Void   foreignExport    (Cell,Cell,Cell,Cell,Cell,Type);
extern  Int    foreignCount;
extern  List   foreignImports;           /* foreign import declarations     */
extern  List   foreignExports;           /* foreign export declarations     */

extern  Void   ffi              (Int);
extern  Void   foreignHeader    (String);
extern  Void   foreignFooter    (String,Text,List,List);
extern  Void   ffiSetFlags      (String);
extern  Void   ffiAddCppInclude (String);
extern  Void   implementForeignImport (Int,Name,Int,Text,Text,Bool,Text,List,Bool,Type);
extern  Void   implementForeignImportDynamic (Int,Int,Text,List,Bool,Type);
extern  Void   implementForeignImportWrapper (Int,Int,Text,List,Bool,Type);
extern  Void   implementForeignImportLabel   (Int,Int,Text,Text,Text,Type);
extern  Void   implementForeignExport        (Int,Int,Text,List,Bool,Type);
extern  Bool   foreignNeedStubs              (List,List);
extern  Bool   generateFFI;	/* running ffihugs? */
extern  Bool   generate_ffi;	/* generate FFI for the current module? */

extern  Void   typeChecker      (Int);
extern  Type   typeCheckExp	(Bool);
extern  Void   typeCheckDefns	(Void);
extern  Cell   provePred	(Kinds,List,Cell);
extern  Cell   resolvePred	(Kinds,Cell);
extern  List   simpleContext	(List,Int);
extern  Cell   rhsExpr		(Cell);
extern  Int    rhsLine		(Cell);
extern  Cell   getProgType	(List,Type);
extern  Cell   superEvid	(Cell,Class,Class);
extern  Void   linkPreludeTC    (Void);
extern  Void   linkPreludeCM    (Void);
extern  Void   linkPreludeFuns  (Void);

extern  Void   compiler         (Int);
extern  Void   compileDefns     (Void);
extern  Void   compileExp       (Void);
extern  Bool   failFree		(Cell);
extern  Int    discrArity       (Cell);

extern  Void   machine          (Int);
extern  Addr   codeGen          (Name,Int,Cell);
extern  Void   implementCfun	(Name,List);
extern  Name   implementRecShw  (Text,Cell);
extern  Name   implementRecEq   (Text,Cell);
extern  Void   addCfunTable	(Tycon);
extern  Name   succCfun		(Name);
extern  Name   nextCfun		(Name,Name);
extern  Name   cfunByNum	(Name,Int);
extern  Void   unwind           (Cell);
extern  Void   run              (Addr,StackPtr);
extern DynTable* dynMemory;
extern void    growMemory       (Void);

extern  Void   eval             (Cell);
extern  Cell   evalWithNoError  (Cell);
extern  Void   evalFails        (StackPtr);
extern  Void   throwException   (Cell) HUGS_noreturn;

#if BYTECODE_PRIMS
extern Int     IntAt            (Addr);
extern Float   FloatAt          (Addr);
extern Double  DoubleAt         (Addr);
extern Cell    CellAt           (Addr);
extern Text    TextAt           (Addr);
extern Addr    AddrAt           (Addr);
extern Int     InstrAt          (Addr);
#endif /* BYTECODE_PRIMS */

extern  Void   builtIn          (Int);
extern  Void   abandon		(String,Cell);
extern  Void   outputString	(FILE *);
extern  Void   dialogue		(Cell);
extern  Cell   consChar		(Char);
extern  Bignum bigInt		(Int);
extern  Bignum bigDouble	(double);
extern  Bignum bigNeg		(Bignum);
extern  Cell   bigToInt		(Bignum);
extern  double bigToDouble	(Bignum);
extern  Bignum bigStr		(String);
extern  Cell   bigOut		(Bignum,Cell,Bool);
extern  Bignum bigShift		(Bignum,Int,Int);
extern  Int    bigCmp		(Bignum,Bignum);
extern Void   setHugsArgs       (Int,String[]);

extern  Void   machdep          (Int);
extern  String findPathname     (String);
extern  String findMPathname    (String);
extern  String findMInDir       (String,String);
extern  Bool   readable         (String,Bool);
#if PROFILING
extern  String timeString	(Void);
#endif
extern  String hugsdir          (Void);
extern  String mkFFIFilename    (String);
extern  String mkFFIFilename2   (String);
extern  Void   freeDLL          (void*);
extern  Void   compileAndLink   (String,String);

extern  Void   plugins          (Int);
extern  Bool   havePlugin       (String);
extern  List   calcFunDepsPreds (List);
extern  Inst   findInstFor      (Cell,Int);


#define NUMARGS         16               /* max num of args; must be 2^n    */
#define appId(seq,arg)  (seq)*NUMARGS+(arg)
#define argNum(n)       (n)%NUMARGS
#define seqNum(n)       (n)/NUMARGS
extern Bool   printingObservations;     /* TRUE => print observed exprs    */
extern Int    appNum;                   /* for counting applications       */
extern Int    obsCount;			/* sanity counter for observations */
extern Bool   isWhnf           (Cell);
extern Cell   getCaf           (Cell);
extern Int    countObserve     (Void);

extern  Void   charOps          (Int);

extern	Void   pushString	(String);

/*-------------------------------------------------------------------------*/

#endif /* __CONNECT_H__ */
