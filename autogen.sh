#!/bin/sh
set -e
set -x

SOURCE_DIR=`dirname $0`

cd $SOURCE_DIR

autopoint --force
intltoolize --automake --copy --force
libtoolize --automake --copy
aclocal -I m4
autoheader
automake --add-missing --copy
autoconf

cd -

export CFLAGS="-g -O0"
export CXXFLAGS="$CFLAGS"
$SOURCE_DIR/configure --enable-maintainer-mode $*
