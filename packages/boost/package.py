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
import sys
import os

class Boost(Package):
    """Boost C++ libraries built for Rice HPCToolkit."""

    homepage = "http://www.boost.org"
    url = "http://downloads.sourceforge.net/project/boost/boost/1.65.1/boost_1_65_1.tar.bz2"
    list_url = "http://sourceforge.net/projects/boost/files/boost/"
    list_depth = 1

    version('1.65.1', '41d7542ce40e171f3f7982aff008ff0d')

    default_install_libs = set(['atomic',
                                'chrono',
                                'date_time',
                                'filesystem',
                                'system',
                                'thread'])

    default_noinstall_libs = set([])

    all_libs = default_install_libs | default_noinstall_libs

    for lib in all_libs:
        variant(lib, default=(lib not in default_noinstall_libs),
                description="Compile with {0} library".format(lib))

    variant('debug', default=False,
            description='Switch to the debug version of Boost')
    variant('shared', default=True,
            description="Additionally build shared libraries")
    variant('multithreaded', default=True,
            description="Build multi-threaded versions of libraries")
    variant('singlethreaded', default=False,
            description="Build single-threaded versions of libraries")
    variant('clanglibcpp', default=False,
            description='Compile with clang libc++ instead of libstdc++')

    def url_for_version(self, version):
        url = "http://downloads.sourceforge.net/project/boost/boost/{0}/boost_{1}.tar.bz2"
        return url.format(version.dotted, version.underscored)

    def determine_toolset(self, spec):
        if spec.satisfies("platform=darwin"):
            return 'darwin'

        toolsets = {'g++': 'gcc',
                    'icpc': 'intel',
                    'clang++': 'clang',
                    'xlc++': 'xlcpp',
                    'xlc++_r': 'xlcpp',
                    'pgc++': 'pgi'}

        if spec.satisfies('@1.47:'):
            toolsets['icpc'] += '-linux'
        for cc, toolset in toolsets.items():
            if cc in self.compiler.cxx_names:
                return toolset

        # fallback to gcc if no toolset found
        return 'gcc'

    def bjam_python_line(self, spec):
        return 'using python : {0} : {1} : {2} : {3} ;\n'.format(
            spec['python'].version.up_to(2),
            spec['python'].command.path,
            spec['python'].headers.directories[0],
            spec['python'].libs[0]
        )

    def determine_bootstrap_options(self, spec, withLibs, options):
        boostToolsetId = self.determine_toolset(spec)
        options.append('--with-toolset=%s' % boostToolsetId)
        options.append("--with-libraries=%s" % ','.join(withLibs))

        with open('user-config.jam', 'w') as f:
            # Boost may end up using gcc even though clang+gfortran is set in
            # compilers.yaml. Make sure this does not happen:
            if not spec.satisfies('%intel'):
                # using intel-linux : : spack_cxx in user-config.jam leads to
                # error: at project-config.jam:12
                # error: duplicate initialization of intel-linux with the following parameters:  # noqa
                # error: version = <unspecified>
                # error: previous initialization at ./user-config.jam:1
                f.write("using {0} : : {1} ;\n".format(boostToolsetId,
                                                       spack_cxx))

    def determine_b2_options(self, spec, options):
        if '+debug' in spec:
            options.append('variant=debug')
        else:
            options.append('variant=release')

        linkTypes = ['static']
        if '+shared' in spec:
            linkTypes.append('shared')

        threadingOpts = []
        if '+multithreaded' in spec:
            threadingOpts.append('multi')
        if '+singlethreaded' in spec:
            threadingOpts.append('single')
        if not threadingOpts:
            raise RuntimeError("At least one of {singlethreaded, " +
                               "multithreaded} must be enabled")

        if len(threadingOpts) > 1:
            raise RuntimeError("Cannot build both single and " +
                               "multi-threaded targets with system layout")
        layout = 'system'

        options.extend([
            'link=%s' % ','.join(linkTypes),
            '--layout=%s' % layout
        ])

        # the equivalent of -g
        options.append('debug-symbols=on')

        if not spec.satisfies('%intel'):
            options.extend([
                'toolset=%s' % self.determine_toolset(spec)
            ])

        # clang is not officially supported for pre-compiled headers
        # and at least in clang 3.9 still fails to build
        #   http://www.boost.org/build/doc/html/bbv2/reference/precompiled_headers.html
        #   https://svn.boost.org/trac/boost/ticket/12496
        if spec.satisfies('%clang'):
            options.extend(['pch=off'])
            if '+clanglibcpp' in spec:
                options.extend(['toolset=clang',
                                'cxxflags="-stdlib=libc++"',
                                'linkflags="-stdlib=libc++"'])

        return threadingOpts

    def add_buildopt_symlinks(self, prefix):
        with working_dir(prefix.lib):
            for lib in os.listdir(os.curdir):
                prefix, remainder = lib.split('.', 1)
                symlink(lib, '%s-mt.%s' % (prefix, remainder))

    def install(self, spec, prefix):
        # On Darwin, Boost expects the Darwin libtool. However, one of the
        # dependencies may have pulled in Spack's GNU libtool, and these two
        # are not compatible. We thus create a symlink to Darwin's libtool
        # and add it at the beginning of PATH.
        if sys.platform == 'darwin':
            newdir = os.path.abspath('darwin-libtool')
            mkdirp(newdir)
            force_symlink('/usr/bin/libtool', join_path(newdir, 'libtool'))
            env['PATH'] = newdir + ':' + env['PATH']

        withLibs = list()
        for lib in Boost.all_libs:
            if "+{0}".format(lib) in spec:
                withLibs.append(lib)
        if not withLibs:
            # if no libraries are specified for compilation, then you dont have
            # to configure/build anything, just copy over to the prefix
            # directory.
            src = join_path(self.stage.source_path, 'boost')
            mkdirp(join_path(prefix, 'include'))
            dst = join_path(prefix, 'include', 'boost')
            install_tree(src, dst)
            return

        # to make Boost find the user-config.jam
        env['BOOST_BUILD_PATH'] = './'

        bootstrap = Executable('./bootstrap.sh')

        bootstrap_options = ['--prefix=%s' % prefix]
        self.determine_bootstrap_options(spec, withLibs, bootstrap_options)

        bootstrap(*bootstrap_options)

        # b2 used to be called bjam, before 1.47 (sigh)
        b2name = './b2' if spec.satisfies('@1.47:') else './bjam'

        b2 = Executable(b2name)
        jobs = make_jobs
        # in 1.59 max jobs became dynamic
        if jobs > 64 and spec.satisfies('@:1.58'):
            jobs = 64
        b2_options = ['-j', '%s' % jobs]

        threadingOpts = self.determine_b2_options(spec, b2_options)

        b2('--clean')

        # In theory it could be done on one call but it fails on
        # Boost.MPI if the threading options are not separated.
        for threadingOpt in threadingOpts:
            b2('install', 'threading=%s' % threadingOpt, *b2_options)

        if '+multithreaded' in spec and '~taggedlayout' in spec:
            self.add_buildopt_symlinks(prefix)

        # The shared libraries are not installed correctly
        # on Darwin; correct this
        if (sys.platform == 'darwin') and ('+shared' in spec):
            fix_darwin_install_name(prefix.lib)
