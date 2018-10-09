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
#  3. check for and set compiler rev
#  6. anything special for blue gene or cray
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

    # We can't build with both PAPI and perfmon for risk of segfault
    # from mismatched header files (unless PAPI installs the perfmon
    # headers).
    variant('papi', default=False,
            description='Use PAPI instead of perfmon for access to '
            'the hardware performance counters.')

    # We always support profiling MPI applications.  +mpi builds
    # hpcprof-mpi, the MPI version of hpcprof.
    variant('mpi', default=False,
            description='Build hpcprof-mpi, the MPI version of hpcprof.')

    variant('all-static', default=False,
            description='Needed when MPICXX builds static binaries '
            'for the compute nodes.')

    depends_on('binutils+libiberty~nls')
    depends_on('boost')
    depends_on('bzip2')
    depends_on('dyninst')
    depends_on('elfutils~nls')
    depends_on('intel-tbb')
    depends_on('libdwarf')
    depends_on('libmonitor+hpctoolkit')
    depends_on('libunwind@1.3-rc1')
    depends_on('xerces-c transcoder=iconv')
    depends_on('xz')
    depends_on('zlib')

    depends_on('intel-xed', when='target=x86_64')
    depends_on('papi', when='+papi')
    depends_on('libpfm4', when='~papi')
    depends_on('mpi', when='+mpi')

    flag_handler = AutotoolsPackage.build_system_flags

    def configure_args(self):
        spec = self.spec
        target = spec.architecture.target

        args = [
            '--with-binutils=%s'     % spec['binutils'].prefix,
            '--with-boost=%s'        % spec['boost'].prefix,
            '--with-bzip=%s'         % spec['bzip2'].prefix,
            '--with-dyninst=%s'      % spec['dyninst'].prefix,
            '--with-elfutils=%s'     % spec['elfutils'].prefix,
            '--with-tbb=%s'          % spec['intel-tbb'].prefix,
            '--with-libdwarf=%s'     % spec['libdwarf'].prefix,
            '--with-libmonitor=%s'   % spec['libmonitor'].prefix,
            '--with-libunwind=%s'    % spec['libunwind'].prefix,
            '--with-xerces=%s'       % spec['xerces-c'].prefix,
            '--with-lzma=%s'         % spec['xz'].prefix,
            '--with-zlib=%s'         % spec['zlib'].prefix,
            ]

        if target == 'x86_64':
            args.append('--with-xed=%s' % spec['intel-xed'].prefix)

        if '+papi' in spec:
            args.append('--with-papi=%s' % spec['papi'].prefix)
        else:
            args.append('--with-perfmon=%s' % spec['libpfm4'].prefix)

        if '+mpi' in spec:
            args.append('MPICXX=%s' % spec['mpi'].mpicxx)

        if '+all-static' in spec:
            args.append('--enable-all-static')

        return args
