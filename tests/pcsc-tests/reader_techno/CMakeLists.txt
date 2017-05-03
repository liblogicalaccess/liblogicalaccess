cmake_minimum_required(VERSION 2.8.12)

lla_test_register_include(
${CMAKE_SOURCE_DIR}/plugins/pluginscards/
${CMAKE_SOURCE_DIR}/plugins/pluginsreaderproviders/
${CMAKE_SOURCE_DIR}/plugins/
)

lla_create_test(reader_techno test_general)
lla_create_test(reader_techno test_led)
lla_create_test(reader_techno test_lcd)
lla_create_test(reader_techno test_prox_read_format)
lla_create_test(reader_techno test_id3_card_select)
lla_create_test(reader_techno test_change_uid)
