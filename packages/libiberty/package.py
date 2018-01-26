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
import os.path
import shutil

class Libiberty(AutotoolsPackage):
    """The libiberty.a library from GNU binutils built static with
    -fPIC as a prerequisite for Dyninst for HPCToolkit."""

    homepage = "https://www.gnu.org/software/binutils/"
    url = "https://ftp.gnu.org/gnu/binutils/binutils-2.28.1.tar.bz2"

    version('2.28.1', '569a85c66421b16cfaa43b5f986db3bb')

    default_cflags = ['-g', '-O']

    # configure and build just libiberty subdir
    @property
    def configure_directory(self):
        return join_path(self.stage.source_path, 'libiberty')

    # add -fPIC to CFLAGS and move to the configure command line.
    # if original cflags is empty, then we must set the default or
    # else fpic is the only flag.
    def flag_handler(self, name, flags):
        if name != 'cflags': return (flags, None, None)

        if flags == []: flags = self.default_cflags
        flags.append(self.compiler.pic_flag)
        return (None, None, flags)

    def configure_args(self):
        args = ['--enable-install-libiberty']
        return args

    # copy libiberty.a to lib, libiberty puts it in lib64
    @run_after('install')
    def copy_library(self):
        lib_file = join_path(self.prefix.lib, 'libiberty.a')
        lib64_file = join_path(self.prefix.lib64, 'libiberty.a')

        if not os.path.isfile(lib_file):
            mkdirp(self.prefix.lib)
            shutil.copy(lib64_file, lib_file)
