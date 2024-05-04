#!/bin/bash

set echo on

# Get a list of all the .c files.
cFilenames=$(find . -type f -name "*.c")

hFilenames=$(find . -type f -name "*.h")

clang-format $cFilenames $hFilenames -i
