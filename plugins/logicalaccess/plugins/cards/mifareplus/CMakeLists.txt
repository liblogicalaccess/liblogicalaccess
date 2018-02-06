cmake_minimum_required(VERSION 3.1)
project(mifarepluscards)

if (WIN32)
	SET(VERSION_PRODUCTNAME_VALUE "mifarepluscards")
	add_definitions(-DVERSION_PRODUCTNAME_VALUE="${VERSION_PRODUCTNAME_VALUE}")
endif()

if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Debug")
endif()

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE INCLUDE *.hpp)

add_library(
	mifarepluscards
	SHARED

	${SRCS}
	${INCLUDE}
	${resourcerc}
  )

SET_TARGET_PROPERTIES(mifarepluscards PROPERTIES VERSION ${LIBLOGICALACCESS_VERSION_STRING})

target_link_libraries(
	mifarepluscards
	${LIBLOGICALACCESS_LIBRARY}
	mifarecards
)

install ( FILES ${INCLUDE} DESTINATION include/logicalaccess/plugins/cards/mifareplus )
install ( TARGETS mifarepluscards RUNTIME DESTINATION bin/${LIB_SUFFIX} ARCHIVE DESTINATION lib/${LIB_SUFFIX} LIBRARY DESTINATION lib/${LIB_SUFFIX} )
install ( FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> DESTINATION pdb/${LIB_SUFFIX} OPTIONAL)