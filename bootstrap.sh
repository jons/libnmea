#!/bin/sh
#
if [ -f .bootstrapped ]; then
  autoreconf
else
  libtoolize
  autoreconf -i
  touch .bootstrapped
fi
