
FILE(GLOB_RECURSE tritium_SOURCES src/*.cpp src/*.cc src/*.c)
LIST(APPEND tritium_SOURCES ${CMAKE_SOURCE_DIR}/version.cpp)

FILE(GLOB_RECURSE tritium_INCLUDES include/*.h)
LIST(APPEND tritium_INCLUDES ${CMAKE_SOURCE_DIR}/version.h ${CMAKE_SOURCE_DIR}/config.h)

INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR} .)
ADD_LIBRARY(Tritium ${tritium_SOURCES})
