#!/usr/bin/bash


for lib in $VENDORED_LIBS
do IFS=":"
  set -- $lib
  wget -nH --directory-prefix=$1 $2

done
