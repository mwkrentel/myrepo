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
#
# Version 2.29 restructured some header files (print_insn_i386 in
# dis-asm.h).  So, we're stuck on rev 2.28 until toolkit adjusts to
# the change.

from spack import *
import os.path
import shutil

class Binutils(AutotoolsPackage):
    """GNU binutils built for Rice HPCToolkit."""

    homepage = "https://www.gnu.org/software/binutils/"
    url = "https://ftp.gnu.org/gnu/binutils/binutils-2.28.1.tar.bz2"

    version('2.28.1', '569a85c66421b16cfaa43b5f986db3bb')

    depends_on('zlib', type='run')

    patch('basename.patch')
    patch('config.patch')

    default_cflags = ['-g', '-O2']

    # set default cflags and move to configure command line
    def flag_handler(self, name, flags):
        if name != 'cflags': return (flags, None, None)

        if flags == []: flags = self.default_cflags
        return (None, None, flags)

    def configure_args(self):
        args = ['--enable-targets=all',
                '--enable-install-libiberty',
                '--disable-werror']
        return args

    # this is temporary, to make the spack install look more like
    # externals's install, until we rework toolkit configure
    @run_after('install')
    def copy_libiberty_zlib(self):
        prefix = self.prefix
        zlib = self.spec['zlib'].prefix

        if not os.path.isfile(join_path(prefix.include, 'libiberty.h')):
            shutil.copy(join_path(prefix.include, 'libiberty', 'demangle.h'),
                        prefix.include)
            shutil.copy(join_path(prefix.include, 'libiberty', 'libiberty.h'),
                        prefix.include)

        if not os.path.isfile(join_path(prefix.lib, 'libiberty.a')):
            shutil.copy(join_path(prefix.lib64, 'libiberty.a'), prefix.lib)

        if not os.path.isfile(join_path(prefix.lib, 'libz.a')):
            shutil.copy(join_path(zlib.lib, 'libz.a'), prefix.lib)
