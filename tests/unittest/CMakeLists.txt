function(create_test file)
    get_filename_component(test_name ${file} NAME_WE)
    add_executable(${test_name} ${file})

    target_include_directories(${test_name} PRIVATE
            ${CMAKE_SOURCE_DIR}/plugins)

    if (MSVC)
        target_link_libraries(${test_name} PUBLIC
                gtest::gtest
                boost::boost
                pcscreaders
                epasscards
                logicalaccess)
    else ()
        target_link_libraries(${test_name} PUBLIC
                gtest::gtest
                boost::boost
                pthread
                pcscreaders
                epasscards
                logicalaccess)
    endif ()
endfunction()

function(add_gtest_test file)
    get_filename_component(test_name ${file} NAME_WE)
    create_test(${file})
    add_test(NAME ${test_name} COMMAND ${test_name} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endfunction()

find_package(boost)
find_package(gtest)

add_gtest_test(test_atrparser.cpp)
add_gtest_test(test_elapsed_time_counter.cpp)
add_gtest_test(test_epass_utils.cpp)
add_gtest_test(test_manchester.cpp)
add_gtest_test(test_key_storage.cpp)
add_gtest_test(test_cl1356plus_utils.cpp)
add_gtest_test(test_format.cpp)
add_gtest_test(test_bitsetstream.cpp)
add_gtest_test(test_diversification.cpp)
add_gtest_test(test_regex.cpp)
add_gtest_test(test_epass_verification_and_parsing.cpp)
add_gtest_test(test_json_dump.cpp)
add_gtest_test(test_signature.cpp)
add_gtest_test(test_tlv.cpp)
add_gtest_test(test_asn1.cpp)
add_gtest_test(test_nfc_data_management.cpp)
