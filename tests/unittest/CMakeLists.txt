find_package(GTest REQUIRED)

function(create_test file)
   get_filename_component(test_name ${file} NAME_WE)
   add_executable(${test_name} ${file})

   target_include_directories(${test_name} PRIVATE
           ${GTEST_INCLUDE_DIRS}
           ${CMAKE_SOURCE_DIR}/plugins
   )

   # We link with remotecryptoiks just for
   # iks-bench.
   # This needs cleanup.

   target_link_libraries(${test_name} PUBLIC
           ${GTEST_BOTH_LIBRARIES} ${Boost_LIBRARIES}
           pthread pcscreaders stidprgreaders
           remotecryptoiksunified
           epasscards
           logicalaccess)

    target_compile_options(${test_name} PRIVATE -O3)
endfunction()

function(add_gtest_test file)
    get_filename_component(test_name ${file} NAME_WE)
    create_test(${file})
    add_test(${test_name} ${test_name})
endfunction()

add_gtest_test(test_atrparser.cpp)
add_gtest_test(test_elapsed_time_counter.cpp)
add_gtest_test(test_epass_utils.cpp)
add_gtest_test(test_manchester.cpp)
add_gtest_test(test_stid_prg_utils.cpp)
add_gtest_test(test_key_storage.cpp)
add_gtest_test(test_cl1356plus_utils.cpp)
add_gtest_test(test_format.cpp)
add_gtest_test(test_bitsetstream.cpp)
add_gtest_test(test_signature.cpp)
add_gtest_test(test_diversification.cpp)

if (NOT LLA_DISABLE_IKS)
    add_gtest_test(test_iks_bench.cpp)
endif()

add_gtest_test(test_tlv.cpp)
