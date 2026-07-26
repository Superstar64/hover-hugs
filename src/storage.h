/* --------------------------------------------------------------------------
 * Defines storage datatypes: Text, Name, Module, Tycon, Cell, List, Pair,
 * Triple, ...
 *
 * The Hugs 98 system is Copyright (c) Mark P Jones, Alastair Reid, the
 * Yale Haskell Group, and the OGI School of Science & Engineering at OHSU,
 * 1994-2003, All rights reserved.  It is distributed as free software under
 * the license in the file "License", which is included in the distribution.
 *
 * $RCSfile: storage.h,v $
 * $Revision: 1.68 $
 * $Date: 2004/11/14 11:02:38 $
 * ------------------------------------------------------------------------*/
#ifndef __STORAGE_H__
#define __STORAGE_H__

#include "prelude.h"

/* --------------------------------------------------------------------------
 * Typedefs for main data types:
 * Many of these type names are used to indicate the intended us of a data
 * item, rather than for type checking purposes.  Sadly (although sometimes,
 * fortunately), the C compiler cannot distinguish between the use of two
 * different names defined to be synonyms for the same types.
 * ------------------------------------------------------------------------*/

typedef Int	     Text;			 /* text string 	   */
typedef Unsigned     Syntax;			 /* syntax (assoc,preced)  */
typedef Int	     Addr;			 /* address of code	   */
typedef Int	     Cell;			 /* general cell value	   */
typedef Cell far     *Heap;			 /* storage of heap	   */
typedef Cell	     Pair;			 /* pair cell		   */
typedef Int	     StackPtr;			 /* stack pointer	   */
typedef Cell	     Offset;			 /* offset/generic variable*/
typedef Int	     Script;			 /* script file number	   */
typedef Int	     Module;			 /* module 	           */
typedef Cell	     Tycon;			 /* type constructor	   */
typedef Cell	     Type;			 /* type expression	   */
typedef Cell	     Kind;			 /* kind expression	   */
typedef Cell	     Kinds;			 /* list of kinds	   */
typedef Cell	     Constr;			 /* constructor expression */
typedef Cell	     Name;			 /* named value 	   */
typedef Void	     (*Prim) (StackPtr);	 /* primitive function	   */
typedef Cell	     Class;			 /* type class		   */
typedef Cell	     Inst;			 /* instance of type class */
typedef Cell	     Triple;			 /* triple of cell values  */
typedef Cell	     List;			 /* list of cells	   */
typedef Cell	     Bignum;			 /* bignum integer	   */
typedef FloatImpType Float;			 /* implementation of Float*/
typedef DoubleImpType Double;			 /* implementation of Double*/
typedef Cell	     Ext;			 /* extension label	   */
typedef Int          Observe;                    /* observation            */
typedef Int          Breakpt;                    /* Breakpoint             */

#ifdef DOTNET
extern  Void         primInvoker (StackPtr,Name);
#endif

/* --------------------------------------------------------------------------
 * Text storage:
 * provides storage for the characters making up identifier and symbol
 * names, string literals, character constants etc...
 * ------------------------------------------------------------------------*/

extern	String	     textToStr		(Text);
extern	Text	     findText		(String);
extern	Text         inventText		(Void);
extern  Text	     inventDictText	(Void);
extern  Bool	     inventedText	(Text);
extern	String	     identToStr		(Cell);
extern	Text	     fixLitText	 	(Text);
extern	Text	     concatText		(String,String);
extern  Text         subText            (String,Int);

/* --------------------------------------------------------------------------
 * Specification of syntax (i.e. default written form of application)
 * ------------------------------------------------------------------------*/

#define MIN_PREC  0		       /* weakest binding operator	   */
#define MAX_PREC  9		       /* strongest binding operator	   */
#define FUN_PREC  (MAX_PREC+2)	       /* binding of function symbols	   */
#define DEF_PREC  MAX_PREC
#define APPLIC	  0	 	       /* written applicatively 	   */
#define LEFT_ASS  1	 	       /* left associative infix	   */
#define RIGHT_ASS 2	 	       /* right associative infix	   */
#define NON_ASS   3	 	       /* non associative infix 	   */
#define DEF_ASS   LEFT_ASS

#define UMINUS_PREC  6			/* Change these settings at your   */
#define UMINUS_ASSOC LEFT_ASS		/* own risk; they may not work!	   */

#define assocOf(x)	((x)&NON_ASS)
#define precOf(x)	((x)>>2)
#define mkSyntax(a,p)	((a)|((p)<<2))
#define DEF_OPSYNTAX	mkSyntax(DEF_ASS,DEF_PREC)
#define NO_SYNTAX	((Syntax)(-1))

#define FFI_NOSAFETY   0
#define FFI_SAFE       1
#define FFI_UNSAFE     2
#define FFI_THREADSAFE 4

/* Note: cconv flags are combined with the above safety flags,
 * so need to be disjoint.
 */
#define FFI_CCONV_UNKNOWN  0
#define FFI_CCONV_CCALL    8
#define FFI_CCONV_STDCALL 16
#define FFI_CCONV_DOTNET  32

#define FFI_TYPE_UNIT      0
#define FFI_TYPE_CHAR      1
#define FFI_TYPE_INT       2
#define FFI_TYPE_INT8      3
#define FFI_TYPE_INT16     4
#define FFI_TYPE_INT32     5
#define FFI_TYPE_INT64     6
#define FFI_TYPE_WORD8     7
#define FFI_TYPE_WORD16    8
#define FFI_TYPE_WORD32    9
#define FFI_TYPE_WORD64   10
#define FFI_TYPE_FLOAT    11
#define FFI_TYPE_DOUBLE   12
#define FFI_TYPE_BOOL     13
#define FFI_TYPE_ADDR     14
#define FFI_TYPE_PTR      15
#define FFI_TYPE_FUNPTR   16
#define FFI_TYPE_FOREIGN  17
#define FFI_TYPE_STABLE   18
#ifdef DOTNET
#define FFI_TYPE_OBJECT   19
#define FFI_TYPE_STRING   20
#endif

#ifdef DOTNET
#define FFI_DOTNET_STATIC 1
#define FFI_DOTNET_FIELD  2
#define FFI_DOTNET_CTOR   4
#define FFI_DOTNET_METHOD 8
#endif


/* --------------------------------------------------------------------------
 * Primitive functions:
 * ------------------------------------------------------------------------*/

struct primitive {		        /* entry in table of primitives	   */
    String ref;				/* primitive reference string	   */
    Int	   arity;			/* primitive function arity	   */
    Prim   imp;				/* primitive implementation	   */
};

/* Modules that define new primitive functions must register a control
 * function (defining INSTALL, RESET, etc code) and a (null-terminated)
 * table of primitive functions.
 *
 * They are stored as a linked list - so there's no wired in limits.
 * Control functions are called in the order they are registered
 * after all other control functions have been called.
 * (At the moment) there's no way of unregistering a module.
 */

struct primInfo {
    Void              (*controlFun) (Int);
    struct primitive  *primFuns;
    struct primInfo   *nextPrimInfo;
};

extern  Void registerPrims (struct primInfo*);
extern  Void controlFuns   (Int); /* Call all control functions in   */
					/* prim list.                      */
extern  struct primInfoDef* setPrimInfoDll (void*);

/* --------------------------------------------------------------------------
 * Program code storage: for holding compiled function defns etc...
 * ------------------------------------------------------------------------*/

extern	Addr	     getMem	(Int);
extern  Void	     nextInstr	(Addr);

/* --------------------------------------------------------------------------
 * Heap storage:
 * Provides a garbage collectable heap for storage of expressions etc.
 * ------------------------------------------------------------------------*/

#define heapAlloc(s) (Heap)(farCalloc(s,sizeof(Cell)))
#define heapBuilt()  (heapFst)
extern  Int          heapSize;
extern  Heap	     heapFst, heapSnd;
#ifdef  GLOBALfst
register Heap	     heapTopFst GLOBALfst;
#else
extern   Heap        heapTopFst;
#endif
#ifdef  GLOBALsnd
register Heap	     heapTopSnd GLOBALsnd;
#else
extern   Heap        heapTopSnd;
#endif
extern   Bool        consGC;		/* Set to FALSE to turn off gc from*/
					/* C stack; use with extreme care! */
extern   Int	     cellsRecovered;	/* cells recovered by last gc	   */

#define fst(c)	     heapTopFst[c]
#define snd(c)	     heapTopSnd[c]
#if PROFILING
extern   Heap	     heapThd, heapTopThd;
#define thd(c)	     heapTopThd[c]
extern   Name	     producer;
extern   Bool        profiling;
extern   Int 	     profInterval;
extern   Void	     profilerLog     (String);
#endif

extern	Pair	     pair	     (Cell,Cell);
extern  Void         garbageCollect  (Void);

extern  Void	     overwrite	     (Pair,Pair);
extern  Cell	     markExpr	     (Cell);
extern  Void	     markWithoutMove (Cell);

#define mark(v)      v=markExpr(v)

#define isPair(c)    ((c)<0)
#define isGenPair(c) ((c)<0 && -heapSize<=(c))

#define whatIs(c)    (isPair(c)? (isTag(fst(c)) ? fst(c) : AP ) : whatCode[c])
extern unsigned char whatCode[];

/* --------------------------------------------------------------------------
 * Box cell tags are used as the fst element of a pair to indicate that
 * the snd element of the pair is to be treated in some special way, other
 * than as a Cell.  Examples include holding integer values, variable name
 * and string text etc.
 * ------------------------------------------------------------------------*/

#define TAGMIN	     1		  /* Box and constructor cell tag values   */
#define BCSTAG	     30 	  /* Box=TAGMIN..BCSTAG-1		   */
#define isTag(c)     (TAGMIN<=(c) && (c)<SPECMIN) /* Tag cell values	   */
#define isBoxTag(c)  (TAGMIN<=(c) && (c)<BCSTAG)  /* Box cell tag values   */
#define isConTag(c)  (BCSTAG<=(c) && (c)<SPECMIN) /* Constr cell tag values*/
#define TAG_MASK     (~0x7f)	  /* This masks 0..SPECMIN-1		   */

#define INDIRECT     1		  /* Indirection node:	      snd :: Cell  */
#define INDIRECT1    2		  /* Temporary indirection:   snd :: Cell  */
#define FREECELL     3		  /* Free list cell:	      snd :: Cell  */
#define VARIDCELL    4		  /* Identifier variable:     snd :: Text  */
#define VAROPCELL    5		  /* Operator variable:       snd :: Text  */
#define DICTVAR	     6		  /* Dictionary variable:     snd :: Text  */
#define CONIDCELL    7		  /* Identifier constructor:  snd :: Text  */
#define CONOPCELL    8		  /* Operator constructor:    snd :: Text  */
#define STRCELL      9		  /* String literal:	      snd :: Text  */
#define INTCELL	     10		  /* Int literal:	      snd :: Int   */
#define ADDPAT	     11		  /* (_+k) pattern discr:     snd :: Int   */
#define HANDCELL     12		  /* IO monad Handle:	      snd :: Int   */
#define WEAKCELL     13		  /* WeakPtr Cell:	      snd::(Cell,List)*/
#define WEAKFIN	     14		  /* Weak pointer with fin:   snd::(k,(v,f))*/
#if !BREAK_FLOATS
#define FLOATCELL    15		  /* Floating pt number:      snd :: Float */
#endif
#define EXTCOPY	     16		  /* Copy of an Ext:	      snd :: Text  */
#define MPCELL       17		  /* Malloc Ptr Cell:         snd :: Int   */
#if SIZEOF_INTP == SIZEOF_INT
#define PTRCELL      18           /* C Heap Pointer:          snd ::Pointer*/
#endif
#define IPCELL       19		  /* Imp Param Cell:	      snd :: Text  */
#define IPVAR	     20		  /* ?x:		      snd :: Text  */
#if DOTNET
#define NPCELL       21		  /* .NET Ptr Cell:           snd :: Int   */
#endif
/* CHARCELL is the whatIs code for chars; if UNICODE_CHARS is set, it
 * is also used for box cells representing character values greater than
 * or equal to NUM_SMALL_CHARS, in the same way as INTCELL.
 */
#define CHARCELL     22		  /* Char literal:	      snd :: Char  */

#define textOf(c)	((Text)(snd(c)))         /* c ::  (VAR|CON)(ID|OP) */
#define qmodOf(c)       (textOf(fst(snd(c))))    /* c ::  QUALIDENT        */
#define qtextOf(c)      (textOf(snd(snd(c))))    /* c ::  QUALIDENT        */
#define mkVar(t)	ap(VARIDCELL,t)
#define mkVarop(t)	ap(VAROPCELL,t)
#define mkCon(t)	ap(CONIDCELL,t)
#define mkConop(t)	ap(CONOPCELL,t)
#define mkQId(m,c)      ap(QUALIDENT,pair(mkCon(m),c))
#define mkQVar(m,t)	ap(QUALIDENT,pair(mkCon(m),mkVar(t)))
#define mkQCon(m,t)	ap(QUALIDENT,pair(mkCon(m),mkCon(t)))
#define mkQVarOp(m,t)	ap(QUALIDENT,pair(mkCon(m),mkVarop(t)))
#define mkQConOp(m,t)	ap(QUALIDENT,pair(mkCon(m),mkConop(t)))
#define mkNestedQual(c) findText(identToStr(c))	/* fragile...              */
#define intValOf(c)	(snd(c))
#define inventVar()	mkVar(inventText())
#define mkDictVar(t)	ap(DICTVAR,t)
#define inventDictVar() mkDictVar(inventDictText())
#define mkStr(t)	ap(STRCELL,t)
#define mkIParam(c)	ap(IPCELL,snd(c))
#define isIP(p)		(whatIs(p) == IPCELL)
#define ipMatch(pi, t)	(isIP(fun(pi)) && textOf(fun(pi)) == t)
#define ipVar(pi)	textOf(fun(pi))

extern	Bool		isVar       (Cell);
extern	Bool		isCon       (Cell);
extern	Bool		isQVar      (Cell);
extern	Bool		isQCon      (Cell);
extern	Bool		isQualIdent (Cell);
extern	Bool		isIdent     (Cell);

#define isFloat(c)      (isPair(c) && fst(c)==FLOATCELL)
extern	Cell	        mkFloat		(FloatPro);
extern  FloatPro	floatOf		(Cell);
extern  String		floatToString   (FloatPro);

#define isDouble(c)     (isPair(c) && fst(c)==DOUBLECELL)
extern	Cell	        mkDouble	(DoublePro);
extern  DoublePro	doubleOf	(Cell);
extern  String		doubleToString  (DoublePro);
extern  DoublePro	stringToDouble  (String);
extern  Cell		part1Double	(DoublePro);
extern  Cell		part2Double	(DoublePro);
extern  DoublePro	doubleFromParts	(Cell,Cell);

#define isPtr(c)        (isPair(c) && fst(c)==PTRCELL)
extern  Cell            mkPtr           (Pointer);
extern  Pointer         ptrOf           (Cell);

/* --------------------------------------------------------------------------
 * Resizeable table struct type.
 * ------------------------------------------------------------------------*/
typedef struct strDynTable {
  /* "data" holds "maxIdx*eltSize" bytes.
   * "maxIdx" may not grow beyond "hWater", unless "hWater" is 0,
   * in which case "maxIdx" does not have an upper bound.
   */
  unsigned long maxIdx;
  unsigned long hWater;
  unsigned long eltSize;
  const char*   tabName;
  void*         data;
} DynTable;

#define IdxDynTable(eltTy,tab,idx)  (((eltTy *)(tab->data))[idx])
extern DynTable* allocDynTable (unsigned long,unsigned long,unsigned long,const char*);
extern void freeDynTable       (DynTable*);
extern void growDynTable       (DynTable*);

/* --------------------------------------------------------------------------
 * Constructor cell tags are used as the fst element of a pair to indicate
 * a particular syntactic construct described by the snd element of the
 * pair.
 * Note that a cell c will not be treated as an application (AP/isAp) node
 * if its first element is a constructor cell tag, whereas a cell whose fst
 * element is a special cell will be treated as an application node.
 * ------------------------------------------------------------------------*/

#define LETREC	     30 	  /* LETREC	snd :: ([Decl],Exp)	   */
#define COND	     31 	  /* COND	snd :: (Exp,Exp,Exp)	   */
#define LAMBDA	     32 	  /* LAMBDA	snd :: Alt		   */
#define FINLIST      33 	  /* FINLIST	snd :: [Exp]		   */
#define DOCOMP       34 	  /* DOCOMP	snd :: (Exp,[Qual])	   */

#define MDOCOMP	     44		  /* MDOCOMP	snd :: (Exp,[Qual])	   */

#define BANG	     35		  /* BANG	snd :: Type		   */
#define COMP         36 	  /* COMP	snd :: (Exp,[Qual])	   */
#define ASPAT	     37 	  /* ASPAT	snd :: (Var,Exp)	   */
#define ESIGN	     38 	  /* ESIGN	snd :: (Exp,Type)	   */
#define RSIGN	     39 	  /* RSIGN	snd :: (Rhs,Type)	   */
#define CASE	     40 	  /* CASE	snd :: (Exp,[Alt])	   */
#define NUMCASE	     41		  /* NUMCASE	snd :: (Exp,Disc,Rhs)	   */
#define FATBAR	     42 	  /* FATBAR	snd :: (Exp,Exp)	   */
#define LAZYPAT      43 	  /* LAZYPAT	snd :: Exp		   */
#define DERIVE	     45		  /* DERIVE	snd :: Cell		   */
#if BREAK_FLOATS
#define FLOATCELL    46		  /* FLOATCELL  snd :: (Int,Int)	   */
#endif

#define POSNUM	     47		  /* POSNUM	snd :: [Int]		   */
#define NEGNUM	     48		  /* NEGNUM	snd :: [Int]		   */

#define BOOLQUAL     49 	  /* BOOLQUAL	snd :: Exp		   */
#define QWHERE	     50 	  /* QWHERE	snd :: [Decl]		   */
#define FROMQUAL     51 	  /* FROMQUAL	snd :: (Exp,Exp)	   */
#define DOQUAL	     52		  /* DOQUAL	snd :: Exp		   */
#define MONADCOMP    53 	  /* MONADCOMP	snd :: ((m,m0),(Exp,[Qual])*/

#define GUARDED      54 	  /* GUARDED	snd :: [guarded exprs]	   */

#define ARRAY        55		  /* Array	snd :: (Bounds,[Values])   */
#define MUTVAR	     56		  /* Mutvar	snd :: Cell		   */
#define HUGSOBJECT   57		  /* HUGSOBJECT	snd :: Cell	           */

#define STABLENAME   58		  /* Stable Nm  snd :: Cell		   */

#define WITHEXP      59 	  /* WITH	snd :: [(Var,Exp)]	   */

#define POLYTYPE     60		  /* POLYTYPE	snd :: (Kind,Type)	   */
#define QUAL	     61		  /* QUAL	snd :: ([Classes],Type)	   */
#define RANK2	     62		  /* RANK2	snd :: (Int,Type)	   */
#define EXIST	     63		  /* EXIST	snd :: (Int,Type)	   */
#define POLYREC	     64		  /* POLYREC	snd :: (Int,Type)	   */
#define BIGLAM	     65		  /* BIGLAM	snd :: (vars,patterns)	   */
#define CDICTS	     66		  /* CDICTS	snd :: ([Pred],Type)	   */

#define LABC	     70		  /* LABC	snd :: (con,[(Vars,Type)]) */
#define CONFLDS	     71		  /* CONFLDS	snd :: (con,[Field])	   */
#define UPDFLDS	     72		  /* UPDFLDS	snd :: (Exp,[con],[Field]) */
#define RECORD	     73		  /* RECORD	snd :: [Val]		   */
#define EXTCASE	     74		  /* EXTCASE	snd :: (Exp,Disc,Rhs)	   */
#define RECSEL	     75		  /* RECSEL	snd :: Ext		   */
#define IMPDEPS	     78		  /* IMPDEPS	snd :: [Binding]	   */

#define QUALIDENT    80           /* Qualified identifier  snd :: (Id,Id)  */
#define HIDDEN       81           /* hiding import list    snd :: [Entity] */
#define MODULEENT    82           /* module in export list snd :: con      */

#define OBSERVEHEAD  83           /* obs. list; snd ::(first,last)         */
#define OBSERVE      84           /* observe marker; snd :: (Cell,observe) */
#define OBSERVESTK   85           /* observe marker on stack               */

#define INFIX	     90		  /* INFIX	snd :: (see tidyInfix)	   */
#define ONLY	     91		  /* ONLY	snd :: Exp		   */
#define NEG	     92		  /* NEG	snd :: Exp		   */

#define ZCOMP        93 	  /* ZCOMP	snd :: (Exp,[[Qual]])	   */

#define DOUBLECELL   97		  /* DOUBLECELL snd :: (Int,Int)	   */
#define I64CELL      98           /* Int/Word64 snd :: (Int,Int)           */

#if SIZEOF_INTP != SIZEOF_INT
#define PTRCELL      99           /* C Heap Pointer snd :: (Int,Int)       */
#endif

/* Last constructor tag must be less than SPECMIN */

/* --------------------------------------------------------------------------
 * Special cell values:
 * ------------------------------------------------------------------------*/


#define SPECMIN      129	  /* must be out of TAG_MASK range         */

#define NONE	     129	  /* Dummy stub				   */
#define STAR	     130	  /* Representing the kind of types	   */
#define ROW	     131	  /* Representing the kind of rows	   */
#define WILDCARD     132	  /* Wildcard pattern			   */

#define SKOLEM	     133	  /* Skolem constant			   */

#define DOTDOT       134          /* ".." in import/export list            */

#define ZERONUM      136	  /* The zero bignum (see POSNUM, NEGNUM)  */

#define NAME	     138	  /* whatIs code for isName		   */
#define TYCON	     139	  /* whatIs code for isTycon		   */
#define CLASS	     140	  /* whatIs code for isClass		   */
#define MODULE       141          /* whatIs code for isModule              */
#define INSTANCE     142          /* whatIs code for isInst                */
#define TUPLE	     143	  /* whatIs code for tuple constructor	   */
#define OFFSET	     144	  /* whatis code for offset		   */
#define AP	     145	  /* whatIs code for application node	   */
#define EXT	     147	  /* whatIs code for isExt		   */

#define SIGDECL      148	  /* Signature declaration		   */
#define FIXDECL      149	  /* Fixity declaration			   */
#define FUNBIND	     150	  /* Function binding			   */
#define PATBIND	     151	  /* Pattern binding			   */

#define DATATYPE     158	  /* Datatype type constructor		   */
#define NEWTYPE	     159	  /* Newtype type constructor		   */
#define SYNONYM	     160	  /* Synonym type constructor		   */
#define RESTRICTSYN  161	  /* Synonym with restricted scope	   */

#define NODEPENDS    163	  /* Stop calculation of deps in type check*/
#define PREDEFINED   164	  /* Predefined name, not yet filled       */


/* --------------------------------------------------------------------------
 * Tuple data/type constructors:
 * ------------------------------------------------------------------------*/

#define TUPMIN	     201
#define isTuple(c)   (TUPMIN<=(c) && (c)<EXTMIN)
#define mkTuple(n)   (TUPMIN+(n))
#define tupleOf(n)   ((Int)((n)-TUPMIN))

#define EXTMIN	     (TUPMIN+NUM_TUPLES)
#define isExt(c)     (EXTMIN<=(c) && (c)<OFFMIN)
#define extText(e)   tabExt[(e)-EXTMIN]
#define extField(c)  arg(fun(c))
#define extRow(c)    arg(c)

extern Text	     DECTABLE(tabExt);
extern Ext           mkExt (Text);

/* --------------------------------------------------------------------------
 * Offsets: (generic types/stack offsets)
 * ------------------------------------------------------------------------*/

#define OFFMIN	     (EXTMIN+NUM_EXT)
#define isOffset(c)  (OFFMIN<=(c) && (c)<MODMIN)
#define offsetOf(c)  ((c)-OFFMIN)
#define mkOffset(o)  (OFFMIN+(o))

/* --------------------------------------------------------------------------
 * Observations
 * ------------------------------------------------------------------------*/

#define OBSMIN	      (OFFMIN+NUM_OFFSETS)
#define observe(n)    tabObserve[(n)-OBSMIN]

struct strObserve {
	Text tag;		/* tag in observe primitive		*/
	Cell head;
};

#define firstObs(c) snd3(c)
#define lastObs(c)  thd3(c)

#define nextObs(c)  fst3(c)
#define seqObs(c)   snd3(c)
#define exprObs(c)  thd3(c)

#define markedExpr(c) snd3(c)
#define markedObs(c)  thd3(c)

extern struct strObserve          DECTABLE(tabObserve);
extern Observe   firstObserve	  (Void);
extern Observe   nextObserve	  (Void);
extern Void      clearObserve	  (Void);
extern Cell      addObsInstance	  (String,Cell,Int);
extern Void      insertAfterObs   (Cell,Cell);

/* --------------------------------------------------------------------------
 * Breakpoints
 * ------------------------------------------------------------------------*/

#define BRKMIN	      (OBSMIN+NUM_OBS_TAGS)
#define breakpt(n)    tabBreakpt[(n)-BRKMIN]

struct strBreakpt {
	Text tag;
	Bool enabled;
	Int  count;			/* number of breaks to skip 	   */
};

extern Void     clearAllBreak   (Void);
extern Bool     breakNow	(String);
extern Void     setBreakpt	(String,Bool);
extern Void     setBreakCount   (String,Int);

/* --------------------------------------------------------------------------
 * Modules:
 * ------------------------------------------------------------------------*/

#define MODMIN        (BRKMIN+NUM_BRKPTS)

#define isModule(c)   (MODMIN<=(c) && (c)<TYCMIN)
#define mkModule(n)   (MODMIN+(n))
#define module(n)     tabModule[(n)-MODMIN]

/* Under Haskell 1.3, the list of qualified imports is always a subset
 * of the list of unqualified imports.  For simplicity and flexibility,
 * we do not attempt to exploit this fact - when a module is imported
 * unqualified, it is added to both the qualified and unqualified
 * import lists.
 * Similarily, Haskell 1.3 does not allow a constructor to be imported
 * or exported without exporting the type it belongs to but the export
 * list is just a flat list of Texts (before static analysis) or
 * Tycons, Names and Classes (after static analysis).
 */
struct strModule {
    Text  text;
    /* Lists of top level objects (local defns + imports)                       */
    /* Invariant: a name/tycons only appears once (i.e., no dups) in this list. */
    List  tycons;
    List  names;
    List  classes;
    List  exports; /* [ Entity | (Entity, NIL|DOTDOT|[Entity]) ] */
    /* (alias,modName) list of local aliases. Used both during compilation and when
     * evaluating an expression in the context of the current module.
     */
    List  modAliases;
    /* List of qualified imports.  Used both during compilation and when
     * evaluating an expression in the context of the current module.
     */
   List  qualImports; /* [ (modName{-Text-},[Entity]) ] */
                      /* NOTE: static.c:checkQualImport() modifies 'modName' to a Module */

    /* For each module imported, record the names that was
     * effectively imported, taking into consideration
     * import lists and 'hiding's. Used to handle re-exportation
     * of modules only -- that is, if a subset S of module A is
     * imported in module B, then the presence of 'module A' in
     * the export list of B, means that S should be re-exported
     * from B, not all of A.
     *
     * The bool in the second pair is used to optimise the handling
     * of a module that's only imported once, but constrained by
     * a 'hiding'.
     */
    List  modImports; /* :: [ (modName{-Module-},(Bool,[Entity])) ] */
};


extern Module currentModule;           /* Module currently being processed */
extern struct strModule DECTABLE(tabModule);

extern Bool   isValidModule (Module);
extern Module newModule	    (Text);
extern Module findModule    (Text);
extern Module findModid     (Cell);
extern Void   setCurrModule (Module);
extern List   getModuleImports (Module);

#define isPrelude(m) (m==modulePrelude)

/* --------------------------------------------------------------------------
 * Type constructor names:
 * ------------------------------------------------------------------------*/

#define TYCMIN	     (MODMIN+NUM_MODULE)
#define isTycon(c)   (TYCMIN<=(c) && (c)<NAMEMIN)
#define mkTycon(n)   (TCMIN+(n))
#define tycon(n)     tabTycon[(n)-TYCMIN]

struct strTycon {
    Text  text;
    Int   line;
    Module mod;                         /* module that defines it          */
    Int   arity;
    Kind  kind;				/* kind (includes arity) of Tycon  */
    Cell  what;				/* DATATYPE/SYNONYM/RESTRICTSYN... */
    Cell  defn;
    Tycon nextTyconHash;
    List clashes;
};

extern struct strTycon DECTABLE(tabTycon);

extern Tycon newTycon	  (Text);
extern Tycon findTycon	  (Text);
extern Tycon addTycon	  (Tycon);
extern Void  removeTycon  (Tycon);
extern Tycon findQualTycon (Cell);
extern Tycon addPrimTycon (Text,Kind,Int,Cell,Cell);

#define isSynonym(h)	(isTycon(h) && tycon(h).what==SYNONYM)
#define isNewtype(h)	(isTycon(h) && tycon(h).what==NEWTYPE)
#define isQualType(t)	(isPair(t) && fst(t)==QUAL)
#define mkPolyType(n,t)	pair(POLYTYPE,pair(n,t))
#define isPolyType(t)	(isPair(t) && fst(t)==POLYTYPE)
#define isPolyOrQualType(t) (isPair(t) && (fst(t)==POLYTYPE || fst(t)==QUAL))
#define polySigOf(t)	fst(snd(t))
#define monotypeOf(t)	snd(snd(t))

/* --------------------------------------------------------------------------
 * Globally defined name values:
 * ------------------------------------------------------------------------*/

#define NAMEMIN      (TYCMIN+NUM_TYCON)
#define isName(c)    (NAMEMIN<=(c) && (c)<INSTMIN)
#define mkName(n)    (NAMEMIN+(n))
#define name(n)      tabName[(n)-NAMEMIN]

struct strName {
    Text text;
    Int  line;
    Module mod;                         /* module that defines it          */
    Syntax syntax;
    Cell parent;                        /* enclosing object                */
					/* :: Class | Instance | Name | NIL*/
    Int  arity;
    Int  number;
    Cell type;
    Cell defn;
    Addr code;
    Text extFun;                        /* for foreign import/export       */
    Int  foreignId;                     /* per module identifier           */
    Int  foreignFlags;                  /* encoding safety + calling conv  */
#ifdef DOTNET
    Cell foreignInfo;                   /* (funName,libName,isStatic,isIO,resultType, [paramType])       */
#endif
    Prim primDef;
    Name nextNameHash;
#if PROFILING
    Int  count;
#endif
    List clashes;
};

extern struct strName DECTABLE(tabName);

/* The number field in a name is used to distinguish various kinds of name:
 *   mfunNo(i) = code for member function, offset i
 *		 members that are sole elements of dict use mfunNo(0)
 *		 members of dicts with more than one elem use mfunNo(n), n>=1
 *   EXECNAME  = code for executable name (bytecodes or primitive)
 *   SELNAME   = code for selector function
 *   DFUNNAME  = code for dictionary builder or selector
 *   cfunNo(i) = code for data constructor
 *		 datatypes with only one constructor uses cfunNo(0)
 *		 datatypes with multiple constructors use cfunNo(n), n>=1
 */

#define EXECNAME	0
#define SELNAME		1
#define DFUNNAME	2
#define CFUNNAME	3

#define isSfun(n)	(name(n).number==SELNAME)
#define isDfun(n)	(name(n).number==DFUNNAME)

#define isCfun(n)	(name(n).number>=CFUNNAME)
#define cfunOf(n)	(name(n).number-CFUNNAME)
#define cfunNo(i)	((i)+CFUNNAME)
#define hasCfun(cs)	(nonNull(cs) && isCfun(hd(cs)))

#define isMfun(n)	(name(n).number<0)
#define mfunOf(n)	((-1)-name(n).number)
#define mfunNo(i)	((-1)-(i))

extern Name   newName	    (Text,Cell);
extern Name   findName	    (Text);
extern Name   addName	    (Name);
extern Void   removeName    (Name);
extern Name   findQualName  (Cell);
extern List   findQualNames (Cell);
extern Name   findQualFun   (Text,Text);
extern Void   addPrim	    (Int,Name,String,Module,Type);
extern Name   addPrimCfun   (Text,Int,Int,Cell);
extern Int    sfunPos	    (Name,Name);
extern Bool   setOldDLLFlag (Bool);

/* --------------------------------------------------------------------------
 * Type class values:
 * ------------------------------------------------------------------------*/

#define INSTMIN      (NAMEMIN+NUM_NAME)	/* instances			   */
#define isInst(c)    (INSTMIN<=(c) && (c)<CLASSMIN)
#define mkInst(n)    (INSTMIN+(n))
#define instOf(c)    ((Int)((c)-INSTMIN))
#define inst(in)     tabInst[(in)-INSTMIN]

struct strInst {
    Class c;				/* class C			   */
    Int   line;
    Kinds kinds;			/* Kinds of variables in head	   */
    Cell  head;				/* :: Pred			   */
    List  specifics;			/* :: [Pred]			   */
    Int   numSpecifics;			/* length(specifics)		   */
    List  implements;
    Name  builder;			/* Dictionary constructor function */
};

/* a predicate (an element :: Pred) is an application of a Class to one or
 * more type expressions
 */

#define CLASSMIN     (INSTMIN+NUM_INSTS)
#define isClass(c)   (CLASSMIN<=(c) && (c)<CHARMIN)
#define mkClass(n)   (CLASSMIN+(n))
#define cclass(n)    tabClass[(n)-CLASSMIN]

struct strClass {
    Text   text;			/* Name of class		   */
    Int    line;			/* Line where declaration begins   */
    Module mod;				/* module that declares it         */
    Int    level;			/* Level in class hierarchy	   */
    Int    arity;			/* Number of arguments		   */
    List   tyvars;			/* Names of tyvars		   */
    Kinds  kinds;			/* Kinds of constructors in class  */
    Cell   head;			/* Head of class		   */
    List   fds;				/* Functional Dependencies	   */
    List   xfds;			/* Xpanded Functional Dependencies */
    Name   dcon;			/* Dictionary constructor function */
    List   supers;			/* :: [Pred]			   */
    Int    numSupers;			/* length(supers)		   */
    List   dsels;			/* Superclass dictionary selectors */
    List   members;			/* :: [Name]			   */
    Int    numMembers;			/* length(members)		   */
    List   defaults;			/* :: [Name]			   */
    List   instances;			/* :: [Inst]			   */
    List   clashes;                     /* Conflicting classes/tycons      */
};

extern struct strClass    DYNDECTABLE(tabClass);
extern struct strInst     DYNDECTABLE(tabInst);

extern Class  newClass	     (Text);
extern Class  classMax	     (Void);
extern Class  findClass	     (Text);
extern Class  addClass	     (Class);
extern Class  findQualClass  (Cell);
extern Inst   newInst	     (Void);
extern Inst   findFirstInst  (Tycon);
extern Inst   findNextInst   (Tycon,Inst);
extern Module findQualifier  (Text);
extern List   findQualifiers (Text);
extern Text   findModAlias   (Text);

/* --------------------------------------------------------------------------
 * Character values:
 * ------------------------------------------------------------------------*/

#define CHARMIN      (CLASSMIN+NUM_CLASSES)


/* --------------------------------------------------------------------------
 * Unicode Characters: NUM_SHORT_CHARS is the number of one-cell characters.
 * Such cells are recognized by the new predicate isShortChar().
 * Larger character values are stored as boxed values, in the same way
 * as integers, with predicate, selector and constructor functions.
 * ------------------------------------------------------------------------*/

#define	NUM_SHORT_CHARS	1280	/* number of one-cell characters           */
				/* (Latin, Greek and Cyrillic)             */

#define isShortChar(c)   ((CHARMIN<=(c))&&(INTMIN>(c)))

extern	Bool isChar    (Cell);
extern	Int  charOf    (Cell);
extern	Cell mkChar    (Int);


/* --------------------------------------------------------------------------
 * Small Integer values:
 * ------------------------------------------------------------------------*/

#define INTMIN	     (CHARMIN+NUM_SHORT_CHARS)


/* All ints out of the range defined below are boxed, so we want the biggest
 * INTMAX possible. But the bigger INTMAX is, the bigger the whatIs array.
 * We strike a balance by choosing an integer range of 4096.
 */

#define INTMAX	     (INTMIN+4096)
#define N_WHATCODE   (INTMAX+1)			 /* size of whatCode array */

#define isSmall(c)   (INTMIN<=(c))
#define INTZERO      (INTMIN/2 + INTMAX/2)
#define MINSMALLINT  (INTMIN - INTZERO)
#define MAXSMALLINT  (INTMAX - INTZERO)
#define mkDigit(c)   ((Cell)((c)+INTMIN))
#define digitOf(c)   ((Int)((c)-INTMIN))

extern	Bool isInt    (Cell);
extern	Int  intOf    (Cell);
extern	Cell mkInt    (Int);
extern  Bool isBignum (Cell);

/* --------------------------------------------------------------------------
 * Implementation of triples:
 * ------------------------------------------------------------------------*/

#define triple(x,y,z) pair(x,pair(y,z))
#define fst3(c)      fst(c)
#define snd3(c)      fst(snd(c))
#define thd3(c)      snd(snd(c))

/* --------------------------------------------------------------------------
 * Implementation of lists:
 * ------------------------------------------------------------------------*/

#define NIL	     0
#define isNull(c)    ((c)==NIL)
#define nonNull(c)   (c)
#define cons(x,xs)   pair(x,xs)
#define singleton(x) cons(x,NIL)
#define doubleton(x,y) cons(x,cons(y,NIL))
#define hd(c)	     fst(c)
#define tl(c)	     snd(c)

extern	Int	     length	   (List);
extern	List	     appendOnto    (List,List);	/* destructive	   */
extern	List	     dupOnto       (List,List);
extern  List	     dupList	   (List);
extern  List	     dupUpto	   (List,Cell);
extern	List	     revOnto	   (List, List);	/* destructive	   */
#define rev(xs)      revOnto((xs),NIL)			/* destructive	   */
extern	Cell	     cellIsMember  (Cell,List);
extern  Cell         cellAssoc     (Cell,List);
extern  Cell         cellRevAssoc  (Cell,List);
extern	List	     concat	   (List);
extern	List	     intersect	   (List,List);
extern	Cell	     varIsMember   (Text,List);
extern	Name	     nameIsMember  (Text,List);
extern	Name	     nameInIEList  (Name,List);
extern	Tycon	     tyconInIEList (Text,List);
extern	Class	     classInIEList (Text,List);
extern  Cell	     intIsMember   (Int,List);
extern	List	     replicate	   (Int,Cell);
extern	List	     diffList	   (List,List);	/* destructive	   */
extern  List         deleteCell    (List,Cell);	/* non-destructive */
extern  List	     take	   (Int,List);	/* destructive	   */
extern  List	     splitAt	   (Int,List);	/* non-destructive */
extern  Cell	     nth	   (Int,List);
extern  List	     removeCell	   (Cell,List);	/* destructive	   */
extern  List         nubList       (List);        /* non-destructive */

/* The following macros provide `inline expansion' of some common ways of
 * traversing, using and modifying lists:
 *
 * N.B. We use the names _f, _a, _xs, Zs, in an attempt to avoid clashes
 *	with identifiers used elsewhere.
 */

#define mapBasic(_init,_step)		{List Zs=(_init);\
					 for(;nonNull(Zs);Zs=tl(Zs))  \
					 _step;}
#define mapModify(_init,_step)		mapBasic(_init,hd(Zs)=_step)

#define mapProc(_f,_xs)			mapBasic(_xs,_f(hd(Zs)))
#define map1Proc(_f,_a,_xs)		mapBasic(_xs,_f(_a,hd(Zs)))
#define map2Proc(_f,_a,_b,_xs)		mapBasic(_xs,_f(_a,_b,hd(Zs)))
#define map3Proc(_f,_a,_b,_c,_xs)	mapBasic(_xs,_f(_a,_b,_c,hd(Zs)))
#define map4Proc(_f,_a,_b,_c,_d,_xs)	mapBasic(_xs,_f(_a,_b,_c,_d,hd(Zs)))

#define mapOver(_f,_xs)			mapModify(_xs,_f(hd(Zs)))
#define map1Over(_f,_a,_xs)		mapModify(_xs,_f(_a,hd(Zs)))
#define map2Over(_f,_a,_b,_xs)		mapModify(_xs,_f(_a,_b,hd(Zs)))
#define map3Over(_f,_a,_b,_c,_xs)	mapModify(_xs,_f(_a,_b,_c,hd(Zs)))
#define map4Over(_f,_a,_b,_c,_d,_xs)	mapModify(_xs,_f(_a,_b,_c,_d,hd(Zs)))

/* This is just what you want for functions with accumulating parameters */
#define mapAccum(_f,_acc,_xs) 	        mapBasic(_xs,_acc=_f(_acc,hd(Zs)))
#define map1Accum(_f,_acc,_a,_xs)	mapBasic(_xs,_acc=_f(_acc,_a,hd(Zs)))
#define map2Accum(_f,_acc,_a,_b,_xs)	mapBasic(_xs,_acc=_f(_acc,_a,_b,hd(Zs)))
#define map3Accum(_f,_acc,_a,_b,_c,_xs) mapBasic(_xs,_acc=_f(_acc,_a,_b,_c,hd(Zs)))

/* --------------------------------------------------------------------------
 * Implementation of function application nodes:
 * ------------------------------------------------------------------------*/

#define ap(f,x)      pair(f,x)
#define ap1(f,x)     ap(f,x)
#define ap2(f,x,y)   ap(ap(f,x),y)
#define ap3(f,x,y,z) ap(ap(ap(f,x),y),z)
#define fun(c)	     fst(c)
#define arg(c)	     snd(c)
#define isAp(c)      (isPair(c) && !isTag(fst(c)))
extern	Cell	     getHead	 (Cell);
extern	List	     getArgs	 (Cell);
extern	Int	     argCount;
extern  Cell         nthArg	 (Int,Cell);
extern  Int	     numArgs	 (Cell);
extern  Cell	     applyToArgs (Cell,List);

/* --------------------------------------------------------------------------
 * Stack implementation:
 *
 * NB: Use of macros makes order of evaluation hard to predict.
 *     For example, "push(1+pop());" doesn't increment TOS.
 * ------------------------------------------------------------------------*/

extern  Cell DECTABLE(cellStack);
#ifdef  GLOBALsp
register StackPtr    sp GLOBALsp;
#else
extern	StackPtr sp;
#endif

#define clearStack() sp=(-1)
#define stackEmpty() (sp==(-1))
#define stack(p)     cellStack[p]
#define chkStack(n)  if (sp>=NUM_STACK-(n)) hugsStackOverflow()
#define push(c)      \
  do {               \
    chkStack(1);     \
    onto(c);         \
  } while (0)
#define onto(c)	     stack(++sp)=(c)
#define pop()	     stack(sp--)
#define drop()	     sp--
#define top()	     stack(sp)
#define pushed(n)    stack(sp-(n))
#define topfun(f)    top()=ap((f),top())
#define toparg(x)    top()=ap(top(),(x))

extern  Void hugsStackOverflow (Void);

#define STACK_CHECK

/* --------------------------------------------------------------------------
 * Script file control:
 * The implementation of script file storage is hidden.
 * ------------------------------------------------------------------------*/

extern Script	   startNewScript   (String);
extern Bool        moduleThisScript (Module);
extern Module      moduleOfScript   (Script);
extern Bool        isPreludeScript  (Void);
extern Module      lastModule       (Void);
extern Script	   scriptThisFile   (Text);
extern Script	   scriptThisName   (Name);
extern Script	   scriptThisTycon  (Tycon);
extern Script	   scriptThisInst   (Inst);
extern Script	   scriptThisClass  (Class);
extern String      fileOfModule     (Module);
extern Void	   dropAScript      (Script);
extern Void	   dropScriptsFrom  (Script);
extern Void        setScriptPrims   (void*);

/* --------------------------------------------------------------------------
 * I/O Handles:
 * ------------------------------------------------------------------------*/

#define HSTDIN		0	/* Numbers for standard handles		   */
#define HSTDOUT		1
#define HSTDERR		2
#define IS_STANDARD_HANDLE(h)  ((h) <= 2)

struct strHandle {		/* Handle description and status flags	   */
    Cell  hcell;		/* Heap representation of handle (or NIL)  */
    FILE  *hfp;			/* Corresponding file pointer		   */
    Int   hmode;		/* Current mode: see below		   */
    Int   hbufMode;             /* Buffering mode.                         */
    Int   hbufSize;             /* < 0 => not yet known.                   */
    Int   hRWState;		/* State of a READWRITE handle (see below) */
    Bool  hBinaryMode;		/* TRUE => Handle opened in binary mode    */
    Char  hLookAhead;		/* Char read by hLookAhead (or <0 if none) */
    				/* This is only used in text mode.         */
};

#define HCLOSED		0000	/* no I/O permitted			   */
#define HSEMICLOSED     0001	/* semiclosed reads only		   */
#define HREAD		0002	/* set to enable reads from handle 	   */
#define HWRITE		0004	/* set to enable writes to handle	   */
#define HAPPEND		0010	/* opened in append mode		   */
#define HREADWRITE      0020    /* set to enable both reading and writing  */

#define HUNKNOWN_BUFFERING (-1) /* the buffering mode of a handle is lazily
				   determined. */

#define HANDLE_NOTBUFFERED    1
#define HANDLE_LINEBUFFERED   2
#define HANDLE_BLOCKBUFFERED  3

#define RW_NEUTRAL            0 /* r/w stream was just opened/at EOF/seeked */
#define RW_READING            1 /* last operation on r/w stream was reading */
#define RW_WRITING            2 /* last operation on r/w stream was writing */

#define	NO_HANDLE          (-1)

extern unsigned long     num_handles;
extern DynTable*         dynTabHandles;
extern struct strHandle* handles;

/* --------------------------------------------------------------------------
 * Malloc Pointers
 * ------------------------------------------------------------------------*/

struct strMallocPtr {		/* Malloc Ptr description                  */
    Cell mpcell;		/* Back pointer to MPCELL                  */
    Void *ptr;			/* Pointer into C world                    */
    Int  refCount;              /* Reference count                         */
    Cell finalizers;		/* List of finalizer functions             */
    				/* or (finalizer, envptr) pairs		   */
};

typedef Void (*CFinalizer)       (Pointer);
typedef Void (*CFinalizerEnv)    (Pointer, Pointer);

extern struct strMallocPtr       mallocPtrs[];
extern Cell   newMallocPtr       (Void *);
extern Cell   mkMallocPtr        (Void *, CFinalizer);
extern Void   freeMallocPtr      (Cell);
extern Void   incMallocPtrRefCnt (Int, Int);

extern Int    mallocPtr_hw;

#define mpOf(c)	   snd(c)
#define derefMP(c) (mallocPtrs[(Int)mpOf(c)].ptr)

/* --------------------------------------------------------------------------
 * Weak Pointers
 * ------------------------------------------------------------------------*/

#define mkWeakPtr(c)    pair(WEAKCELL,pair(c,NIL))
#define derefWeakPtr(c) fst(snd(c))
#define nextWeakPtr(c) snd(snd(c))

extern List finalizers;
extern List liveWeakPtrs;


/* --------------------------------------------------------------------------
 * Foreign Function Interface
 * ------------------------------------------------------------------------*/

#include "HsFFI.h"

extern Int              part1Int64      (HsInt64);
extern Int              part2Int64      (HsInt64);
extern HsInt64          int64FromParts  (Int,Int);

/* --------------------------------------------------------------------------
 * Stable pointers
 * ------------------------------------------------------------------------*/

extern  Int         mkStablePtr    (Cell);
extern  Cell        derefStablePtr (Int);
extern  Void        freeStablePtr  (Int);

/* --------------------------------------------------------------------------
 * Misc:
 * ------------------------------------------------------------------------*/

extern  Void   setLastExpr       (Cell);
extern  Cell   getLastExpr       (Void);
extern  List   addTyconsMatching (String,List);
extern  List   addNamesMatching  (String,List);

/*-------------------------------------------------------------------------*/


/* whatIs1 is faster than whatIs, but it will return NIL if fst(c) is NIL
 * Used with care in the right places it gains us speed.
 */

#define isTag1(c)   (((c) & TAG_MASK) == 0)	    /* doesn't exclude NIL */

#define whatIs1(c)  (isPair(c)? (isTag1(fst(c))? fst(c) : AP ) : whatCode[c])

#define isAp1(c)    (isPair(c) && !isTag1(fst(c)))


#endif /* __STORAGE_H__ */
