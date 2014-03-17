
set(CMAKE_POSITION_INDEPENDENT_CODE ON) 

if(UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
else()
  if (MSVC)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
  endif ()
endif()

file(GLOB_RECURSE SRCS src/*.cpp)
file(GLOB_RECURSE HDRS include/*.h)
file(GLOB_RECURSE HDPPRS include/*.hpp)

add_library(
	logicalaccess-cryptolib
	STATIC
	${SRCS}
	${HDRS}
	${HDPPRS}
  )
  
 
install ( FILES ${INCLUDE} DESTINATION include/logicalaccess/plugins/cryptolib )

