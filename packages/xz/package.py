##############################################################################
# Copyright (c) 2013-2018, Lawrence Livermore National Security, LLC.
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

class Xz(AutotoolsPackage):
    """The XZ (lzma) compression library built for Rice HPCToolkit."""

    homepage = "http://tukaani.org/xz/"
    url      = "http://tukaani.org/xz/xz-5.2.3.tar.bz2"
    list_url = "http://tukaani.org/xz/old.html"

    version('5.2.3', '1592e7ca3eece099b03b35f4d9179e7c')

    # set default cflags (-g -O2) and move to configure line
    def flag_handler(self, name, flags):
        if name != 'cflags': return (flags, None, None)

        if '-g' not in flags: flags.append('-g')
        for flag in flags:
            if flag[0:2] == '-O': break
        else:
            flags.append('-O2')

        return (None, None, flags)

    def configure_args(self):
        args = ['--enable-shared',
                '--enable-static',
                '--enable-threads=yes']
        return args
