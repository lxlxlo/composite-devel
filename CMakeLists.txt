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
SET(VERSION_MINOR "007")
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

SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Composite - Live performance sequencing, sampling, and looping")
SET(CPACK_PACKAGE_VENDOR "Composite Developers")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/README.txt")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/COPYING")

SET(CPACK_PACKAGE_VERSION_MAJOR "${VERSION_MAJOR}")
SET(CPACK_PACKAGE_VERSION_MINOR "${VERSION_MINOR}")
SET(CPACK_PACKAGE_VERSION_PATCH "${VERSION_PATCH}")
IF(VERSION_SUFFIX)
    SET(CPACK_PACKAGE_VERSION_PATCH "${VERSION_PATCH}-${VERSION_SUFFIX}")
ENDIF(VERSION_SUFFIX)

#SET(CPACK_PACKAGE_INSTALL_DIRECTORY "LMMS ${VERSION}")
#IF(WIN32)
    #SET(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/data\\\\nsis_branding.bmp")
    #SET(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/data\\\\lmms.ico")
    #SET(CPACK_NSIS_INSTALLED_ICON_NAME "lmms.exe")
    #SET(CPACK_NSIS_DISPLAY_NAME "Linux MultiMedia Studio (LMMS)")
    #SET(CPACK_NSIS_HELP_LINK "http:\\\\\\\\lmms.sourceforge.net")
    #SET(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\lmms.sourceforge.net")
    #SET(CPACK_NSIS_CONTACT "lmms-devel@lists.sourceforge.net")
    #SET(CPACK_PACKAGE_EXECUTABLES "lmms.exe;Linux MultiMedia Studio")
    #SET(CPACK_NSIS_MENU_LINKS "lmms.exe;Linux MultiMedia Studio")
#ELSE(WIN32)
#    SET(CPACK_STRIP_FILES "bin/lmms;${PLUGIN_DIR}/*.so")
#    SET(CPACK_PACKAGE_EXECUTABLES "composite-gui" "Composite binary")
#ENDIF(WIN32)

SET(MACOSX_BUNDLE_ICON_FILE "${CMAKE_SOURCE_DIR}/data/img/gray/icon.svg")
SET(MACOSX_BUNDLE_GUI_IDENTIFIER "Composite")
SET(MACOSX_BUNDLE_LONG_VERSION_STRING "${VERSION}")
SET(MACOSX_BUNDLE_BUNDLE_NAME "Composite Sequencer")
SET(MACOSX_BUNDLE_SHORT_VERSION_STRING "${VERSION}")
SET(MACOSX_BUNDLE_BUNDLE_VERSION "${VERSION}")
SET(MACOSX_BUNDLE_COPYRIGHT "Jérémy Zurcher, 2009")

#SET(CPACK_SOURCE_GENERATOR "TBZ2")
#SET(CPACK_SOURCE_PACKAGE_FILE_NAME "composite-${VERSION}")
#INCLUDE(CPack)

add_custom_target(dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)

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

