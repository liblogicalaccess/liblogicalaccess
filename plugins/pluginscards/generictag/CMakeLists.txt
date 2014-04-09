cmake_minimum_required(VERSION 2.6)
project(generictagcards)

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE INCLUDE *.hpp)

add_library(
	generictagcards
	SHARED
	${SRCS}
	${INCLUDE}
  )

SET_TARGET_PROPERTIES(generictagcards PROPERTIES VERSION ${LIBLOGICALACCESS_VERSION_STRING})

target_link_libraries(
	generictagcards
	${LIBLOGICALACCESS_LIBRARY}
)

install ( FILES ${INCLUDE} DESTINATION include/logicalaccess/plugins/cards/generictag )
install ( TARGETS generictagcards RUNTIME DESTINATION bin/${TARGET_ARCH}/${LIB_SUFFIX} ARCHIVE DESTINATION lib/${TARGET_ARCH}/${LIB_SUFFIX} LIBRARY DESTINATION lib/${TARGET_ARCH}/${LIB_SUFFIX} )