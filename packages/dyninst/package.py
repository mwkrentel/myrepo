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
#
# Dyninst builds with cmake, but we avoid CMakePackage because that
# forces always rebuilding cmake (won't accept cmake on path).
#
# Also, we only build a subset of libraries: symtab, parseapi and
# prereqs.

from spack import *

class Dyninst(Package):
    """Dyninst instrumentation library built for Rice HPCToolkit.
    This version builds SymtabAPI, ParseAPI and their prerequisites
    for control-flow analysis."""

    homepage = "https://dyninst.org"
    url = "https://github.com/dyninst/dyninst"

    version('master', git='https://github.com/dyninst/dyninst.git',
            branch='master', preferred=True)

    version('johnmc', git='https://github.com/jmellorcrummey/dyninst',
            branch='new-parallel-parsing')

    version('parallel', git='https://github.com/dyninst/dyninst.git',
            branch='new-parallel-parsing')

    variant('debug', default=False, description='Use Cmake build type Debug')
    variant('openmp', default=False, description='Enable OpenMP support')

    depends_on('boost', type='link')
    depends_on('elfutils', type='link')
    depends_on('libiberty', type='link')
    depends_on('intel-tbb', type='link', when='@johnmc')
    depends_on('intel-tbb', type='link', when='@parallel')

    def patch(self):
        # Disable cotire, it breaks parallel builds when building both
        # shared and static libraries.
        filter_file(r'^(.*)[sS][eE][tT].*USE_COTIRE.*', r'\1set(USE_COTIRE false)',
                    join_path('cmake', 'shared.cmake'))

        # Disable testsuite, or else we have to fetch a git submodule.
        filter_file(r'^.*add_subdirectory.*testsuite.*', '# disable testsuite',
                    'CMakeLists.txt')

    # Move cxxflags to compiler wrapper (default).
    flag_handler = Package.inject_flags

    def install(self, spec, prefix):
        boost_root = spec['boost'].prefix
        elf_incl = spec['elfutils'].prefix.include
        elf_lib = join_path(spec['elfutils'].prefix.lib, 'libelf.so')
        dwarf_lib = join_path(spec['elfutils'].prefix.lib, 'libdw.so')
        libiberty_lib = join_path(spec['libiberty'].prefix.lib, 'libiberty.a')

        build_subdirs = ['common', 'elf', 'dwarf', 'symtabAPI',
                         'instructionAPI', 'parseAPI']

        build_type = 'RelWithDebInfo'
        if '+debug' in spec:
            build_type = 'Debug'

        args = ['.',
                '-DCMAKE_INSTALL_PREFIX=%s' % prefix,
                '-DCMAKE_BUILD_TYPE=%s' % build_type,
                '-DCMAKE_C_COMPILER=%s' % spack_cc,
                '-DCMAKE_CXX_COMPILER=%s' % spack_cxx,
                '-DUSE_COTIRE=False',
                '-DBUILD_DOCS=Off',
                '-DBUILD_RTLIB=Off',
                '-DBUILD_TARBALLS=Off',
                '-DPATH_BOOST=%s' % boost_root,
                '-DLIBELF_INCLUDE_DIR=%s' % elf_incl,
                '-DLIBELF_LIBRARIES=%s' % elf_lib,
                '-DLIBDWARF_INCLUDE_DIR=%s' % elf_incl,
                '-DLIBDWARF_LIBRARIES=%s' % dwarf_lib,
                '-DIBERTY_LIBRARIES=%s' % libiberty_lib]

        if spec.version == Version('johnmc') or spec.version == Version('parallel'):
            tbb = spec['intel-tbb'].prefix
            args.extend(['-DTBB_ROOT_DIR=%s' % tbb,
                         '-DTBB_INCLUDE_DIR=%s' % tbb.include,
                         '-DTBB_LIBRARY=%s' % tbb.lib])

        if '+openmp' in spec:
            args.extend(['-DCMAKE_C_FLAGS=%s' % self.compiler.openmp_flag,
                         '-DCMAKE_CXX_FLAGS=%s' % self.compiler.openmp_flag])

        cmake(*args)

        # build only symtab, parseapi and prereqs
        top_src = self.stage.source_path
        for dir in build_subdirs:
            with working_dir(join_path(top_src, dir)):
                make()

        for dir in build_subdirs:
            with working_dir(join_path(top_src, dir)):
                make('install')
