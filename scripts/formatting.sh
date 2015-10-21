#!/bin/bash
# This script runs clang-format on some source file.
# The goal is to progressively convert the code base to a unified formatting.
# When a file goes through many modification, it may be worth to profit and
# fix its formatting.

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

clang-format-3.8 -style=file -i ${DIR}/../include/logicalaccess/cards/IKSStorage.hpp;
find ${DIR}/../include/logicalaccess/iks -name "*.hpp"  | xargs clang-format-3.8 -style=file -i {} \;
find ${DIR}/../src/iks -name "*.cpp"  | xargs clang-format-3.8 -style=file -i {} \;



find ${DIR}/../tests/other/ -name "*.cpp"  | xargs clang-format-3.8 -style=file -i {} \;
clang-format-3.8 -style=file -i ${DIR}/../tests/pcsc-tests/DESFireEV1/test_changekey_iks.cpp
