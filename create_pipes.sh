#!/bin/sh

in=/tmp/prga-hw08.in
out=/tmp/prga-hw08.out

rm -rf $in $out
mkfifo $in
mkfifo $out
