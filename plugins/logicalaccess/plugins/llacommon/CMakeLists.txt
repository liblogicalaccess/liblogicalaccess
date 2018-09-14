set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
else()
    if (MSVC)
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
    endif ()
endif()

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE HDRS *.h)
file(GLOB_RECURSE HDPPRS *.hpp)

set(INCLUDE ${HDRS} ${HDPPRS} )

add_library(
        llacommon
        SHARED
        ${SRCS}
        ${INCLUDE}
)

if (ANDROID)
# Android need to link against w/e the library requires.
# There is a Android CMake flag to prevent this behavior, but the system linker
# on Android behavior in a way that I can't understand, and build-time error it detects
# seem to always translate to runtime-link-time issue.
target_link_libraries(llacommon PUBLIC ${Boost_LIBRARIES})
else()
    # Travis test
    target_link_libraries(llacommon PUBLIC ${Boost_LIBRARIES})
endif()

install ( FILES ${INCLUDE} DESTINATION ${LIBLOGICALACCESS_INCLUDE_DIR}/logicalaccess/plugins/llacommon )
install ( TARGETS llacommon ARCHIVE DESTINATION lib/${LIB_SUFFIX} LIBRARY DESTINATION lib/${LIB_SUFFIX} )
if (MSVC)
    install(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> DESTINATION pdb/${LIB_SUFFIX} OPTIONAL)
endif()
