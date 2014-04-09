cmake_minimum_required(VERSION 2.6)
project(infineonmydcards)

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE INCLUDE *.hpp)

add_library(
	infineonmydcards
	SHARED
	${SRCS}
	${INCLUDE}
  )

SET_TARGET_PROPERTIES(infineonmydcards PROPERTIES VERSION ${LIBLOGICALACCESS_VERSION_STRING})

target_link_libraries(
	infineonmydcards
	iso15693cards
	${LIBLOGICALACCESS_LIBRARY}
)

install ( FILES ${INCLUDE} DESTINATION include/logicalaccess/plugins/cards/infineonmyd )
install ( TARGETS infineonmydcards RUNTIME DESTINATION bin/${TARGET_ARCH}/${LIB_SUFFIX} ARCHIVE DESTINATION lib/${TARGET_ARCH}/${LIB_SUFFIX} LIBRARY DESTINATION lib/${TARGET_ARCH}/${LIB_SUFFIX} )