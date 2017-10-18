#!/bin/bash
# This script runs clang-format on some source file.
# The goal is to progressively convert the code base to a unified formatting.
# When a file goes through many modification, it may be worth to profit and
# fix its formatting.

find ./include/logicalaccess ./src ./plugins ./tests ./samples ./vc++ \( -name '*.cpp' -or -name '*.hpp' \) | xargs clang-format-3.9 -style=file -i
