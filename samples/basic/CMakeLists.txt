

SET(SOURCE basic.cpp)

add_executable(
    basic
    ${SOURCE}
)

remove_definitions(-DLIBLOGICALACCESS_EXPORTS -DLIBLOGICALACCESS_EXPORTS_DLL)

target_link_libraries(
        basic
        logicalaccess
)
