cmake_minimum_required(VERSION 2.6)
project(rfideasreaders)

if (WIN32)
	SET(VERSION_PRODUCTNAME_VALUE "rfideasreaders")
	add_definitions(-DVERSION_PRODUCTNAME_VALUE="${VERSION_PRODUCTNAME_VALUE}")
endif()

file(GLOB SRCS *.cpp)
file(GLOB include *.hpp)

set(INCLUDE_LIB ${include} ${include_core})
set(SOURCE_LIB ${SRCS})

add_library(
	rfideasreaders
	SHARED
	
	${SOURCE_LIB}
	${INCLUDE_LIB}
	${resourcerc}
  )

SET_TARGET_PROPERTIES(rfideasreaders PROPERTIES VERSION ${LIBLOGICALACCESS_VERSION_STRING})  

target_link_libraries(
	rfideasreaders
	${LIBLOGICALACCESS_LIBRARY}
)

install ( FILES ${include} DESTINATION include/logicalaccess/plugins/readers/rfideas )
install ( TARGETS rfideasreaders RUNTIME DESTINATION bin/${LIB_SUFFIX} ARCHIVE DESTINATION lib/${LIB_SUFFIX} LIBRARY DESTINATION lib/${LIB_SUFFIX} )