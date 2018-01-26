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

class Libmonitor(AutotoolsPackage):
    """Libmonitor process control library built for Rice HPCToolkit."""

    homepage = "https://github.com/hpctoolkit/libmonitor"
    url = "https://github.com/hpctoolkit/libmonitor"

    version('2017.12.06', commit = '6be9bc85ff756198b9c7',
            git = 'https://github.com/hpctoolkit/libmonitor')

    default_cflags = ['-g', '-O2']

    signals = 'SIGBUS, SIGSEGV, SIGPROF, 36, 37, 38'

    # fixme: need to use CC = powerpc64-bgq-linux-gcc for blue gene/q
    # back end.

    # set default cflags and move to configure command line
    def flag_handler(self, name, flags):
        if name != 'cflags': return (flags, None, None)

        if flags == []: flags = self.default_cflags
        return (None, None, flags)

    def configure_args(self):
        args = [ '--enable-client-signals=%s' % self.signals ]
        return args
