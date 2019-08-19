#!/bin/sh
#
#  Search a spack install directory to find paths to gotcha, tcmalloc
#  and libunwind.
#
#  Copyright (c) 2019, Rice University.
#  See the file LICENSE for details.
#
#  Mark W. Krentel
#  August 2019
#
#  Usage:
#    ./setup.sh  path/to/spack/linux-redhat-x86/gcc-x.y.z
#

die() {
    echo "error: $@"
    exit 1
}

SPACK_INSTALL="$1"

make_incl=makefile.incl

topdir=`/bin/pwd`

#------------------------------------------------------------

test "x$SPACK_INSTALL" != x || die "missing spack install directory"

cd "$SPACK_INSTALL" || die "unable to cd: $SPACK_INSTALL"

ABS_SPACK=`/bin/pwd`

cd "$topdir" || die "unable to cd: $topdir"

#------------------------------------------------------------

#  Returns: full path of subdir of SPACK_INSTALL whose name begins
#  with 'base', the most recent one if there are multiple subdirs.
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
        die "unable to find: $base"
    fi

    echo $ans
}

#------------------------------------------------------------

GOTCHA=`spackdir gotcha`
TCMALLOC=`spackdir gperftools`
UNWIND=`spackdir libunwind`

rm -f "$make_incl"

cat >"$make_incl" <<EOF
#
#  Makefile include paths
#

TOPDIR =   $topdir
GOTCHA =   $GOTCHA
TCMALLOC = $TCMALLOC
UNWIND =   $UNWIND

EOF

cat "$make_incl"

#------------------------------------------------------------

name=run-gotcha

rm -f "$name"

cat >"$name" <<EOF
#!/bin/sh

LD_LIBRARY_PATH="${GOTCHA}/lib64/:\$LD_LIBRARY_PATH"

export LD_PRELOAD='${topdir}/libmygotcha.so'

exec "\$@"
EOF
chmod 755 "$name"

#----------

name=run-preload

rm -f "$name"

cat >"$name" <<EOF
#!/bin/sh

export LD_PRELOAD='${topdir}/libpreload.so'

exec "\$@"
EOF
chmod 755 "$name"

#----------

name=run-hybrid

rm -f "$name"

cat >"$name" <<EOF
#!/bin/sh

LD_LIBRARY_PATH="${GOTCHA}/lib64/:\$LD_LIBRARY_PATH"

export LD_PRELOAD='${topdir}/libhybrid.so'

exec "\$@"
EOF
chmod 755 "$name"

