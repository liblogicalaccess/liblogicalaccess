cmake_minimum_required(VERSION 2.6)
project(AxessTMCLegicReaders)

file(GLOB SRCS *.cpp)
file(GLOB include *.h)

file(GLOB include_readercardadapters ${CMAKE_CURRENT_SOURCE_DIR}/ReaderCardAdapters/*.h)
SOURCE_GROUP("Header Files\\ReaderCardAdapters" FILES ${include_readercardadapters})

file(GLOB source_readercardadapters ${CMAKE_CURRENT_SOURCE_DIR}/ReaderCardAdapters/*.cpp)
SOURCE_GROUP("Source Files\\ReaderCardAdapters" FILES ${source_readercardadapters})

set(INCLUDE_LIB ${include} ${include_core} ${include_readercardadapters})
set(SOURCE_LIB ${SRCS} ${source_readercardadapters})

add_library(
	AxessTMCLegicReaders
	SHARED
	
	${SOURCE_LIB}
	${INCLUDE_LIB}
  )
  
target_link_libraries(
	AxessTMCLegicReaders
	liblogicalaccess
	boost_system
	boost_filesystem
)