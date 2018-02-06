cmake_minimum_required(VERSION 3.1)
project(iso7816readers)

if (WIN32)
	SET(VERSION_PRODUCTNAME_VALUE "iso7816readers")
	add_definitions(-DVERSION_PRODUCTNAME_VALUE="${VERSION_PRODUCTNAME_VALUE}")
endif()

file(GLOB SRCS *.cpp)
file(GLOB include *.hpp)

file(GLOB include_commands ${CMAKE_CURRENT_SOURCE_DIR}/commands/*.hpp)
SOURCE_GROUP("Header Files\\commands" FILES ${include_commands})

file(GLOB source_commands ${CMAKE_CURRENT_SOURCE_DIR}/commands/*.cpp)
SOURCE_GROUP("Source Files\\commands" FILES ${source_commands})

set(INCLUDE_LIB ${include} ${include_commands})
set(SOURCE_LIB ${SRCS} ${source_commands})

add_library(
	iso7816readers
	SHARED
	${SOURCE_LIB}
	${INCLUDE_LIB}
	${resourcerc}
  )
  
SET_TARGET_PROPERTIES(iso7816readers PROPERTIES VERSION ${LIBLOGICALACCESS_VERSION_STRING})
  
target_link_libraries(
	iso7816readers
	desfirecards
	logicalaccess-cryptolib
	iso7816cards
	samav2cards
	${LIBLOGICALACCESS_LIBRARY}
	twiccards
)

install ( FILES ${include} DESTINATION include/logicalaccess/plugins/readers/iso7816 )
install ( FILES ${include_commands} DESTINATION include/logicalaccess/plugins/readers/iso7816/commands )
install ( TARGETS iso7816readers RUNTIME DESTINATION bin/${LIB_SUFFIX} ARCHIVE DESTINATION lib/${LIB_SUFFIX} LIBRARY DESTINATION lib/${LIB_SUFFIX} )
install ( FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> DESTINATION pdb/${LIB_SUFFIX} OPTIONAL)