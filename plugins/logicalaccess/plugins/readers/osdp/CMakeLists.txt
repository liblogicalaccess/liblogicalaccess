cmake_minimum_required(VERSION 3.1)
project(osdpreaders)

if (WIN32)
	SET(VERSION_PRODUCTNAME_VALUE "osdpreaders")
	add_definitions(-DVERSION_PRODUCTNAME_VALUE="${VERSION_PRODUCTNAME_VALUE}")
endif()

file(GLOB SRCS *.cpp)
file(GLOB include *.hpp)

file(GLOB include_readercardadapters ${CMAKE_CURRENT_SOURCE_DIR}/readercardadapters/*.hpp)
SOURCE_GROUP("Header Files\\readercardadapters" FILES ${include_readercardadapters})

file(GLOB source_readercardadapters ${CMAKE_CURRENT_SOURCE_DIR}/readercardadapters/*.cpp)
SOURCE_GROUP("Source Files\\readercardadapters" FILES ${source_readercardadapters})

set(INCLUDE_LIB ${include} ${include_readercardadapters})
set(SOURCE_LIB ${SRCS} ${source_readercardadapters})

add_library(
	osdpreaders
	SHARED
	${SOURCE_LIB}
	${INCLUDE_LIB}
	${resourcerc}
  )
  
SET_TARGET_PROPERTIES(osdpreaders PROPERTIES VERSION ${LIBLOGICALACCESS_VERSION_STRING})
  
target_link_libraries(
	osdpreaders
	logicalaccess-cryptolib
	iso7816readers
	iso7816cards
	${LIBLOGICALACCESS_LIBRARY}
	${OPENSSL_LIBRARIES}
)

install ( FILES ${include} DESTINATION include/logicalaccess/plugins/readers/osdp )
install ( FILES ${include_readercardadapters} DESTINATION include/logicalaccess/plugins/readers/osdp/readercardadapters )
install ( TARGETS osdpreaders RUNTIME DESTINATION bin/${LIB_SUFFIX} ARCHIVE DESTINATION lib/${LIB_SUFFIX} LIBRARY DESTINATION lib/${LIB_SUFFIX} )
install ( FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> DESTINATION pdb/${LIB_SUFFIX} OPTIONAL)