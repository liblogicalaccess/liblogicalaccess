cmake_minimum_required(VERSION 2.6)
project(rfideasreaders)

file(GLOB SRCS *.cpp)
file(GLOB include *.hpp)

set(INCLUDE_LIB ${include} ${include_core})
set(SOURCE_LIB ${SRCS})

add_library(
	rfideasreaders
	SHARED
	
	${SOURCE_LIB}
	${INCLUDE_LIB}
  )

SET_TARGET_PROPERTIES(rfideasreaders PROPERTIES VERSION ${LIBLOGICALACCESS_VERSION_STRING})  

target_link_libraries(
	rfideasreaders
	logicalaccess
)

install ( FILES ${include} DESTINATION include/logicalaccess/plugins/readers/rfideas )
install ( TARGETS rfideasreaders LIBRARY DESTINATION lib${LIB_SUFFIX}/logicalaccess )
