######################################################################
### Composite Build Script (CMake)                                 ###
### http://gabe.is-a-geek.org/composite/                           ###
######################################################################

CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

######################################################################
### REQUIRED LIBRARIES                                             ###
######################################################################

FIND_PACKAGE(Qt4 4.3.0 REQUIRED)
INCLUDE(${QT_USE_FILE})

######################################################################
### LIBRARY SOURCES AND BUILD                                      ###
######################################################################

LIST(APPEND composite_gui_HEADERS
  ${CMAKE_SOURCE_DIR}/version.h
  ${CMAKE_BINARY_DIR}/config.h
  )

# Note that these should also appear
# in composite_gui_HEADERS
LIST(APPEND composite_gui_MOC_HEADERS
  )

LIST(APPEND composite_gui_SOURCES
  main.cpp
  )

FILE(GLOB_RECURSE composite_gui_EXTRA_HEADERS
  ${CMAKE_SOURCE_DIR}/src/Tritium/Tritium/*
  ${CMAKE_SOURCE_DIR}/src/Composite/Composite/*
  )

QT4_WRAP_CPP(composite_gui_MOC ${composite_gui_MOC_HEADERS}) 

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/Tritium
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR})

ADD_EXECUTABLE(composite-gui
  ${composite_gui_HEADERS}
  ${composite_gui_SOURCES}
  ${composite_gui_MOC}
  ${composite_gui_EXTRA_HEADERS}
  )

TARGET_LINK_LIBRARIES(composite-gui
  Composite
  Tritium
  ${QT_LIBRARIES}
  )

INSTALL(TARGETS composite-gui RUNTIME DESTINATION bin)

######################################################################
### CONFIGURATION SUMMARY                                          ###
######################################################################

MESSAGE("\n"
"Configuration Summary for composite-gui\n"
"---------------------------------------\n"
)

MACRO(lib_report name)
  IF(${name}_FOUND)
    message("ENABLED..... ${name}")
  ELSE(${name}_FOUND)
    message("disabled.... ${name}")
  ENDIF(${name}_FOUND)
ENDMACRO(lib_report)

lib_report(QT4)

