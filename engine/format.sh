#!/bin/bash

# Get a list of all the .c and .cpp files.
cFilenames=$(find . -type f -name "*.c*")

hFilenames=$(find . -type f -name "*.h")

clang-format $cFilenames $hFilenames -i
