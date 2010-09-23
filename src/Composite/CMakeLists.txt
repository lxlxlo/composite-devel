######################################################################
### Tritium Build Script (CMake)                                   ###
### http://gabe.is-a-geek.org/composite/                           ###
######################################################################
CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

######################################################################
### REQUIRED LIBRARIES                                             ###
######################################################################

###
### Qt 4 http://qt.nokia.com/
###

SET(QT_USE_QTXML true)
FIND_PACKAGE(Qt4 4.3.0 REQUIRED)
INCLUDE(${QT_USE_FILE})


######################################################################
### LIBRARY SOURCES AND BUILD                                      ###
######################################################################

LIST(APPEND libcomposite_HEADERS
  ${CMAKE_BINARY_DIR}/config.h
  Composite/Main/MainWindow.hpp
  )

# These must also appear in libcomposite_HEADERS
LIST(APPEND libcomposite_MOC_HEADERS
  Composite/Main/MainWindow.hpp
  )

LIST(APPEND libcomposite_SOURCES
  src/Main/MainWindow.cpp
  )

FILE(GLOB_RECURSE libcomposite_EXTRA_HEADERS
  ${CMAKE_SOURCE_DIR}/src/Tritium/Tritium/*
  )

INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR} .)

ADD_LIBRARY(Composite SHARED
  ${libcomposite_SOURCES}
  ${libcomposite_HEADERS}
  ${libcomposite_EXTRA_HEADERS}
  )

TARGET_LINK_LIBRARIES(Composite
  Tritium
  ${QT_LIBRARIES}
  ${LIBS}
  )

SET_TARGET_PROPERTIES(Composite
  PROPERTIES
  LINK_INTERFACE_LIBRARIES ""
  )

INSTALL(TARGETS Composite LIBRARY DESTINATION lib)

######################################################################
### CONFIGURATION SUMMARY                                          ###
######################################################################

MESSAGE("\n"
"Configuration Summary for Composite\n"
"-----------------------------------\n"
)

MACRO(lib_report name)
  IF(${name}_FOUND)
    message("ENABLED..... ${name}")
  ELSE(${name}_FOUND)
    message("disabled.... ${name}")
  ENDIF(${name}_FOUND)
ENDMACRO(lib_report)

lib_report(QT4)
