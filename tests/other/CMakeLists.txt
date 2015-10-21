cmake_minimum_required(VERSION 2.8.12)

lla_create_test(other test_islog_key_server)

lla_test_register_include(
${CMAKE_SOURCE_DIR}/plugins/pluginscards/desfire/
${CMAKE_SOURCE_DIR}/plugins/pluginscards/samav2/
${CMAKE_SOURCE_DIR}/plugins/pluginscards/iso7816/
${CMAKE_SOURCE_DIR}/plugins/readers/iso7816/commands/
${CMAKE_SOURCE_DIR}/plugins/
)

lla_create_test(other test_access_control_format_prox)

