######################################################################
### Composite Build Script (CMake)                                 ###
### http://gabe.is-a-geek.org/composite/                           ###
######################################################################

CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

######################################################################
### PROJECT META-INFO                                              ###
######################################################################

PROJECT(composite)
SET(VERSION_MAJOR "0")
SET(VERSION_MINOR "009")
#SET(VERSION_PATCH "")
SET(VERSION "${VERSION_MAJOR}.${VERSION_MINOR}")

######################################################################
### CMAKE SETUP                                                    ###
######################################################################

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules")

# The CMake Policy mechanism is designed to help keep existing
# projects building as new versions of CMake introduce changes in
# behavior.
# http://www.cmake.org/cmake/help/cmake2.6docs.html#command:cmake_policy
IF(COMMAND CMAKE_POLICY)
#	CMAKE_POLICY(SET CMP0005 NEW)
	CMAKE_POLICY(SET CMP0003 NEW)   # add_library
ENDIF(COMMAND CMAKE_POLICY)

######################################################################
### CONFIGURATION OPTIONS                                          ###
######################################################################

OPTION(WANT_LRDF
  "Include LRDF (Lightweight Resource Description Framework with special support for LADSPA plugins) support <http://sourceforge.net/projects/lrdf/>"
  OFF
  )

OPTION(WANT_LIBARCHIVE
  "Enable use of libarchive instead of libtar and libz.  This is now the default."
  ON )


######################################################################
### REQUIRED LIBRARIES                                             ###
######################################################################

### See src/Tritium/CMakeLists.txt
### See src/composite-gui/CMakeLists.txt

######################################################################
### BUILD/INSTALL SETUP                                            ###
######################################################################

SET(DATA_PATH "${CMAKE_INSTALL_PREFIX}/share/composite/data")

######################################################################
### CONFIGURING THE SOFTWARE....                                   ###
######################################################################

CONFIGURE_FILE(config.h.in config.h)
CONFIGURE_FILE(version.cpp.in version.cpp)
CONFIGURE_FILE(composite.desktop.in composite.desktop)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

ADD_SUBDIRECTORY(src)

ENABLE_TESTING()

INSTALL(DIRECTORY data DESTINATION ${DATA_PATH}/..)
INSTALL(FILES ${CMAKE_BINARY_DIR}/composite.desktop DESTINATION "${CMAKE_INSTALL_PREFIX}/share/applications")
INSTALL(FILES ${CMAKE_SOURCE_DIR}/data/doc/composite_sampler.1
        DESTINATION share/man/man1)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/data/doc/composite_midi.7
        DESTINATION share/man/man7)
INCLUDE(InstallRequiredSystemLibraries)

######################################################################
### UTILS FOR MAKING OFFICIAL RELEASES                             ###
######################################################################

# Note that this assumes a Git archive, and packages
# Git HEAD.
ADD_CUSTOM_TARGET(dist
  git archive --format=tar --prefix=composite-${VERSION}/ HEAD
     | gzip -9
     > ${CMAKE_BINARY_DIR}/composite_${VERSION}.orig.tar.gz
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  COMMENT "Making composite_${VERSION}.orig.tar.gz"
  )

######################################################################
### CONFIGURATION SUMMARY                                          ###
######################################################################

MESSAGE("\n"
"Overall Configuration Summary\n"
"-----------------------------\n"
"* Install Directory           : ${CMAKE_INSTALL_PREFIX}\n"
"* Data path                   : ${DATA_PATH}\n"
)

MESSAGE(
"\n"
"-----------------------------------------------------------------\n"
"FOR ADVANCED CONFIGURATION, USE ccmake INSTEAD OF cmake\n"
"\n"
"IMPORTANT:\n"
"after installing missing packages, remove CMakeCache.txt before\n"
"running cmake again!\n"
"-----------------------------------------------------------------\n"
)

