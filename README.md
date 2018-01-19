# myrepo

This repository is (will be) a satellite spack repository for building
HPCToolkit prerequisite packages.  This is for building the 3rd-party
packages with the quirks and special contraints needed for HPCToolkit.
This repo, when finished, will replace hpctoolkit-externals and be
moved to the HPCToolkit project on github.

Spack is copyright Lawrence Livermore National Security, LLC, and is
available as a github project at:

https://github.com/spack

Mark W. Krentel  
October 2017

--------------------

To use this repository to build the HPCToolkit prerequisite packages,
clone both this repository and the main LLNL spack repository.

```
git clone https://github.com/mwkrentel/myrepo
git clone https://github.com/spack/spack
```

Run the `./setup.sh` script in myrepo to link the myrepo satellite
repository to the main spack repository.  This script writes two files
in the main spack tree: `config.yaml` sets the location of the install
prefix, and `repos.yaml` sets the location of the satellite
repository.

This script takes two arguments: the path to the spack repository and
optionally, the path to an install prefix.  If you don't specify an
install prefix, then the default spack install directory is
`<spack-root>/opt/spack/`.

If successful, the `setup.sh` script displays the list of known
repositories and the install prefix.  This should include myrepo first
and then the main spack repository.  For example:

```
cd myrepo
./setup spack-root [prefix]

==> 2 package repositories.
hpctoolkit    /home/krentel/myrepo
builtin       /home/krentel/spack-root/var/spack/repos/builtin

install prefix: /home/krentel/prefix
```

Source the `env.sh` script (for bash) to add the spack bin directory
to your PATH, or else run the spack command with a full pathname.

Finally, run `spack install package ...` to install packages.

Note: this is a work in progress and only a few packages are currently
available.

