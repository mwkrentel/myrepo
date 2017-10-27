#!/bin/sh
#
#  Copyright (c) 2017, Rice University.
#  See the file LICENSE for details.
#
#  Mark W. Krentel
#  October 2017
#
#  This script uses a spack install directory to manually configure
#  and make Dyninst (outside of spack), especially the new-parallel-
#  parsing branch.  To build Dyninst:
#
#   1. use the recipes in this repo to build dyninst's prereqs,
#   2. clone a copy of the dyninst repository,
#   3. edit and run this script.
#
#  Usage: edit this file and run ./mk-dyninst.sh
#  Options:
#   -C   rerun cmake config (else, just run make)
#   -V   turn on verbose make output
#   -X   stop after config, don't run make
#
#  Note: paths should be relative to the directory in which this
#  script is run.  Also, CC and CXX will be converted to full paths
#  before passing them to cmake.
#

DYNINST_ROOT=dyninst
SPACK_INSTALL=path/to/spack/linux-redhat-x86/gcc-x.y.z

BUILD_TYPE=RelWithDebInfo

CC=gcc
CXX=g++

BUILD=build-dyninst
INSTALL=install-dyninst

jobs='-j 4'

build_subdirs='common elf dwarf symtabAPI instructionAPI parseAPI'

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
    echo "mk-dyninst: error: $@"
    exit 1
}

#------------------------------------------------------------

do_config=no
do_verbose=no
do_make=yes

for opt in "$@" ; do
    case "$opt" in
	-C ) do_config=yes ;;
	-V ) do_verbose=yes ;;
	-X ) do_make=no ;;
	* ) echo "unknown option: $opt" ;;
    esac
done

#------------------------------------------------------------

# Set these only if necessary.

case "$INSTALL" in
    /* ) PREFIX="$INSTALL" ;;
    * )  PREFIX="${topdir}/$INSTALL" ;;
esac

case "$SPACK_INSTALL" in
    /* ) ABS_SPACK="$SPACK_INSTALL" ;;
    * )  ABS_SPACK="${topdir}/$SPACK_INSTALL" ;;
esac

case "$CC" in
    /* ) ;;
    * ) CC=`type $CC | awk '{print $3}'` ;;
esac

case "$CXX" in
    /* ) ;;
    * ) CXX=`type $CXX | awk '{print $3}'` ;;
esac

boost_root=`spackdir boost`

# elf, dwarf and libiberty libs are the actual library
libelf=`spackdir elfutils`
libelf_inc="${libelf}/include"
libelf_libs="${libelf}/lib/libelf.so"

libdwarf="$libelf"
libdwarf_inc="${libdwarf}/include"
libdwarf_libs="${libdwarf}/lib/libdw.so"

libiberty=`spackdir libiberty`
libiberty_libs="${libiberty}/lib64/libiberty.a"

# tbb lib is the directory, not the library
tbb_root=`spackdir intel-tbb`
tbb_inc="${tbb_root}/include"
tbb_lib_dir="${tbb_root}/lib"

#------------------------------------------------------------

set --  \
    -DCMAKE_INSTALL_PREFIX="$PREFIX"  \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE"  \
    -DBUILD_RTLIB=Off  \
    -DBUILD_TARBALLS=Off  \
    -DBUILD_DOCS=Off  \
    -DCMAKE_C_COMPILER="$CC"  \
    -DCMAKE_CXX_COMPILER="$CXX"  \
    -DPATH_BOOST="$boost_root"  \
    -DLIBELF_INCLUDE_DIR="$libelf_inc"  \
    -DLIBELF_LIBRARIES="$libelf_libs"  \
    -DLIBDWARF_INCLUDE_DIR="$libdwarf_inc"  \
    -DLIBDWARF_LIBRARIES="$libdwarf_libs"  \
    -DIBERTY_LIBRARIES="$libiberty_libs"  \
    -DTBB_ROOT_DIR="$tbb_root"  \
    -DTBB_INCLUDE_DIR="$tbb_inc"  \
    -DTBB_LIBRARY="$tbb_lib_dir"

#------------------------------------------------------------

if test "$do_config" = yes ; then
    echo "===> configure"

    cd "$topdir" || die "unable to cd: $topdir"

    rm -rf "$BUILD" "$INSTALL"
    mkdir  "$BUILD" "$INSTALL"
    cd "$BUILD" || die "unable to cd: $BUILD"

    echo
    echo cmake ../"$DYNINST_ROOT"
    echo "$@" | tr ' ' '\n'
    echo

    cmake  ../"$DYNINST_ROOT"  "$@"

    test $? -eq 0 || die "cmake failed"
else
    cd "$topdir" || die "unable to cd: $topdir"

    if cd "$BUILD" && test -f Makefile && test -d parseAPI
    then :
    else
	cat <<EOF
missing or invalid build directory: $BUILD
rerun with: ./mk-dyninst.sh -C
EOF
	exit 1
    fi
fi

#------------------------------------------------------------

if test "$do_make" != yes ; then
    exit 0
fi

echo
echo "===> make"

cd "$topdir" || die "unable to cd: $topdir"
cd "$BUILD" || die "unable to cd: $BUILD"

builddir=`/bin/pwd`

if test "$do_verbose" = yes ; then
    export VERBOSE=1
fi

for dir in $build_subdirs
do
    cd "$builddir" || die "unable to cd: $builddir"
    cd "$dir" || die "unable to cd: $dir"
    echo ; echo "===> make $dir"
    make $jobs
    test $? -eq 0 || die "make failed: $dir"
done

echo
echo "===> make install"

cd "$topdir" || die "unable to cd: $topdir"
rm -rf "$INSTALL"
mkdir "$INSTALL" || die "unable to mkdir: $INSTALL"

for dir in $build_subdirs
do
    cd "$builddir" || die "unable to cd: $builddir"
    cd "$dir" || die "unable to cd: $dir"
    echo ; echo "===> make install $dir"
    make install
    test $? -eq 0 || die "make install failed: $dir"
done

echo done

