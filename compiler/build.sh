#!/bin/bash
set -xe
make -C ../embedvm/tools
ln -sf ../embedvm/vmsrc/embedvm.c embedvm.c
ln -sf ../embedvm/vmsrc/embedvm.h embedvm.h
#gcc -o localversion localversion.c embedvm.c
./arduino-cc.sh -o firmware hostapp.cc  embedvm.c