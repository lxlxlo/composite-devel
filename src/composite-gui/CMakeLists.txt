######################################################################
### Composite Build Script (CMake)                                 ###
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
### JACK (JACK support is required for libTritium, so we just
### need to set the flag since it used to be optional.)
###
add_definitions(-DJACK_SUPPORT)

###
### Qt 4 http://qt.nokia.com/
###

SET(QT_USE_QTNETWORK true)
SET(QT_USE_QTXML true)
FIND_PACKAGE(Qt4 4.3.0 REQUIRED)
INCLUDE(${QT_USE_FILE})

###
### Boost Framework http://www.boost.org/
### Used for unit testing
###
find_package(Boost 1.33 COMPONENTS unit_test_framework REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

######################################################################
### LIBRARY SOURCES AND BUILD                                      ###
######################################################################

FILE(GLOB_RECURSE composite_UIS *.ui)
FILE(GLOB_RECURSE composite_SRCS *.cpp)
FILE(GLOB_RECURSE composite_H *.hpp)

QT4_WRAP_UI(composite_UIS_H ${composite_UIS})
QT4_WRAP_CPP(composite_MOC ${composite_H}) 

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/Tritium
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR})

ADD_EXECUTABLE(composite-gui ${composite_SRCS} ${composite_MOC} ${composite_UIS_H})
TARGET_LINK_LIBRARIES(composite-gui
    Tritium
    ${QT_LIBRARIES}
    )

INSTALL(TARGETS composite-gui RUNTIME DESTINATION bin)

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
lib_report(Boost)

