######################################################################
### Composite Build Script (CMake)                                 ###
### http://gabe.is-a-geek.org/composite/                           ###
######################################################################

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

######################################################################
### PROJECT META-INFO                                              ###
######################################################################

PROJECT(composite)
SET(VERSION_MAJOR "0")
SET(VERSION_MINOR "003")
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

# XXX TODO!!

# OPTION(WANT_DEBUG	"build with debug information" OFF)
OPTION(WANT_LRDF	"Include LRDF (Lightweight Resource Description Framework with special support for LADSPA plugins) support" OFF)

OPTION(WANT_LIBARCHIVE
	"Enable use of libarchive instead of libtar"
	OFF)


######################################################################
### REQUIRED LIBRARIES                                             ###
######################################################################

###
### Qt 4 http://qt.nokia.com/
###

SET(QT_USE_QTNETWORK true)
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
set(LIBS ${FLAC_LIBRARIES})
add_definitions(-DFLAC_SUPPORT)

find_package(FLAC++ REQUIRED)
include_directories(${FLAC++_INCLUDE_DIRS})
set(LIBS ${FLAC++_LIBRARIES})
add_definitions(-DFLAC_SUPPORT)

###
### libsndfile http://www.mega-nerd.com/libsndfile/
###
find_package(LibSndfile REQUIRED)
include_directories(${LibSndfile_INCLUDE_DIRS})
set(LIBS ${LibSndfile_LIBRARIES})

### ...................................................
### Either libtar and libz, or libarchive are required.
### ...................................................

###
### libtar http://www.feep.net/libtar/
### Zlib http://www.zlib.net/
###
find_package(LibTar)
find_package(ZLIB)
IF(LibTar_FOUND AND ZLIB_FOUND)
	include_directories(${LibTar_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS})
	set(LIBS ${LibSndfile_LIBRARIES} ${ZLIB_INCLUDE_DIRS})
ENDIF()

###
### libarchive http://code.google.com/p/libarchive/
###
IF(LibTar_FOUND AND ZLIB_FOUND)
ELSE()
       find_package(LibArchive REQUIRED)
       include_directories(${LibArchive_INCLUDE_DIRS})
       set(LIBS ${LibArchive_LIBRARIES})
       add_definitions(-DLIBARCHIVE_SUPPORT)
ENDIF()  

######################################################################
### OPTIONAL LIBRARIES                                             ###
######################################################################

IF(WANT_LRDF)
  find_package(LRDF REQUIRED)
  include_directories(${LRDF_INCLUDE_DIRS})
  set(LIBS ${LRDF_LIBRARIES})
  add_definitions(-DLRDF_SUPPORT)
ENDIF(WANT_LRDF)

######################################################################
### HEADER CHECKS                                                  ###
######################################################################

#INCLUDE(CheckIncludeFiles)
#INCLUDE(CheckLibraryExists)
#CHECK_INCLUDE_FILES(zlib.h HAVE_ZLIB_H)
#CHECK_LIBRARY_EXISTS (tar tar_open TODO HAVE_LIBTAR)            # TODO
#CHECK_INCLUDE_FILES(time.h HAVE_TIME_H)
#
#IF (HAVE_LIBTAR)
#    SET(LIBTAR_LIBRARIES tar)
#ENDIF(HAVE_LIBTAR)

######################################################################
### INSTALL SETUP                                                  ###
######################################################################

SET(DATA_PATH "${CMAKE_INSTALL_PREFIX}/share/composite/data")

######################################################################
### CONFIGURATION SUMMARY                                          ###
######################################################################

MESSAGE("\n"
"Installation Summary\n"
"--------------------\n"
"* Install Directory           : ${CMAKE_INSTALL_PREFIX}\n"
"* Data path                   : ${DATA_PATH}\n"
)

MACRO(lib_report name)
  IF(${name}_FOUND)
    message("FOUND..... ${name}")
  ELSE()
    message("missing... ${name}")
  ENDIF()
ENDMACRO(lib_report)

MESSAGE(
"LIBRARIES\n"
"---------\n"
)
lib_report(QT4)
lib_report(JACK)
lib_report(LibSndfile)
lib_report(LRDF)
lib_report(FLAC)
lib_report(FLAC++)
lib_report(LibTar)
lib_report(ZLIB)
lib_report(LibArchive)

MESSAGE(
"\n"
"-----------------------------------------------------------------\n"
"IMPORTANT:\n"
"after installing missing packages, remove CMakeCache.txt before\n"
"running cmake again!\n"
"-----------------------------------------------------------------\n"
)

######################################################################
### CONFIGURING THE SOFTWARE....                                   ###
######################################################################

CONFIGURE_FILE(config.h.in config.h)
CONFIGURE_FILE(version.cpp.in version.cpp)
CONFIGURE_FILE(composite.desktop.in composite.desktop)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

ADD_SUBDIRECTORY(libs/Tritium)
ADD_SUBDIRECTORY(gui)

INSTALL(DIRECTORY data DESTINATION ${DATA_PATH}/..)
INSTALL(FILES ${CMAKE_BINARY_DIR}/composite.desktop DESTINATION "${CMAKE_INSTALL_PREFIX}/share/applications")

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
