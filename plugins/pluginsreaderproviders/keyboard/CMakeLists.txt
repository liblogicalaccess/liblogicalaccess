cmake_minimum_required(VERSION 3.1)
project(keyboardreaders)

if (WIN32)
	SET(VERSION_PRODUCTNAME_VALUE "keyboardreaders")
	add_definitions(-DVERSION_PRODUCTNAME_VALUE="${VERSION_PRODUCTNAME_VALUE}")
endif()

file(GLOB SRCS *.cpp)
file(GLOB include *.h)
file(GLOB includepp *.hpp)

file(GLOB include_readercardadapters ${CMAKE_CURRENT_SOURCE_DIR}/readercardadapters/*.hpp)
SOURCE_GROUP("Header Files\\readercardadapters" FILES ${include_readercardadapters})

file(GLOB source_readercardadapters ${CMAKE_CURRENT_SOURCE_DIR}/readercardadapters/*.cpp)
SOURCE_GROUP("Source Files\\readercardadapters" FILES ${source_readercardadapters})

set(INCLUDE_LIB ${include} ${includepp} ${include_core} ${include_readercardadapters})
set(SOURCE_LIB ${SRCS} ${source_readercardadapters})


add_library(
	keyboardreaders
	SHARED
	
	${SOURCE_LIB}
	${INCLUDE_LIB}
	${resourcerc}
  )

SET_TARGET_PROPERTIES(keyboardreaders PROPERTIES VERSION ${LIBLOGICALACCESS_VERSION_STRING})
  
target_link_libraries(
	keyboardreaders
	${LIBLOGICALACCESS_LIBRARY}
	UserEnv
)

install ( FILES ${include} ${includepp} DESTINATION include/logicalaccess/plugins/readers/keyboard )
install ( FILES ${include_readercardadapters} DESTINATION include/logicalaccess/plugins/readers/keyboard/readercardadapters )
install ( TARGETS keyboardreaders RUNTIME DESTINATION bin/${LIB_SUFFIX} ARCHIVE DESTINATION lib/${LIB_SUFFIX} LIBRARY DESTINATION lib/${LIB_SUFFIX} )