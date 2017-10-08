cmake_minimum_required(VERSION 3.1)

lla_test_register_include(
${CMAKE_SOURCE_DIR}/plugins/pluginscards/
${CMAKE_SOURCE_DIR}/plugins/pluginsreaderproviders/
${CMAKE_SOURCE_DIR}/plugins/
)

lla_create_test(desfireev1 test_general)
lla_create_test(desfireev1 test_storage_service)
lla_create_test(desfireev1 test_changekey_iks)
