cmake_minimum_required(VERSION 2.6)
project(rplethreaders)

if (NOT TARGET_ARCH)
	set(TARGET_ARCH "Win32")
endif()

file(GLOB SRCS *.cpp)
file(GLOB include *.hpp)

file(GLOB include_readercardadapters ${CMAKE_CURRENT_SOURCE_DIR}/readercardadapters/*.hpp)
SOURCE_GROUP("Header Files\\readercardadapters" FILES ${include_readercardadapters})

file(GLOB source_readercardadapters ${CMAKE_CURRENT_SOURCE_DIR}/readercardadapters/*.cpp)
SOURCE_GROUP("Source Files\\readercardadapters" FILES ${source_readercardadapters})

set(INCLUDE_LIB ${include} ${include_core} ${include_readercardadapters})
set(SOURCE_LIB ${SRCS} ${source_readercardadapters})

set (LINK_OPT)

add_library(
	rplethreaders
	SHARED
	
	${SOURCE_LIB}
	${INCLUDE_LIB}
  )

if (WIN32)
  set(LINK_OPT liblogicalaccess)
else()
  set(LINK_OPT logicalaccess pthread boost_system boost_filesystem)
endif()

SET_TARGET_PROPERTIES(rplethreaders PROPERTIES VERSION 1.65)
  
target_link_libraries(
	rplethreaders
	${LINK_OPT}
)
