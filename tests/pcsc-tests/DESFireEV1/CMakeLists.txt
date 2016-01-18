cmake_minimum_required(VERSION 2.8.12)

lla_test_register_include(
${CMAKE_SOURCE_DIR}/plugins/pluginscards/desfire/
${CMAKE_SOURCE_DIR}/plugins/pluginscards/samav2/
${CMAKE_SOURCE_DIR}/plugins/pluginscards/iso7816/
${CMAKE_SOURCE_DIR}/plugins/readers/iso7816/commands/
${CMAKE_SOURCE_DIR}/plugins/
)

lla_create_test(desfireev1 test_general)
lla_create_test(desfireev1 test_storage_service)
lla_create_test(desfireev1 test_changekey_iks)
