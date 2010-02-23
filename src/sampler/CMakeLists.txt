######################################################################
### Composite Build Script (CMake)                                 ###
### http://gabe.is-a-geek.org/composite/                           ###
######################################################################

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

######################################################################
### REQUIRED LIBRARIES                                             ###
######################################################################

FIND_PACKAGE(Qt4 4.3.0 COMPONENTS QtCore REQUIRED)
INCLUDE(${QT_USE_FILE})

######################################################################
### SOURCES AND BUILD                                              ###
######################################################################

LIST(APPEND LV2_SOURCES
  EngineLv2.hpp
  EngineLv2.cpp
  )

LIST(APPEND LV2_METADATA
  manifest.ttl
  composite_sampler.ttl
  )

INCLUDE(
  ${QT_USE_FILE}
)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/src/Tritium
  ${CMAKE_SOURCE_DIR}/src/3rd-party/lv2
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  )

ADD_LIBRARY(composite_sampler MODULE
  ${LV2_SOURCES}
  )

TARGET_LINK_LIBRARIES(composite_sampler
  Tritium
  ${QT_LIBRARIES}
  )

SET_TARGET_PROPERTIES(composite_sampler
  PROPERTIES
  PREFIX ""
  )

INSTALL(TARGETS composite_sampler LIBRARY DESTINATION lib/lv2/composite_sampler.lv2)
INSTALL(FILES ${LV2_METADATA} DESTINATION lib/lv2/composite_sampler.lv2)
