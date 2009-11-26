######################################################################
### Tritium Build Script (CMake)                                   ###
### http://gabe.is-a-geek.org/composite/                           ###
######################################################################
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

###
### This script will one day be standalone.
### For now, it depends on the CMakeLists.txt at the top
### of the source tree.
###

######################################################################
### LIBRARY SOURCES AND BUILD                                      ###
######################################################################

FILE(GLOB_RECURSE tritium_SOURCES src/*.cpp src/*.cc src/*.c)
LIST(APPEND tritium_SOURCES ${CMAKE_SOURCE_DIR}/version.cpp)

FILE(GLOB_RECURSE tritium_INCLUDES Tritium/*.hpp src/*.hpp)
LIST(APPEND tritium_INCLUDES ${CMAKE_SOURCE_DIR}/version.h ${CMAKE_SOURCE_DIR}/config.h)

INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR} .)
ADD_LIBRARY(Tritium SHARED ${tritium_SOURCES} ${CMAKE_BINARY_DIR}/version.cpp)

TARGET_LINK_LIBRARIES(Tritium
  ${QT_LIBRARIES}
  ${LibSndfile_LIBRARIES}
  ${JACK_LIBRARIES}
  ${FLAC_LIBRARIES}
  ${FLAC++_LIBRARIES}
  ${LRDF_LIBRARIES}
  ${LibTar_LIBRARIES}
  ${ZLIB_LIBRARIES}
  ${LibArchive_LIBRARIES}
  )

INSTALL(TARGETS Tritium LIBRARY DESTINATION lib)

ADD_SUBDIRECTORY(test)
ENABLE_TESTING()
