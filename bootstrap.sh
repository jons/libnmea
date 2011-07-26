#!/bin/sh
#
if [ "$1" == "--force" ]; then
  libtoolize -f
  autoreconf -i -f
elif [ -f .bootstrapped ]; then
  autoreconf
else
  libtoolize
  autoreconf -i
fi
touch .bootstrapped
