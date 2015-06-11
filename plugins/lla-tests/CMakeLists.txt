cmake_minimum_required(VERSION 2.8.12)

set(TARGET_NAME lla-tests)
set(SOURCE_FILES utils.cpp myclock.cpp subtest_tracker.cpp)

add_library(${TARGET_NAME} SHARED ${SOURCE_FILES})
target_link_libraries(${TARGET_NAME} ${LIBLOGICALACCESS_LIBRARY} pcscreaders)
target_include_directories(${TARGET_NAME} PRIVATE
${CMAKE_SOURCE_DIR}/plugins
${CMAKE_SOURCE_DIR}/include/
)

set (include_tests
${CMAKE_CURRENT_SOURCE_DIR}/myclock.hpp
${CMAKE_CURRENT_SOURCE_DIR}/subtest_tracker.hpp
${CMAKE_CURRENT_SOURCE_DIR}/utils.hpp
${CMAKE_CURRENT_SOURCE_DIR}/macros.hpp
)


install ( TARGETS lla-tests RUNTIME DESTINATION bin/${LIB_SUFFIX} ARCHIVE DESTINATION lib/${LIB_SUFFIX} LIBRARY DESTINATION lib/${LIB_SUFFIX} )
install ( FILES ${include_tests} DESTINATION include/logicalaccess/plugins/lla-tests )
