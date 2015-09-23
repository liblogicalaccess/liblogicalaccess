cmake_minimum_required(VERSION 2.8.12)

lla_test_register_include("")
lla_create_test(mifare1k test_general)
lla_create_test(mifare1k test_increment)
lla_create_test(mifare1k test_iterate_memory)
lla_create_test(mifare1k test_storage_service)
lla_create_test(mifare1k test_utils)