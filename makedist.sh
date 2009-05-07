#!/bin/sh

# this script is intended for developers/packagers only!

# this makes a distribution tarball that should appear
# in dist/$package-$version.tar.gz

distdir=dist

make distclean
set -e

./autogen.sh
./configure
make
make dist

