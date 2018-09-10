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

# Todo:
#
#  1. add papi
#
#  2. add mpi
#
#  3. check for and set compiler rev
#
#  4. intel-xed only on x86
#

from spack import *


class Hpctoolkit(AutotoolsPackage):
    """HPCToolkit is an integrated suite of tools for measurement and analysis
    of program performance on computers ranging from multicore desktop systems
    to the nation's largest supercomputers. By using statistical sampling of
    timers and hardware performance counters, HPCToolkit collects accurate
    measurements of a program's work, resource consumption, and inefficiency
    and attributes them to the full calling context in which they occur."""

    homepage = "http://hpctoolkit.org"
    git      = "https://github.com/mwkrentel/hpctoolkit.git"

    version('config', branch='config')

    depends_on('binutils+libiberty')
    depends_on('boost')
    depends_on('bzip2')
    depends_on('dyninst')
    depends_on('elfutils')
    depends_on('intel-tbb')
    depends_on('intel-xed')
    depends_on('libdwarf')
    depends_on('libmonitor+hpctoolkit')
    depends_on('libpfm4')
    depends_on('libunwind')
    depends_on('xerces-c')
    depends_on('xz')
    depends_on('zlib')

    def configure_args(self):
        spec = self.spec

        args = [
            '--with-binutils=%s'     % spec['binutils'].prefix,
            '--with-boost=%s'        % spec['boost'].prefix,
            '--with-bzip=%s'         % spec['bzip2'].prefix,
            '--with-symtabAPI=%s'    % spec['dyninst'].prefix,
            '--with-libelf=%s'       % spec['elfutils'].prefix,
            '--with-tbb=%s'          % spec['intel-tbb'].prefix,
            '--with-xed2=%s'         % spec['intel-xed'].prefix,
            '--with-libdwarf=%s'     % spec['libdwarf'].prefix,
            '--with-libmonitor=%s'   % spec['libmonitor'].prefix,
            '--with-perfmon=%s'      % spec['libpfm4'].prefix,
            '--with-libunwind=%s'    % spec['libunwind'].prefix,
            '--with-xerces=%s'       % spec['xerces-c'].prefix,
            '--with-lzma=%s'         % spec['xz'].prefix,
            '--with-zlib=%s'         % spec['zlib'].prefix,
            ]

        return args
