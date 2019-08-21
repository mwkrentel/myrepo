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
#    ./setup.sh  /path/to/spack/tools/dir  [ /path/to/spack/app/dir ]
#
#  Tools dir is for gotcha (GNU)
#  App dir is for gperftools and libunwind
#

die() {
    echo "error: $@"
    exit 1
}

TOOLS_DIR="$1"
APP_DIR="$2"

make_incl=makefile.incl

topdir=`/bin/pwd`

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

test "x$TOOLS_DIR" != x || die "missing spack install directory"

cd "$TOOLS_DIR" || die "unable to cd: $TOOLS_DIR"
ABS_SPACK=`/bin/pwd`

cd "$topdir" || die "unable to cd: $topdir"

GOTCHA=`spackdir gotcha`

if test "x$APP_DIR" != x ; then
    cd "$APP_DIR" || die "unable to cd: $APP_DIR"
    ABS_SPACK=`/bin/pwd`
fi

cd "$topdir" || die "unable to cd: $topdir"

TCMALLOC=`spackdir gperftools`
UNWIND=`spackdir libunwind`

#------------------------------------------------------------

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

