cmake_minimum_required(VERSION 3.1)

lla_create_test(other test_islog_key_server)

lla_test_register_include(
${CMAKE_SOURCE_DIR}/plugins/pluginscards/
${CMAKE_SOURCE_DIR}/plugins/pluginsreaderproviders/
${CMAKE_SOURCE_DIR}/plugins/
)

lla_create_test(other test_access_control_format_prox)

