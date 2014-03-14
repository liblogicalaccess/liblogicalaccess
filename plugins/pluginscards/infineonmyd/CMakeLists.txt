cmake_minimum_required(VERSION 2.6)
project(infineonmydcards)

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE INCLUDE *.hpp)

add_library(
	infineonmydcards
	SHARED
	${SRCS}
  )

SET_TARGET_PROPERTIES(infineonmydcards PROPERTIES VERSION ${LIBLOGICALACCESS_VERSION_STRING})

target_link_libraries(
	infineonmydcards
	iso15693cards
	logicalaccess
)

install ( FILES ${INCLUDE} DESTINATION include/logicalaccess/plugins/cards/infineonmyd )
install ( TARGETS infineonmydcards LIBRARY DESTINATION lib${LIB_SUFFIX}/logicalaccess )
