cmake_minimum_required(VERSION 3.10)
project(basic CXX)

add_executable(basic
        basic.cpp)

target_link_libraries(basic
        iso7816cards
        desfirecards
        logicalaccess
        iso7816cards
        iso7816readers
        pcscreaders
        )

install(TARGETS basic
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib${LIB_SUFFIX}
        ARCHIVE DESTINATION lib${LIB_SUFFIX})
