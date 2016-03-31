cmake_minimum_required(VERSION 2.8.12)

lla_test_register_include(
${CMAKE_SOURCE_DIR}/plugins/pluginscards/desfire/
${CMAKE_SOURCE_DIR}/plugins/pluginscards/samav2/
${CMAKE_SOURCE_DIR}/plugins/pluginscards/iso7816/
${CMAKE_SOURCE_DIR}/plugins/readers/iso7816/commands/
${CMAKE_SOURCE_DIR}/plugins/pluginsreaderproviders/iso7816/
${CMAKE_SOURCE_DIR}/plugins/
)

lla_create_test(reader_techno test_general)
lla_create_test(reader_techno test_led)
lla_create_test(reader_techno test_lcd)
lla_create_test(reader_techno test_prox_read_format)
lla_create_test(reader_techno test_id3_card_select)
lla_create_test(reader_techno test_change_uid)
