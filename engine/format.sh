#!/bin/bash

# Get a list of all the .c and .cpp files.
filenames=$(find . -type f \( -name "*.c*" -o -name "*.cpp" -o -name ".h" -o -name ".inl" \))


clang-format $filenames -i
