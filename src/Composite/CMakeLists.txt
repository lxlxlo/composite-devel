######################################################################
### Composite Build Script (CMake)                                 ###
### http://gabe.is-a-geek.org/composite/                           ###
######################################################################
CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

######################################################################
### SUBPROJECTS                                                    ###
######################################################################

# art generation has its own CMake file:
ADD_SUBDIRECTORY(img)

SET(BINDIR ${CMAKE_CURRENT_BINARY_DIR})
SET(SRCDIR ${CMAKE_CURRENT_SOURCE_DIR})

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

# See also libcomposite_MOC_HEADERS
LIST(APPEND libcomposite_HEADERS
  ${CMAKE_BINARY_DIR}/config.h

  Looks/Colors.hpp
  Looks/Sizes.hpp

  Util/Length.hpp
  Util/Property.hpp
  Util/ReadOnlyProperty.hpp

  Main/MainWidgetPrivate.hpp

  Models/AudioEnginePrivate.hpp
  )

# These do NOT need to also appear in libcomposite_HEADERS
LIST(APPEND libcomposite_MOC_HEADERS
  Main/MainWidget.hpp
  Main/MatrixView.hpp
  Main/LibraryView.hpp

  Models/AudioEngine.hpp

  Widgets/Toolbar.hpp
  )

LIST(APPEND libcomposite_SOURCES
  Main/MainWidget.cpp
  Main/MatrixView.cpp
  Main/LibraryView.cpp

  Looks/Colors.cpp
  Looks/Sizes.cpp

  Models/AudioEngine.cpp

  Widgets/Toolbar.cpp
  )

FILE(GLOB_RECURSE libcomposite_EXTRA_HEADERS
  ${CMAKE_SOURCE_DIR}/src/Tritium/Tritium/*
  )

INCLUDE_DIRECTORIES(
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/Tritium
  )

QT4_WRAP_CPP(libcomposite_MOC ${libcomposite_MOC_HEADERS})
CONFIGURE_FILE(composite.qrc.in ${CMAKE_CURRENT_BINARY_DIR}/composite.qrc)
QT4_ADD_RESOURCES(composite_qrc ${CMAKE_CURRENT_BINARY_DIR}/composite.qrc)

ADD_LIBRARY(Composite SHARED
  ${libcomposite_SOURCES}
  ${libcomposite_HEADERS}
  ${libcomposite_MOC_HEADERS}
  ${libcomposite_EXTRA_HEADERS}
  ${libcomposite_MOC}
  ${composite_qrc}
  )

# Depend on the img/ folder
ADD_DEPENDENCIES(Composite
  composite_artwork
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
