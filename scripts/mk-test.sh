#!/bin/sh
#
#  Copyright (c) 2017-2018, Rice University.
#  See the file LICENSE for details.
#
#  Mark W. Krentel
#  March 2017, revised October 2017
#
#  This script uses a spack install directory to build a Dyninst
#  application, especially an app for the new-parallel-parsing branch.
#  It adds the include and lib directories for dyninst and its prereqs
#  (boost, elfutils, dwarf, tbb, etc) to the compile line.  To build a
#  Dyninst app:
#
#   1. use the recipes in this repo to build dyninst's prereqs,
#   2. use the mk-dyninst.sh script to build dyninst,
#   3. edit and run this script.
#
#  Usage: edit this file and run:
#   ./mk-test.sh [options] ... main.cpp
#
#  Note: paths should be relative to the directory in which this
#  script is run.  If the build succeeds, the script writes the file
#  'env.sh' with shell commands to set LD_LIBRARY_PATH.
#

DYNINST_INSTALL=install-dyninst
SPACK_INSTALL=path/to/spack/linux-redhat-x86/gcc-x.y.z

CXX=g++
CXXFLAGS='-g -O -std=c++11 -fopenmp'

environ=env.sh

topdir=`/bin/pwd`

#------------------------------------------------------------

# Returns: full path of subdir of SPACK_INSTALL whose name begins with
# 'base', the most recent one if there are multiple subdirs.
#
spackdir()
{
    base="$1"
    ans=no

    cd "$ABS_SPACK"

    # use symlink, if set
    if test -d "$base" ; then
	echo "${ABS_SPACK}/$base"
	return
    fi

    # use most recent dir, if more than one
    for f in ${base}-* ; do
        if test -d "$f" ; then
            new="${ABS_SPACK}/$f"
            if test "$ans" = no || test "$new" -nt "$ans" ; then
                ans="$new"
            fi
        fi
    done

    if test "$ans" = no ; then
        echo "unable to find: $base" 1>&2
    fi

    echo $ans
}

die() {
    if test "x$1" != x ; then
	echo "mk-test: error: $@"
    fi
    cat - <<EOF
usage: ./mk-test.sh [options] ... main.cpp

  options = options added to the compile line
  main.cpp = C++ source file(s)

EOF
    exit 1
}

#------------------------------------------------------------

output=

for opt in "$@" ; do
    case "$opt" in
	*.cpp | *.cxx ) output=`expr "$opt" : '\(.*\)....'` ;;
	*.cc ) output=`expr "$opt" : '\(.*\)...'` ;;
	*.C ) output=`expr "$opt" : '\(.*\)..'` ;;
    esac
    if test "x$output" != x ; then break ; fi
done

test "x$output" != x || die "missing source file"

#------------------------------------------------------------

# Set these only if necessary.

case "$DYNINST_INSTALL" in
    /* ) ABS_DYNINST="$DYNINST_INSTALL" ;;
    * )  ABS_DYNINST="${topdir}/$DYNINST_INSTALL" ;;
esac

case "$SPACK_INSTALL" in
    /* ) ABS_SPACK="$SPACK_INSTALL" ;;
    * )  ABS_SPACK="${topdir}/$SPACK_INSTALL" ;;
esac

dyninst_inc="${ABS_DYNINST}/include"
dyninst_lib="${ABS_DYNINST}/lib"

boost=`spackdir boost`
boost_inc="${boost}/include"
boost_lib="${boost}/lib"

libelf=`spackdir elfutils`
libelf_inc="${libelf}/include"
libelf_lib="${libelf}/lib"

libdwarf="$libelf"
libdwarf_inc="${libdwarf}/include"
libdwarf_lib="${libdwarf}/lib"

tbb_root=`spackdir intel-tbb`
tbb_inc="${tbb_root}/include"
tbb_lib="${tbb_root}/lib"

bzip=`spackdir bzip2`
bzip_lib="${bzip}/lib"

lzma=`spackdir xz`
lzma_lib="${lzma}/lib"

zlib=`spackdir zlib`
zlib_lib="${zlib}/lib"

#------------------------------------------------------------

set --  \
    -o "$output"  \
    $CXXFLAGS  \
    "$@"  \
    -I${dyninst_inc}  \
    -I${boost_inc}  \
    -I${libelf_inc}  \
    -I${libdwarf_inc}  \
    -I${tbb_inc}  \
    -L${dyninst_lib}  \
    -lparseAPI  -linstructionAPI  -lsymtabAPI  \
    -ldynDwarf  -ldynElf  -lcommon  \
    -L${boost_lib}  -lboost_system  \
    -L${libelf_lib}  -lelf  \
    -L${libdwarf_lib}  -ldw  \
    -L${tbb_lib}  -ltbb  \
    -ldl

echo $CXX $@
echo

$CXX "$@"

test $? -eq 0 || die "compile failed"

#------------------------------------------------------------

# If the program built ok, then write env.sh with shell commands to
# set LD_LIBRARY_PATH.

rm -f "$environ"

cat >"$environ" <<EOF
#
#  Source me to set LD_LIBRARY_PATH
#

LD_LIBRARY_PATH="${dyninst_lib}:\${LD_LIBRARY_PATH}"
LD_LIBRARY_PATH="${boost_lib}:\${LD_LIBRARY_PATH}"
LD_LIBRARY_PATH="${libelf_lib}:\${LD_LIBRARY_PATH}"
LD_LIBRARY_PATH="${libdwarf_lib}:\${LD_LIBRARY_PATH}"
LD_LIBRARY_PATH="${tbb_lib}:\${LD_LIBRARY_PATH}"
LD_LIBRARY_PATH="${bzip_lib}:\${LD_LIBRARY_PATH}"
LD_LIBRARY_PATH="${lzma_lib}:\${LD_LIBRARY_PATH}"
LD_LIBRARY_PATH="${zlib_lib}:\${LD_LIBRARY_PATH}"

export LD_LIBRARY_PATH
EOF

