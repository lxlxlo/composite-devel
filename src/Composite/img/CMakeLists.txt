######################################################################
### Composite Build Script (CMake)                                 ###
### http://gabe.is-a-geek.org/composite/                           ###
######################################################################
CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

######################################################################
### REQUIRED TOOLS                                                 ###
######################################################################

###
### ImageMagick http://www.imagemagick.org/
###
### Need 'convert' to convert SVG's to PNG's and then
### split up the PNG's to individual files.
###

FIND_PACKAGE(ImageMagick COMPONENTS convert REQUIRED)

IF( "${ImageMagick_convert_EXECUTABLE}" STREQUAL "" )
  # I.e. we're using a pre-2.8 CMake version of FindImageMagick
  SET(CONVERT_CMD ${IMAGEMAGICK_CONVERT_EXECUTABLE})
ELSE( "${ImageMagick_convert_EXECUTABLE}" STREQUAL "" )
  SET(CONVERT_CMD ${ImageMagick_convert_EXECUTABLE})
ENDIF( "${ImageMagick_convert_EXECUTABLE}" STREQUAL "" )

IF( CONVERT_CMD STREQUAL "" )
  MESSAGE(FATAL_ERROR "Could not find ImageMagick's `convert` tool")
ENDIF( CONVERT_CMD STREQUAL "" )

MESSAGE( "ImageMagick convert command is: ${CONVERT_CMD}" )

######################################################################
### BUILD RULES                                                    ###
######################################################################

SET(SRCDIR ${CMAKE_CURRENT_SOURCE_DIR})
SET(BINDIR ${CMAKE_CURRENT_BINARY_DIR})

ADD_CUSTOM_TARGET(composite_artwork ALL
  COMMAND touch ${BINDIR}/stamp-composite_artwork
  DEPENDS ${BINDIR}/stamp-composite_icons
  )

######################################################################
### ICON GENERATION                                                ###
###                                                                ###
### All icons are generated from `icons.svg` like so:              ###
###                                                                ###
### icons.svg --> icons.png --+-> icon_play.png                    ###
###                           +-> icon_stop.png                    ###
###                           +-> icon_matrix.png                  ###
###                                                                ###
### The file `icons.svg` is a horizontal strip of 128x128 icons.   ###
######################################################################

ADD_CUSTOM_COMMAND(OUTPUT stamp-composite_icons
  COMMAND touch ${BINDIR}/stamp-composite_icons
  DEPENDS
  icons.svg
  ${BINDIR}/icons.png
  ${BINDIR}/icon_play.png
  ${BINDIR}/icon_stop.png
  ${BINDIR}/icon_matrix.png
  )

ADD_CUSTOM_COMMAND(OUTPUT icons.png
  COMMAND ${CONVERT_CMD} -background none ${SRCDIR}/icons.svg ${BINDIR}/icons.png
  DEPENDS ${SRCDIR}/icons.svg
  )

ADD_CUSTOM_COMMAND(OUTPUT icon_play.png
  COMMAND ${CONVERT_CMD} ${BINDIR}/icons.png -crop 128x128+0+0 ${BINDIR}/icon_play.png
  DEPENDS ${BINDIR}/icons.png
  )

ADD_CUSTOM_COMMAND(OUTPUT icon_stop.png
  COMMAND ${CONVERT_CMD} ${BINDIR}/icons.png -crop 128x128+128+0 ${BINDIR}/icon_stop.png
  DEPENDS ${BINDIR}/icons.png
  )

ADD_CUSTOM_COMMAND(OUTPUT icon_matrix.png
  COMMAND ${CONVERT_CMD} ${BINDIR}/icons.png -crop 128x128+256+0 ${BINDIR}/icon_matrix.png
  DEPENDS ${BINDIR}/icons.png
  )
