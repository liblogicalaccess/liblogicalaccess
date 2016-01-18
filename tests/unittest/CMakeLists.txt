find_package(GTest REQUIRED)


function(create_test file)
   get_filename_component(test_name ${file} NAME_WE)
   add_executable(${test_name} ${file})

   target_include_directories(${test_name} PRIVATE
           ${GTEST_INCLUDE_DIRS}
           ${CMAKE_SOURCE_DIR}/plugins
   )

   target_link_libraries(${test_name} ${GTEST_BOTH_LIBRARIES} ${Boost_LIBRARIES} pthread logicalaccess pcscreaders)
endfunction()

function(add_gtest_test file)
    get_filename_component(test_name ${file} NAME_WE)
    create_test(${file})
    add_test(${test_name} ${test_name})
endfunction()

add_gtest_test(test_atrparser.cpp)
add_gtest_test(test_elapsed_time_counter.cpp)
