##############################################################################
# Copyright (c) 2013-2017, Lawrence Livermore National Security, LLC.
# Produced at the Lawrence Livermore National Laboratory.
#
# This file is part of Spack.
# Created by Todd Gamblin, tgamblin@llnl.gov, All rights reserved.
# LLNL-CODE-647188
#
# For details, see https://github.com/spack/spack
# Please also see the NOTICE and LICENSE files for our notice and the LGPL.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License (as
# published by the Free Software Foundation) version 2.1, February 1999.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the IMPLIED WARRANTY OF
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the terms and
# conditions of the GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
##############################################################################

# Modified for Rice HPCToolkit.

from spack import *

class Libdwarf(Package):
    """Libdwarf built for Rice HPCToolkit.  This version only builds
    libdwarf, not dwarfdump."""

    homepage = "http://www.prevanders.net/dwarf.html"
    url = "https://www.prevanders.net/libdwarf-20170709.tar.gz"

    version('20170709', '68a3c9aa7d01a433924a74bda588b378')

    depends_on('elfutils', type='link')
    depends_on('zlib', type='link')

    patch('make.patch')
    patch('typedef.patch')

    parallel=False

    # fixme: read cflags from spec and pass to configure
    def install(self, spec, prefix):

        mkdirp(prefix.include, prefix.lib)

        opts = ['CPPFLAGS=-I%s -I%s' % (spec['elfutils'].prefix.include,
                                        spec['zlib'].prefix.include),
                'LDFLAGS=-L%s -L%s'  % (spec['elfutils'].prefix.lib,
                                        spec['zlib'].prefix.lib)]

        # the spack wrappers put the elf include dir ahead of our
        # patched makefile, so we have to hide elf's dwarf.h.
        dwarf_h = join_path(spec['elfutils'].prefix, 'include/dwarf.h')

        with hide_files(dwarf_h):

            with working_dir('libdwarf'):
                #
                # this version builds both a shared library and a
                # static library with -fPIC.  the --disable-fpic
                # option is broken, so a static library without -fPIC
                # requires hacking the configure script.
                #
                configure('--enable-shared', *opts)
                make()

                install('dwarf.h', prefix.include)
                install('libdwarf.h', prefix.include)
                install('libdwarf.a', prefix.lib)
                install('libdwarf.so', prefix.lib)
