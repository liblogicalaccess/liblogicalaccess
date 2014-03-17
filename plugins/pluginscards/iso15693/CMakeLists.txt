cmake_minimum_required(VERSION 2.6)
project(iso15693cards)

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE INCLUDE *.hpp)

add_library(
	iso15693cards
	SHARED
	${SRCS}
  )

SET_TARGET_PROPERTIES(iso15693cards PROPERTIES VERSION ${LIBLOGICALACCESS_VERSION_STRING})

target_link_libraries(
	iso15693cards
	logicalaccess
)

install ( FILES ${INCLUDE} DESTINATION include/logicalaccess/plugins/cards/iso15693 )
install ( TARGETS iso15693cards RUNTIME DESTINATION bin ARCHIVE DESTINATION lib${LIB_SUFFIX}/logicalaccess LIBRARY DESTINATION lib${LIB_SUFFIX}/logicalaccess )
