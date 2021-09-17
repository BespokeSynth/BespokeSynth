#!/usr/bin/env bash

if [ ! -f Makefile ]; then
  
  echo "No Makefile found. Please download Projucer and choose 'Save this exporter' for Linux Makefile."

else

make CONFIG=Release -j`nproc`

fi
