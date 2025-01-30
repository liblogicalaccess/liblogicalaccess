file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE HDRS *.h)
file(GLOB_RECURSE HDPPRS *.hpp)

set(INCLUDE ${HDRS} ${HDPPRS})
find_package(boost)
add_library(llacommon
        SHARED
        ${SRCS}
        ${INCLUDE})

if (MSVC)
    target_compile_options(llacommon PUBLIC "/wd4251")
endif ()

# We somewhat cheat with include path here...
# They are propagated to other libraries because other libs
# relies on llacommon...
target_include_directories(llacommon PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../include
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../../plugins)

target_link_libraries(llacommon PUBLIC boost::boost)
generate_export_header(llacommon
        EXPORT_MACRO_NAME LLA_COMMON_API
        EXPORT_FILE_NAME "${CMAKE_CURRENT_SOURCE_DIR}/lla_common_api.hpp")
target_compile_definitions(llacommon PRIVATE llacommon_EXPORTS=1)
install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/lla_common_api.hpp" DESTINATION include/logicalaccess/plugins/llacommon)

install(FILES ${INCLUDE} DESTINATION include/logicalaccess/plugins/llacommon)
install(TARGETS llacommon RUNTIME DESTINATION bin/${LIB_SUFFIX} ARCHIVE DESTINATION lib/${LIB_SUFFIX} LIBRARY DESTINATION lib/${LIB_SUFFIX})
if (MSVC)
    install(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> DESTINATION pdb/${LIB_SUFFIX} OPTIONAL)
endif ()
