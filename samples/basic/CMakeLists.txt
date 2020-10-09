cmake_minimum_required(VERSION 3.1)
project(basic CXX)

add_executable(basic
        basic.cpp)

target_link_libraries(basic
        logicalaccess)

install(TARGETS basic
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib${LIB_SUFFIX}
        ARCHIVE DESTINATION lib${LIB_SUFFIX})
