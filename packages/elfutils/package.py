##############################################################################
# Copyright (c) 2013-2017, Lawrence Livermore National Security, LLC.
# Produced at the Lawrence Livermore National Laboratory.
#
# This file is part of Spack.
# Created by Todd Gamblin, tgamblin@llnl.gov, All rights reserved.
# LLNL-CODE-647188
#
# For details, see https://github.com/llnl/spack
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

class Elfutils(AutotoolsPackage):
    """Elfutils built for Rice HPCToolkit."""

    homepage = "https://fedorahosted.org/elfutils/"

    url = "https://sourceware.org/elfutils/ftp/0.170/elfutils-0.170.tar.bz2"
    list_url = "https://sourceware.org/elfutils/ftp"
    list_depth = 1

    version('0.170', '03599aee98c9b726c7a732a2dd0245d5')

    depends_on('bzip2', type='link')
    depends_on('xz', type='link')
    depends_on('zlib', type='link')

    default_cflags = ['-g', '-O2']

    # set default cflags (if not specified), always add -g, and
    # disable -Werror (too dangerous).
    def flag_handler(self, name, flags):
        if name != 'cflags': return (flags, None, None)

        if flags == []: flags = self.default_cflags
        if '-g' not in flags: flags.insert(0, '-g')
        flags.append('-Wno-error')
        return (None, None, flags)

    # disable maintainer mode, so we don't need flex or bison
    def configure_args(self):
        return [ '--disable-maintainer-mode' ]

    # install elf.h to include directory
    @run_after('install')
    def install_elf_h(self):
        install(join_path(self.stage.source_path, 'libelf', 'elf.h'),
                self.prefix.include)
