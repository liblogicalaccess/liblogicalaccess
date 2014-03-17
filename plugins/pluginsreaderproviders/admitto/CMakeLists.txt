cmake_minimum_required(VERSION 2.6)
project(admittoreaders)

file(GLOB SRCS *.cpp)
file(GLOB include *.hpp)

file(GLOB include_readercardadapters ${CMAKE_CURRENT_SOURCE_DIR}/readercardadapters/*.hpp)
SOURCE_GROUP("Header Files\\readercardadapters" FILES ${include_readercardadapters})

file(GLOB source_readercardadapters ${CMAKE_CURRENT_SOURCE_DIR}/readercardadapters/*.cpp)
SOURCE_GROUP("Source Files\\readercardadapters" FILES ${source_readercardadapters})

set(INCLUDE_LIB ${include} ${include_core} ${include_readercardadapters})
set(SOURCE_LIB ${SRCS} ${source_readercardadapters})

add_library(
	admittoreaders
	SHARED
	${SOURCE_LIB}
	${INCLUDE_LIB}
	)

SET_TARGET_PROPERTIES(admittoreaders PROPERTIES VERSION ${LIBLOGICALACCESS_VERSION_STRING})

target_link_libraries(
	admittoreaders
	logicalaccess
)

install ( FILES ${include} DESTINATION include/logicalaccess/plugins/readers/admitto )
install ( FILES ${include_readercardadapters} DESTINATION include/logicalaccess/plugins/readers/admitto/readercardadapters )
install ( TARGETS admittoreaders RUNTIME DESTINATION bin ARCHIVE DESTINATION lib${LIB_SUFFIX}/logicalaccess LIBRARY DESTINATION lib${LIB_SUFFIX}/logicalaccess )
