## Hello emacs, this is -*- makefile -*-
## $Id$
## André Rabello <Andre.Rabello@ufrj.br>

## Local definitions
SRCDIR= src
SRCFILE= fifo.c bound.c cdf.c encode.c decode.c
TSTFILE= test_encode.c test_cdf.c test_decode.c
INCDIR= include
LIBS= gmp arith
LIBDIR= .
##DEBUG= DEBUGGING # descomente caso deseje usar `malloc-traccing'
DEFINES= $(DEBUG)

## Compiler definitions
CC= gcc
COMMONFLAGS= -O2 -Wall -ansi -pedantic
INCLUDE= $(INCDIR:%=-I%)
CXXFLAGS= $(COMMONFLAGS) $(INCLUDE)
CFLAGS= $(COMMONFLAGS) $(INCLUDE)
CPPFLAGS = $(DEFINES:%=-D%)
SRC= $(SRCFILE:%=$(SRCDIR)/%)
OBJ= $(SRC:%.c=%.o)
TSTNAME = $(TSTFILE:%.c=%)
LDFLAGS= $(COMMONFLAGS) $(LIBDIR:%=-L%) 
LDLIBS= $(LIBS:%=-l%)
MKDEPFILE= .depend
MKDEP= makedepend -f$(MKDEPFILE)

## Build Rules
all: depend libarith.a doc

libarith.a: $(OBJ)
	ar -ruvs $@ $(OBJ) 

test: $(TSTNAME)

test_%: $(SRCDIR)/test_%.o libarith.a
	$(CC) $(LDFLAGS) $< $(LDLIBS) -o $@

.PHONY: clean depend tag doc

doc:
	$(MAKE) -C doc

depend:
	@if [ ! -e $(MKDEPFILE) ]; then touch $(MKDEPFILE); fi
	$(MKDEP) $(INCLUDE) $(SRC) $(TSTFILE:%=$(SRCDIR)/%)

## Clean-up
GARBAGE = "*~" "*.o" "*.a" "TAGS" "train" "$(MKDEPFILE:%=%*)" \
	  $(TSTNAME) core "*.mesg"
GARBDIR = .
FINDCLEANOPT = -a -type f -print0 #-maxdepth 1
clean:
	@for i in $(GARBAGE); do \
	  find $(GARBDIR) -name "$$i" $(FINDCLEANOPT) | xargs -0tr rm -f;\
	 done
	$(MAKE) -C doc clean

tags:
	@find . -name '*.h' -or -name '*.c' | etags -

## Dependencies (not obligatory)
sinclude $(MKDEPFILE)
