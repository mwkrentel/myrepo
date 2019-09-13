#
#  Build libs and apps for testing gotcha.
#
#  ./setup  path/to/spack/tools-compiler/dir  \
#      [ path/to/spack/app-compiler/dir ]
#
#  Copyright (c) 2019, Rice University.
#  See the file LICENSE for details.
#
#  Mark W. Krentel
#  August 2019
#
include makefile.incl

# Tools compiler, normally GNU, for libmygotcha.so, etc.
GCC = gcc

CFLAGS = -g  -O
PIC = -fPIC

# Application compiler, any family, for reduce, etc.
CC = $(GCC)

APP_CFLAGS = $(CFLAGS)
OPENMP = -fopenmp

SAFE_FLAGS = -fno-builtin-malloc  -fno-builtin-calloc  \
	-fno-builtin-realloc  -fno-builtin-free

RPATH_TOP = -Wl,-rpath=$(TOPDIR)
RPATH_GOT = -Wl,-rpath=$(GOTCHA)/lib64
RPATH_TCM = -Wl,-rpath=$(TCMALLOC)/lib  -Wl,-rpath=$(UNWIND)/lib

#------------------------------------------------------------

TOOLS = libmygotcha.so  libpreload.so  libhybrid.so  \
	preinit.o  tool-init.o  wrap.o

APPS = reduce  preinit  simple  main  wrap-main  wrap-reduce

TC_APPS = reduce-tcmalloc  reduce-minimal

SCRIPTS = run-gotcha  run-preload  run-hybrid

most: tools apps

all:  tools apps tc-apps

tools: $(TOOLS)

apps:  $(APPS)

tc-apps:  $(TC_APPS)

.PHONY:  all  tools  apps  tc-apps  clean  distclean

#------------------------------------------------------------

# GNU compiler for gotcha and preload overrides

libmygotcha.so: gotcha.c
	$(GCC)  -o $@  $<  $(CFLAGS)  -shared  $(PIC)  \
	-I $(GOTCHA)/include  -L $(GOTCHA)/lib64 -lgotcha  $(RPATH_GOT)

libpreload.so: preload.c
	$(GCC)  -o $@  $<  $(CFLAGS)  -shared  $(PIC)  -ldl

libhybrid.so: hybrid.c
	$(GCC)  -o $@  $<  $(CFLAGS)  -shared  $(PIC)  -ldl  \
	-I $(GOTCHA)/include  -L $(GOTCHA)/lib64 -lgotcha  $(RPATH_GOT)

preinit.o: preinit.c
	$(GCC)  -c  -o $@  $<  $(CFLAGS)  -I $(GOTCHA)/include

tool-init.o: tool-init.c
	$(GCC)  -c  -o $@  $<  $(CFLAGS)  -I $(GOTCHA)/include

wrap.o: wrap.c  libmygotcha.so
	$(GCC)  -c  -o $@  $<  $(CFLAGS)  -I $(GOTCHA)/include

#------------------------------------------------------------

# Application compiler, any family

libreduce.so: libreduce.c
	$(CC)  -o $@  $<  $(APP_CFLAGS)  -shared  $(PIC)  $(OPENMP)  -ldl

reduce: reduce.c  libreduce.so
	$(CC)  -o $@  $<  $(APP_CFLAGS)  $(OPENMP)  $(RPATH_TOP)  \
	-L$(TOPDIR)  -lreduce  -ldl

wrap-reduce: reduce.c  libreduce.so  wrap.o
	$(CC)  -o $@  $<  $(APP_CFLAGS)  $(OPENMP)  $(RPATH_TOP)  \
	-L$(TOPDIR)  -lreduce  -ldl  \
	-Wl,--wrap=main  wrap.o  \
	-L $(GOTCHA)/lib64 -lgotcha  $(RPATH_GOT)

reduce-tcmalloc: reduce.c libreduce.so
	$(CC)  -o $@  $<  $(APP_CFLAGS)  $(OPENMP)  $(SAFE_FLAGS)  \
	$(RPATH_TOP)  $(RPATH_TCM)  \
	-L$(TOPDIR)  -lreduce  -L$(TCMALLOC)/lib -ltcmalloc  -ldl

reduce-minimal: reduce.c libreduce.so
	$(CC)  -o $@  $<  $(APP_CFLAGS)  $(OPENMP)  $(SAFE_FLAGS)  \
	$(RPATH_TOP)  $(RPATH_TCM)  \
	-L$(TOPDIR)  -lreduce  -L$(TCMALLOC)/lib -ltcmalloc_minimal  -ldl

preinit: reduce.c libreduce.so preinit.o
	$(CC)  -o $@  $<  $(APP_CFLAGS)  $(OPENMP)  $(RPATH_TOP)  \
	-L$(TOPDIR)  -lreduce  -ldl  \
	preinit.o  -L $(GOTCHA)/lib64 -lgotcha  $(RPATH_GOT)

simple: simple.c tool-init.o
	$(CC)  -o $@  $<  $(APP_CFLAGS)  $(OPENMP)  -ldl  \
	tool-init.o  -L $(GOTCHA)/lib64 -lgotcha  $(RPATH_GOT)

libside.so: side.c
	$(CC)  -o $@  $<  $(APP_CFLAGS)  -shared  $(PIC)

main: main.c  libside.so
	$(CC)  -o $@  $<  $(APP_CFLAGS)  $(RPATH_TOP)  \
	-L$(TOPDIR)  -lside

wrap-main: main.c  libside.so  wrap.o
	$(CC)  -o $@  $<  $(APP_CFLAGS)  $(RPATH_TOP)  \
	-L$(TOPDIR)  -lside  \
	-Wl,--wrap=main  wrap.o  \
	-L $(GOTCHA)/lib64 -lgotcha  $(RPATH_GOT)

#------------------------------------------------------------

clean:
	rm -f  $(TOOLS)  $(APPS)  $(TC_APPS)  *~  *.o  *.so

distclean: clean
	rm -f  $(SCRIPTS)  makefile.incl

