#!/bin/bash
# This script runs clang-format on some source file.
# The goal is to progressively convert the code base to a unified formatting.
# When a file goes through many modification, it may be worth to profit and
# fix its formatting.

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

clang-format-3.8 -style=file -i ${DIR}/../include/logicalaccess/cards/IKSStorage.hpp
find ${DIR}/../include/logicalaccess/iks -name "*.hpp"  | xargs clang-format-3.8 -style=file -i {} \;

find ${DIR}/../src/iks -name "*.cpp"  | xargs clang-format-3.8 -style=file -i {} \;


find ${DIR}/../tests/other/ -name "*.cpp"  | xargs clang-format-3.8 -style=file -i {} \;
clang-format-3.8 -style=file -i ${DIR}/../tests/pcsc-tests/DESFireEV1/test_changekey_iks.cpp

clang-format-3.8 -style=file -i ${DIR}/../src/cards/keystorage.cpp

clang-format-3.8 -style=file -i ${DIR}/../plugins/pluginsreaderproviders/pcsc/atrparser.cpp ${DIR}/../plugins/pluginsreaderproviders/pcsc/atrparser.hpp ${DIR}/../plugins/pluginsreaderproviders/pcsc/pcscfeatures.{cpp,hpp}

find ${DIR}/../tests/unittest -name "*.cpp"  | xargs clang-format-3.8 -style=file -i {} \;
clang-format-3.8 -style=file -i ${DIR}/../src/utils.cpp ${DIR}/../include/logicalaccess/utils.hpp

clang-format-3.8 -style=file -i ${DIR}/../src/services/uidchanger/uidchangerservice.cpp ${DIR}/../include/logicalaccess/services/uidchanger/uidchangerservice.hpp

clang-format-3.8 -style=file -i ${DIR}/../plugins/pluginscards/mifareultralight/mifareultralightuidchangerservice.{cpp,hpp}


clang-format-3.8 -style=file -i ${DIR}/../include/logicalaccess/services/reader_service.hpp
clang-format-3.8 -style=file -i ${DIR}/../include/logicalaccess/services/licensechecker/license_checker_service.hpp
