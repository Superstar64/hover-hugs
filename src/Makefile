# --------------------------------------------------------------------------
# Makefile for Hugs interpreter
#
# The Hugs 98 system is Copyright (c) Mark P Jones, Alastair Reid, the
# Yale Haskell Group, and the OGI School of Science & Engineering at OHSU,
# 1994-2004, All rights reserved.  It is distributed as
# free software under the license in the file "License", which is
# included in the distribution.
# --------------------------------------------------------------------------

# Targets:
#
# <default>:   	  make hugs and runhugs
# install:	  make and install programs
# hugs:	  make minimal working system
# runhugs: make batch-mode version of Hugs
# clean:       	  delete files not required in running system
# distclean:      delete files that can be regenerated using C compiler
# veryclean:      delete all machine generated files 
#                   (you need perl, bison/yacc, etc to rebuild these files)
# TAGS:           build emacs TAGS table

OPTFLAGS	= -O2

LIBS		= -lreadline -lncurses -lm -ldl 

.SUFFIXES	:
.SUFFIXES	: .c .h .o

HFILES		= HsFFI.h builtin.h char.h command.h config.h connect.h \
		  errors.h evaluator.h goal.h machdep.h module.h observe.h \
		  options.h opts.h output.h prelude.h script.h server.h \
		  storage.h strutil.h subst.h unitable.h
CFILES		= hugs.c runhugs.c server.c edit.c observe.c \
		  builtin.c char.c compiler.c errors.c evaluator.c ffi.c \
		  goal.c input.c machdep.c machine.c module.c opts.c \
		  output.c plugin.c script.c static.c storage.c strutil.c \
		  subst.c type.c version.c
YFILES		= parser.y
SOURCES		= $(HFILES) $(CFILES) $(YFILES) 
OBJECTS		= builtin.o char.o compiler.o \
		  errors.o evaluator.o ffi.o \
		  goal.o input.o machdep.o \
		  machine.o module.o opts.o \
		  output.o plugin.o script.o \
		  static.o storage.o strutil.o \
		  subst.o type.o version.o unitable.o
IOBJECTS	= hugs.o edit.o observe.o $(OBJECTS)

################################################################
# Default target
################################################################

# This rule goes first to make it the default choice
default		: all

all		: hugs runhugs ffihugs

################################################################
# Hugs interpreter and standalone evaluator
################################################################

hugs	: $(IOBJECTS)
		  $(CC) $(LDFLAGS) $(IOBJECTS) $(LIBS) -o hugs

SERVER_OBJECTS	= server.o $(OBJECTS) 
runhugs : runhugs.o $(SERVER_OBJECTS)
		  $(CC) $(LDFLAGS) runhugs.o $(SERVER_OBJECTS) $(LIBS) -o runhugs

ffihugs : ffihugs.o $(SERVER_OBJECTS)
		  $(CC) $(LDFLAGS) ffihugs.o $(SERVER_OBJECTS) $(LIBS) -o ffihugs

ffihugs.o : runhugs.o
		  $(CC) -c $(CFLAGS) $(OPTFLAGS) -DFFI_COMPILER runhugs.c -o ffihugs.o


################################################################
# C and Yacc rules
################################################################

.c.o	:
		  $(CC) -c $(CFLAGS) $(OPTFLAGS) $<

# Modules to be compiled without optimization.
# (old comment: to avoid optimisation bugs in certain compilers.
#               This may be overly conservative on some compilers.)
# (The following explanation is based on a posting by Alastair Reid.)
# These modules allocate cells on the Hugs heap and assume a conservative
# garbage collector.  On some (especially RISC) architectures, the
# optimizer may identify a pointer to a Cell as a common subexpression,
# and hold that instead of the Cell.  This would then be missed by the
# conservative garbage collector's simplistic scan of the C stack.
# Modules associated with evaluation are safe because they don't assume
# conservative GC (see IMPORTANT NOTICE in builtin.c).
compiler.o : compiler.c
		  $(CC) -c $(CFLAGS) compiler.c
module.o : module.c
		  $(CC) -c $(CFLAGS) module.c
subst.o	: subst.c
		  $(CC) -c $(CFLAGS) subst.c
static.o : static.c
		  $(CC) -c $(CFLAGS) static.c
type.o	: type.c
		  $(CC) -c $(CFLAGS) type.c

parser.c	: parser.y
		  -$(YACC) parser.y
		  mv y.tab.c parser.c

unitable.c: mkunitable UnicodeData-4.0.0.txt
	./mkunitable UnicodeData-4.0.0.txt > unitable.c

UnicodeData-4.0.0.txt:
	wget https://www.unicode.org/Public/4.0-Update/UnicodeData-4.0.0.txt -o 

UnicodeData-License.txt:
	wget https://www.unicode.org/license.txt -O UnicodeData-License.txt

################################################################
# Generating object dependencies (requires gcc)
################################################################

depend: $(SOURCES) parser.c unitable.c
	gcc -MM $(CFILES) parser.c unitable.c > MkDepend.mk

################################################################
# Checking header files (requires gcc)
################################################################

check: $(HFILES:%.h=%.h-check)

%.h-check: %.h
	gcc -fsyntax-only $<

################################################################
# Object dependencies
################################################################

-include MkDepend.mk

################################################################
# End of Makefile
################################################################
