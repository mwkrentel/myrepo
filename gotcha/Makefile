#
#  Build libs and apps for testing gotcha.
#  First, run:  ./setup path/to/spack
#
#  Copyright (c) 2019, Rice University.
#  See the file LICENSE for details.
#
#  Mark W. Krentel
#  August 2019
#
include makefile.incl

# Application compiler
CC = gcc
CXX = g++

CFLAGS = -g  -O
OPENMP = -fopenmp
PIC = -fPIC

# Tools compiler
GCC = gcc
GXX = g++

SAFE_FLAGS = -fno-builtin-malloc  -fno-builtin-calloc  \
	-fno-builtin-realloc  -fno-builtin-free

RPATH_TOP = -Wl,-rpath=$(TOPDIR)
RPATH_GOT = -Wl,-rpath=$(GOTCHA)/lib64
RPATH_TCM = -Wl,-rpath=$(TCMALLOC)/lib  -Wl,-rpath=$(UNWIND)/lib

#------------------------------------------------------------

PROGS = reduce  red-tcmalloc  red-minimal

LIBS = libmygotcha.so  libpreload.so  libhybrid.so

all: progs libs

progs: $(PROGS)

libs:  $(LIBS)

.PHONY:  all  progs  libs  clean  distclean

#------------------------------------------------------------

reduce: reduce.c libreduce.so
	$(CC)  -o $@  $<  $(CFLAGS)  $(OPENMP)  $(RPATH_TOP)  \
	-L$(TOPDIR)  -lreduce

libreduce.so: libreduce.c
	$(CC)  -o $@  $<  $(CFLAGS)  -shared  $(PIC)  $(OPENMP)  -ldl

red-tcmalloc: reduce.c libreduce.so
	$(CC)  -o $@  $<  $(CFLAGS)  $(OPENMP)  $(RPATH_TOP)  $(RPATH_TCM)  \
	$(SAFE_FLAGS)  \
	-L$(TOPDIR)  -lreduce  -L$(TCMALLOC)/lib -ltcmalloc

red-minimal: reduce.c libreduce.so
	$(CC)  -o $@  $<  $(CFLAGS)  $(OPENMP)  $(RPATH_TOP)  $(RPATH_TCM)  \
	$(SAFE_FLAGS)  \
	-L$(TOPDIR)  -lreduce  -L$(TCMALLOC)/lib -ltcmalloc_minimal

libmygotcha.so: gotcha.c
	$(GCC)  -o $@  $<  $(CFLAGS)  -shared  $(PIC)  \
	-I $(GOTCHA)/include  -L $(GOTCHA)/lib64 -lgotcha  $(RPATH_GOT)

libpreload.so: preload.c
	$(GCC)  -o $@  $<  $(CFLAGS)  -shared  $(PIC)  -ldl

libhybrid.so: hybrid.c
	$(GCC)  -o $@  $<  $(CFLAGS)  -shared  $(PIC)  -ldl  \
	-I $(GOTCHA)/include  -L $(GOTCHA)/lib64 -lgotcha  $(RPATH_GOT)

clean:
	rm -f  $(PROGS)  $(LIBS)  *~  *.o  *.so

distclean: clean
	rm -f  makefile.incl  run-gotcha  run-hybrid  run-preload

