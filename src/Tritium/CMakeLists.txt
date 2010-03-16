######################################################################
### Tritium Build Script (CMake)                                   ###
### http://gabe.is-a-geek.org/composite/                           ###
######################################################################
CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

######################################################################
### REQUIRED LIBRARIES                                             ###
######################################################################

###
### LADSPA
###
add_definitions(-DLADSPA_SUPPORT)

###
### Qt 4 http://qt.nokia.com/
###

SET(QT_USE_QTXML true)
FIND_PACKAGE(Qt4 4.3.0 REQUIRED)
INCLUDE(${QT_USE_FILE})

###
### JACK http://jackaudio.org/
###
find_package(JACK REQUIRED)
include_directories(${JACK_INCLUDE_DIRS})
set(LIBS ${JACK_LIBRARIES})
add_definitions(-DJACK_SUPPORT)

###
### FLAC http://flac.sourceforge.net/
###
find_package(FLAC REQUIRED)
include_directories(${FLAC_INCLUDE_DIRS})
set(LIBS ${LIBS} ${FLAC_LIBRARIES})
add_definitions(-DFLAC_SUPPORT)

find_package(FLAC++ REQUIRED)
include_directories(${FLAC++_INCLUDE_DIRS})
set(LIBS ${LIBS} ${FLAC++_LIBRARIES})
add_definitions(-DFLAC_SUPPORT)

###
### libsndfile http://www.mega-nerd.com/libsndfile/
###
find_package(LibSndfile REQUIRED)
include_directories(${LibSndfile_INCLUDE_DIRS})
set(LIBS ${LIBS} ${LibSndfile_LIBRARIES})

### ...................................................
### Either libtar and libz, or libarchive are required.
### ...................................................

###
### libtar http://www.feep.net/libtar/
### Zlib http://www.zlib.net/
###
IF(NOT WANT_LIBARCHIVE)
  find_package(LibTar)
  find_package(ZLIB)
ENDIF(NOT WANT_LIBARCHIVE)
IF(LibTar_FOUND AND ZLIB_FOUND)
	include_directories(${LibTar_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS})
	set(LIBS ${LIBS} ${LibTar_LIBRARIES} ${ZLIB_LIBRARIES})
ENDIF(LibTar_FOUND AND ZLIB_FOUND)

###
### libarchive http://code.google.com/p/libarchive/
###
IF(LibTar_FOUND AND ZLIB_FOUND)
ELSE(LibTar_FOUND AND ZLIB_FOUND)
       find_package(LibArchive REQUIRED)
       include_directories(${LibArchive_INCLUDE_DIRS})
       set(LIBS ${LIBS} ${LibArchive_LIBRARIES})
       add_definitions(-DLIBARCHIVE_SUPPORT)
ENDIF(LibTar_FOUND AND ZLIB_FOUND)

###
### Boost Framework http://www.boost.org/
### Used for unit testing
###
find_package(Boost 1.33 COMPONENTS unit_test_framework REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

######################################################################
### OPTIONAL LIBRARIES                                             ###
######################################################################

IF(WANT_LRDF)
  find_package(LRDF REQUIRED)
  include_directories(${LRDF_INCLUDE_DIRS})
  set(LIBS ${LIBS} ${LRDF_LIBRARIES})
  add_definitions(-DLRDF_SUPPORT)
ENDIF(WANT_LRDF)

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
  ${LIBS}
  )

SET_TARGET_PROPERTIES(Tritium
  PROPERTIES
  LINK_INTERFACE_LIBRARIES ""
  )

INSTALL(TARGETS Tritium LIBRARY DESTINATION lib)

######################################################################
### CONFIGURATION SUMMARY                                          ###
######################################################################

MESSAGE("\n"
"Configuration Summary for Tritium\n"
"---------------------------------\n"
)

MACRO(lib_report name)
  IF(${name}_FOUND)
    message("ENABLED..... ${name}")
  ELSE(${name}_FOUND)
    message("disabled.... ${name}")
  ENDIF(${name}_FOUND)
ENDMACRO(lib_report)

lib_report(QT4)
lib_report(JACK)
lib_report(LibSndfile)
lib_report(LRDF)
lib_report(FLAC)
lib_report(FLAC++)
lib_report(LibTar)
lib_report(ZLIB)
lib_report(LibArchive)
lib_report(Boost)
lib_report(Boost_UNIT_TEST_FRAMEWORK)

######################################################################
### UNIT TESTS                                                     ###
######################################################################

ADD_SUBDIRECTORY(test)
ENABLE_TESTING()

