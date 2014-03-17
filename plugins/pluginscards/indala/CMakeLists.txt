cmake_minimum_required(VERSION 2.6)
project(indalacards)

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE INCLUDE *.hpp)

add_library(
	indalacards
	SHARED
	${SRCS}
	${INCLUDE}
  )

SET_TARGET_PROPERTIES(indalacards PROPERTIES VERSION ${LIBLOGICALACCESS_VERSION_STRING})

target_link_libraries(
	indalacards
	logicalaccess
)

install ( FILES ${INCLUDE} DESTINATION include/logicalaccess/plugins/cards/indala )
install ( TARGETS indalacards RUNTIME DESTINATION bin ARCHIVE DESTINATION lib${LIB_SUFFIX}/logicalaccess LIBRARY DESTINATION lib${LIB_SUFFIX}/logicalaccess )
