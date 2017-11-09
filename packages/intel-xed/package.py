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
import glob

class IntelXed(Package):
    """The Intel X86 Instruction Encoder Decoder library built for Rice
    HPCToolkit.  This version only builds the decoder library."""

    homepage = "https://intelxed.github.io/"
    url = "https://github.com/intelxed/xed"

    version('2017.11.07',
            git = 'https://github.com/intelxed/xed',
            commit = '0f857b386f1885c4')

    resource(name = 'mbuild',
             git = 'https://github.com/intelxed/mbuild',
             commit = '9eefb36a01167e56',
             destination = '')

    # fixme: read cflags from spec and translate to mfile syntax.
    # this version always uses: -g -O2
    def install(self, spec, prefix):
        mfile = Executable('./mfile.py')

        # build and install static libxed.a
        mfile('--clean')
        mfile('-j', str(make_jobs),
              '--debug',
              '--opt=2',
              '--no-encoder',
              '--no-werror')

        mkdirp(prefix.lib)

        libs = glob.glob(join_path('obj', 'lib*.a'))
        for lib in libs:
            install(lib, prefix.lib)

        # build and install shared libxed.so
        mfile('--clean')
        mfile('-j', str(make_jobs),
              '--debug',
              '--opt=2',
              '--shared',
              '--no-encoder',
              '--no-werror')

        libs = glob.glob(join_path('obj', 'lib*.so'))
        for lib in libs:
            install(lib, prefix.lib)

        # install header files
        install_tree(join_path('include', 'public'), prefix.include)
