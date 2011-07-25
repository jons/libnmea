#!/bin/sh
#
set -e
if [ "$1" == "--force" ]; then
  rm -f aclocal.m4
  libtoolize -c -f
  autoreconf -i -f
elif [ -f .bootstrapped ]; then
  autoreconf
else
  libtoolize -c
  aclocal
  autoreconf -i
fi
touch .bootstrapped
