cmake_minimum_required(VERSION 3.10)
project(proxlitecards)

if (MSVC)
    SET(VERSION_PRODUCTNAME_VALUE "proxlitecards")
    add_definitions(-DVERSION_PRODUCTNAME_VALUE="${VERSION_PRODUCTNAME_VALUE}")
endif ()

set(HEADER_RELATIVE_INSTALL_DIR "include/logicalaccess/plugins/cards/proxlite")

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE INCLUDE *.hpp)

add_library(proxlitecards
        SHARED
        ${SRCS}
        ${INCLUDE}
        ${LLA_RESOURCE_RC})

SET_TARGET_PROPERTIES(proxlitecards PROPERTIES VERSION ${LIBLOGICALACCESS_VERSION_STRING})

target_link_libraries(proxlitecards
        logicalaccess)
LLA_GENERATE_EXPORTS(LLA_CARDS_PROXLITE)


install(FILES ${INCLUDE} DESTINATION ${HEADER_RELATIVE_INSTALL_DIR})
install(TARGETS proxlitecards RUNTIME DESTINATION bin/${LIB_SUFFIX} ARCHIVE DESTINATION lib/${LIB_SUFFIX} LIBRARY DESTINATION lib/${LIB_SUFFIX})
if (MSVC)
    install(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> DESTINATION pdb/${LIB_SUFFIX} OPTIONAL)
endif ()
