
FILE(GLOB_RECURSE hydrogen_SOURCES src/*.cpp src/*.cc src/*.c)
LIST(APPEND hydrogen_SOURCES ${CMAKE_SOURCE_DIR}/version.cpp)

FILE(GLOB_RECURSE hydrogen_INCLUDES include/*.h)
LIST(APPEND hydrogen_INCLUDES ${CMAKE_SOURCE_DIR}/version.h ${CMAKE_SOURCE_DIR}/config.h)

INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR} include)
ADD_LIBRARY(hydrogen ${hydrogen_SOURCES})
