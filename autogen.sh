#!/bin/sh
set -e 

# This script is for developers and package maintainers only!  End users should
# never have to run this script when compiling unless they modify the source by
# hand (in which case you're one of the cool kids :)
#
# this requires aclocal/automake 1.6 or higher
# autoconf 2.5x should work

panic () {
	echo "$@"
	exit 1
}

aclocal || panic "aclocal (included with automake) version 1.6 or higher is required"
autoconf --force
autoheader --force
automake --foreign --include-deps --add-missing --copy --force-missing || panic "automake version 1.6 or higher is required"

echo ""
echo "All the autotools stuff seems to have passed alright."
echo "Run the standard  ./configure && make  to see if it compiles alright"

