cmake_minimum_required(VERSION 2.6)
project(samav2cards)

if (WIN32)
	SET(VERSION_PRODUCTNAME_VALUE "samav2cards")
	add_definitions(-DVERSION_PRODUCTNAME_VALUE="${VERSION_PRODUCTNAME_VALUE}")
endif()

file(GLOB_RECURSE SRCS *.cpp)
file(GLOB_RECURSE INCLUDE *.hpp)

file(GLOB include_commands ${CMAKE_CURRENT_SOURCE_DIR}/commands/*.hpp)
SOURCE_GROUP("Header Files\\commands" FILES ${include_commands})

file(GLOB source_commands ${CMAKE_CURRENT_SOURCE_DIR}/commands/*.cpp)
SOURCE_GROUP("Source Files\\commands" FILES ${source_commands})

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../)

if (WIN32)
  LINK_DIRECTORIES(${LINK_DIRECTORIES} ${CMAKE_CURRENT_SOURCE_DIR}/../desfire/bin/${TARGET_ARCH}/${VC_FOLDER_NAME})
else()
  LINK_DIRECTORIES(${LINK_DIRECTORIES} ${CMAKE_CURRENT_SOURCE_DIR}/../desfire)
endif()

add_library(
	samav2cards
	SHARED
	${SRCS}
	${source_commands}
	${INCLUDE}
	${include_commands}
	${resourcerc}
  )

SET_TARGET_PROPERTIES(samav2cards PROPERTIES VERSION ${LIBLOGICALACCESS_VERSION_STRING})

target_link_libraries(
	samav2cards
	desfirecards
	logicalaccess-cryptolib
	${LIBLOGICALACCESS_LIBRARY}
)

install ( FILES ${INCLUDE} DESTINATION include/logicalaccess/plugins/cards/samav2 )
install ( TARGETS samav2cards RUNTIME DESTINATION bin/${LIB_SUFFIX} ARCHIVE DESTINATION lib/${LIB_SUFFIX} LIBRARY DESTINATION lib/${LIB_SUFFIX} )
if (MSVC)
	install(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> DESTINATION pdb/${LIB_SUFFIX} OPTIONAL)
endif()