cmake_minimum_required(VERSION 2.6)
project(A3MLGM5600Readers)


file(GLOB SRCS *.cpp)
file(GLOB include *.h)

file(GLOB include_readercardadapters ${CMAKE_CURRENT_SOURCE_DIR}/ReaderCardAdapters/*.h)
SOURCE_GROUP("Header Files\\ReaderCardAdapters" FILES ${include_readercardadapters})

file(GLOB source_readercardadapters ${CMAKE_CURRENT_SOURCE_DIR}/ReaderCardAdapters/*.cpp)
SOURCE_GROUP("Source Files\\ReaderCardAdapters" FILES ${source_readercardadapters})

set(INCLUDE_LIB ${include} ${include_core} ${include_readercardadapters})
set(SOURCE_LIB ${SRCS} ${source_readercardadapters})

add_library(
	A3MLGM5600Readers
	SHARED
	
	${SOURCE_LIB}
	${INCLUDE_LIB}
  )

set(LINK_OPT)
if (UNIX)
  set(LINK_OPT pthread boost_system boost_filesystem)
endif()

target_link_libraries(
    A3MLGM5600Readers
    liblogicalaccess
    ${LINK_OPT}
)