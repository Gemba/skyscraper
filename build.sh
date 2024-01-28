#!/bin/bash

### Bash script to automate full clean build of Skyscraper 

# Cleanup
rm .qmake.stash
make clean

# Make/build and install
qmake
make -j$(nproc)
sudo make install

