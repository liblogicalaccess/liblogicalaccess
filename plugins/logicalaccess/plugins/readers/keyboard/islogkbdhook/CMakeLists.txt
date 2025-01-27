cmake_minimum_required(VERSION 3.5)
project(islogkbdhook)

if (MSVC)
	SET(VERSION_PRODUCTNAME_VALUE "islogkbdhook")
	add_definitions(-DVERSION_PRODUCTNAME_VALUE="${VERSION_PRODUCTNAME_VALUE}")
endif()

file(GLOB SRCS *.cpp)


if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(TARGET_EXE_NAME islogkbdhook64)
    set(BACKING_LIB_NAME islogkbdhooklib64)
else()
    set(TARGET_EXE_NAME islogkbdhook32)
    set(BACKING_LIB_NAME islogkbdhooklib32)
endif()

add_executable(${TARGET_EXE_NAME} WIN32
	${SRCS}
	${LLA_RESOURCE_RC})

SET_TARGET_PROPERTIES(${TARGET_EXE_NAME} PROPERTIES VERSION ${LIBLOGICALACCESS_VERSION_STRING})
target_link_libraries(${TARGET_EXE_NAME}
	logicalaccess
    ${BACKING_LIB_NAME})
target_include_directories(${TARGET_EXE_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../islogkbdhooklib)

install ( TARGETS ${TARGET_EXE_NAME} RUNTIME DESTINATION bin/${LIB_SUFFIX} ARCHIVE DESTINATION lib/${LIB_SUFFIX} LIBRARY DESTINATION lib/${LIB_SUFFIX} )
if (MSVC)
	install(FILES $<TARGET_PDB_FILE:${TARGET_EXE_NAME}> DESTINATION pdb/${LIB_SUFFIX} OPTIONAL)
endif()
