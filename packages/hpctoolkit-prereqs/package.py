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
import platform

class HpctoolkitPrereqs(Package):
    """Meta package to install prerequisites for Rice HPCToolkit,
    the spack version of hpctoolkit externals."""

    # The url and version are fake until spack implements class
    # MetaPackage.

    homepage = "http://hpctoolkit.org/"
    url = "https://github.com/hpctoolkit/libmonitor"

    version('master', branch = 'master',
            git = 'https://github.com/hpctoolkit/libmonitor')

    pkg_list = ['binutils', 'boost', 'dyninst', 'elfutils',
                'intel-tbb', 'libdwarf', 'libiberty', 'libmonitor',
                'libunwind', 'xerces-c']

    # Fixme: machine() is the build type, what we really want is the
    # host (target) type from the spec.
    if platform.machine() == 'x86_64':
        pkg_list.append('intel-xed')

    for pkg in pkg_list:
        depends_on(pkg, type='run')

    # Write the list of prereq packages and their install prefixes to
    # prefix.etc.  Spack requires that we must install something.
    def install(self, spec, prefix):
        etc = join_path(prefix, 'etc')
        mkdirp(etc)

        f = open(join_path(etc, 'prereqs.txt'), 'w')
        for pkg in self.pkg_list:
            f.write("%s: %s\n" % (pkg, spec[pkg].prefix))
        f.close()
