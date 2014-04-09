cmake_minimum_required(VERSION 2.6)
project(scielreaders)

file(GLOB SRCS *.cpp)
file(GLOB include *.hpp)

file(GLOB include_readercardadapters ${CMAKE_CURRENT_SOURCE_DIR}/readercardadapters/*.hpp)
SOURCE_GROUP("Header Files\\readercardadapters" FILES ${include_readercardadapters})

file(GLOB source_readercardadapters ${CMAKE_CURRENT_SOURCE_DIR}/readercardadapters/*.cpp)
SOURCE_GROUP("Source Files\\readercardadapters" FILES ${source_readercardadapters})

set(INCLUDE_LIB ${include} ${include_core} ${include_readercardadapters})
set(SOURCE_LIB ${SRCS} ${source_readercardadapters})

add_library(
	scielreaders
	SHARED
	${SOURCE_LIB}
	${INCLUDE_LIB}
  )

SET_TARGET_PROPERTIES(scielreaders PROPERTIES VERSION ${LIBLOGICALACCESS_VERSION_STRING})
  
target_link_libraries(
	scielreaders
	${LIBLOGICALACCESS_LIBRARY}
)

install ( FILES ${include} ${includepp} DESTINATION include/logicalaccess/plugins/readers/sciel )
install ( FILES ${include_readercardadapters} DESTINATION include/logicalaccess/plugins/readers/sciel/readercardadapters )
install ( TARGETS scielreaders RUNTIME DESTINATION bin/${TARGET_ARCH}/${LIB_SUFFIX} ARCHIVE DESTINATION lib/${TARGET_ARCH}/${LIB_SUFFIX} LIBRARY DESTINATION lib/${TARGET_ARCH}/${LIB_SUFFIX} )