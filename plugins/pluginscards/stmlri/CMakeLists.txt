cmake_minimum_required(VERSION 2.6)
project(stmlri512cards)

if (WIN32)
	SET(VERSION_PRODUCTNAME_VALUE "stmlri512cards")
	add_definitions(-DVERSION_PRODUCTNAME_VALUE="${VERSION_PRODUCTNAME_VALUE}")
endif()

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE INCLUDE *.hpp)

add_library(
	stmlri512cards
	SHARED

	${SRCS}
	${INCLUDE}
	${resourcerc}
  )

SET_TARGET_PROPERTIES(stmlri512cards PROPERTIES VERSION ${LIBLOGICALACCESS_VERSION_STRING})

target_link_libraries(
	stmlri512cards
	${LIBLOGICALACCESS_LIBRARY}
)

install ( FILES ${INCLUDE} DESTINATION include/logicalaccess/plugins/cards/stmlri512 )
install ( TARGETS stmlri512cards RUNTIME DESTINATION bin/${LIB_SUFFIX} ARCHIVE DESTINATION lib/${LIB_SUFFIX} LIBRARY DESTINATION lib/${LIB_SUFFIX} )