##############################################################################
#  Copyright (c) 2017, Rice University.
#  All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are
#  met:
#
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#
#  * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
#  * Neither the name of Rice University (RICE) nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
#  This software is provided by RICE and contributors "as is" and any
#  express or implied warranties, including, but not limited to, the
#  implied warranties of merchantability and fitness for a particular
#  purpose are disclaimed. In no event shall RICE or contributors be
#  liable for any direct, indirect, incidental, special, exemplary, or
#  consequential damages (including, but not limited to, procurement of
#  substitute goods or services; loss of use, data, or profits; or
#  business interruption) however caused and on any theory of liability,
#  whether in contract, strict liability, or tort (including negligence
#  or otherwise) arising in any way out of the use of this software, even
#  if advised of the possibility of such damage.
##############################################################################

from spack import *

class Libiberty(AutotoolsPackage):
    """The libiberty.a library from GNU binutils built static with
    -fPIC as a prereq for Dyninst for HPCToolkit."""

    homepage = "https://www.gnu.org/software/binutils/"
    url      = "https://ftp.gnu.org/gnu/binutils/binutils-2.29.1.tar.bz2"

    version('2.29.1', '9af59a2ca3488823e453bb356fe0f113')

    default_cflags = ['-g', '-O']

    # configure and build just libiberty
    @property
    def configure_directory(self):
        return join_path(self.stage.source_path, 'libiberty')

    # add -fPIC to CFLAGS, and "correct" empty CFLAGS to '-g -O'
    def cflags_handler(self, spack_env, flag_val):
        flag_name = flag_val[0].upper()
        flags = flag_val[1]
        if flags == []: flags = self.default_cflags

        spack_env.set(flag_name, ' '.join(flags))
        spack_env.append_flags(flag_name, self.compiler.pic_flag)
        return []

    def configure_args(self):
        args = ['--enable-install-libiberty']
        return args
