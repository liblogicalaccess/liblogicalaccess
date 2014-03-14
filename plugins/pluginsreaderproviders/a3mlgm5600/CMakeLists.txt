cmake_minimum_required(VERSION 2.6)
project(a3mlgm5600readers)


file(GLOB SRCS *.cpp)
file(GLOB include *.hpp)

file(GLOB include_readercardadapters ${CMAKE_CURRENT_SOURCE_DIR}/readercardadapters/*.hpp)
SOURCE_GROUP("Header Files\\readercardadapters" FILES ${include_readercardadapters})

file(GLOB source_readercardadapters ${CMAKE_CURRENT_SOURCE_DIR}/readercardadapters/*.cpp)
SOURCE_GROUP("Source Files\\readercardadapters" FILES ${source_readercardadapters})

set(INCLUDE_LIB ${include} ${include_core} ${include_readercardadapters})
set(SOURCE_LIB ${SRCS} ${source_readercardadapters})

add_library(
	a3mlgm5600readers
	SHARED
	${SOURCE_LIB}
  )


SET_TARGET_PROPERTIES(a3mlgm5600readers PROPERTIES VERSION ${LIBLOGICALACCESS_VERSION_STRING})

target_link_libraries(
    a3mlgm5600readers
    logicalaccess
    ${LINK_OPT}
)

install ( FILES ${include} DESTINATION include/logicalaccess/plugins/readers/a3mlgm5600 )
install ( FILES ${include_readercardadapters} DESTINATION include/logicalaccess/plugins/readers/a3mlgm5600/readercardadapters )
install ( TARGETS a3mlgm5600readers LIBRARY DESTINATION lib${LIB_SUFFIX}/logicalaccess )
