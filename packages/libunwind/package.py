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
import os.path

class Libunwind(AutotoolsPackage):
    """A library for unwinding the call stack of a running program,
    built for Rice HPCToolkit."""

    homepage = "http://www.nongnu.org/libunwind/"
    url = "https://github.com/libunwind/libunwind"

    version('2018.01.17', commit = '7d6cc6696ab8a808da3d',
            git = 'https://github.com/libunwind/libunwind')

    depends_on('xz', type='link')

    default_cflags = ['-g', '-O2']

    # run autoreconf ourself, so we don't have to rebuild all its
    # prereqs (perl).
    def autoreconf(self, spec, prefix):
        reconf = Executable('autoreconf')
        reconf('-f', '-i')

        if not os.path.exists(self.configure_abs_path):
            msg = 'configure script not found in {0}'
            raise RuntimeError(msg.format(self.configure_directory))

    # set default cflags and move to configure command line
    def flag_handler(self, name, flags):
        if name != 'cflags': return (flags, None, None)

        if flags == []: flags = self.default_cflags
        return (None, None, flags)

    def configure_args(self):
        args = ['--enable-shared',
                '--enable-static',
                '--enable-minidebuginfo',
                '--disable-coredump',
                '--disable-ptrace',
                '--disable-setjmp',
                '--disable-tests']
        return args
