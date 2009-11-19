CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(composite)

SET(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})

# The CMake Policy mechanism is designed to help keep existing projects building as new versions of CMake introduce changes in behavior.
# http://www.cmake.org/cmake/help/cmake2.6docs.html#command:cmake_policy
IF(COMMAND CMAKE_POLICY)
#	CMAKE_POLICY(SET CMP0005 NEW)
	CMAKE_POLICY(SET CMP0003 NEW)   # add_library
ENDIF(COMMAND CMAKE_POLICY)

SET(MANDATORY_PACKAGES_LIST)
# OPTIONS
OPTION(WANT_DEBUG	"build with debug information" OFF)                                             # TODO
OPTION(WANT_LIBARCHIVE	"Enable use of libarchive instead of libtar" OFF)                               # TODO
OPTION(WANT_JACK	"Include JACK (Jack Audio Connection Kit) support" ON)
OPTION(WANT_LRDF	"Include LRDF (Lightweight Resource Description Framework with special support for LADSPA plugins) support" OFF)
OPTION(WANT_FLAC	"Include FLAC (Free Lossless Audio Codec) support" ON)
OPTION(WANT_FLACPP	"Include FLAC++ (Free Lossless Audio Codec) support" ON) # kludge

# DEFAULT OPTIONS
IF(WIN32)
    MESSAGE( "?????")
    SET(WANT_JACK OFF)
    SET(STATUS_JACK "<not supported on this platform>")
ELSE(WIN32)
    IF(APPLE)
        MESSAGE( "?????")
    ELSE(APPLE)
        SET(WANT_LIBSNDFILE ON)
        LIST(APPEND MANDATORY_PACKAGES_LIST LIBSNDFILE)
        EXECUTE_PROCESS(COMMAND svnversion -n OUTPUT_VARIABLE SVN_VERSION )
    ENDIF(APPLE)
ENDIF(WIN32)

# VERSION
SET(VERSION_MAJOR "0")
SET(VERSION_MINOR "003")
#SET(VERSION_PATCH "0")
SET(VERSION "${VERSION_MAJOR}.${VERSION_MINOR}")
#IF(VERSION_SUFFIX)
#    SET(VERSION "${VERSION}-${VERSION_SUFFIX}")
#ENDIF(VERSION_SUFFIX)
#ADD_DEFINITIONS(-D'COMPOSITE_VERSION="${VERSION}"')

# HEADER CHECKS
INCLUDE(CheckIncludeFiles)
INCLUDE(CheckLibraryExists)
CHECK_INCLUDE_FILES(zlib.h HAVE_ZLIB_H)
CHECK_LIBRARY_EXISTS (tar tar_open TODO HAVE_LIBTAR)            # TODO
CHECK_INCLUDE_FILES(time.h HAVE_TIME_H)
CHECK_INCLUDE_FILES(pthread.h HAVE_PTHREAD_H)

IF (HAVE_LIBTAR)
    SET(LIBTAR_LIBRARIES tar)
ENDIF(HAVE_LIBTAR)

# PKG-CONFIG CHECKS
INCLUDE(FindPkgConfig)
MACRO(my_check var_name version to_install)
    IF(WANT_${var_name})
        PKG_SEARCH_MODULE(${var_name} ${version})
        IF(${var_name}_FOUND)
            SET(STATUS_${var_name} "${${var_name}_VERSION}")
            SET(${var_name}_SUPPORT ON)
            ADD_DEFINITIONS(-D${var_name}_SUPPORT)              # COMPOSITE IS ACTUALLY FLAG DRIVEN
        ELSE(${var_name}_FOUND)
            SET(STATUS_${var_name} "not found, please install ${to_install} (or similiar) if you require ${var_name} support")
        ENDIF(${var_name}_FOUND)
    ELSE(WANT_${var_name})
        SET(STATUS_${var_name} "not desired")
    ENDIF(WANT_${var_name})
ENDMACRO(my_check)

#
MY_CHECK(LIBSNDFILE sndfile libsndfile-dev)
MY_CHECK(JACK jack>=0.109.0 libjack-dev)
MY_CHECK(LRDF lrdf liblrdf-dev)
MY_CHECK(FLAC flac libflac-dev)
MY_CHECK(FLACPP flac++ libflac++-dev)

FOREACH( _pkg ${MANDATORY_PACKAGES_LIST})
    if(NOT ${_pkg}_SUPPORT)
        MESSAGE(FATAL_ERROR "${_pkg} is required but was not found")
    ENDIF(NOT ${_pkg}_SUPPORT)
ENDFOREACH(_pkg)

# CHECK FOR Qt4
#SET(QT_MIN_VERSION "4.3.0")
#find_package(Qt4 4.3.0 COMPONENTS QtCore QtGui QtNetwork QtXml REQUIRED )
#include(${QT_USE_FILE})
SET(QT_USE_QTNETWORK true)
SET(QT_USE_QTXML true)
FIND_PACKAGE(Qt4 4.3.0 REQUIRED)
INCLUDE(${QT_USE_FILE})


SET(DATA_PATH "${CMAKE_INSTALL_PREFIX}/share/composite/data")
#
MESSAGE("\n"
"Installation Summary\n"
"--------------------\n"
"* Install Directory           : ${CMAKE_INSTALL_PREFIX}\n"
"* Data path                   : ${DATA_PATH}\n"
"* Qt4 version                 : ${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}.${QT_VERSION_PATCH}\n"
"* libsndfile                  : ${STATUS_LIBSNDFILE}\n"
)

MESSAGE(
"Supported audio interfaces\n"
"--------------------------\n"
"* JACK                        : ${STATUS_JACK}\n"
)

MESSAGE(
"Usefull extensions\n"
"-----------------------------------------\n"
"* LRDF                        : ${STATUS_LRDF}\n"
)

MESSAGE(
"Supported file formats for samples\n"
"-----------------------------------------\n"
"* WAVE                        : builtin\n"
"* FLAC                        : ${STATUS_FLAC}\n"
"* FLAC++                      : ${STATUS_FLACPP}\n"
)

MESSAGE(
"\n"
"-----------------------------------------------------------------\n"
"IMPORTANT:\n"
"after installing missing packages, remove CMakeCache.txt before\n"
"running cmake again!\n"
"-----------------------------------------------------------------\n"
)

CONFIGURE_FILE(config.h.in config.h)
CONFIGURE_FILE(version.cpp.in version.cpp)
CONFIGURE_FILE(composite.desktop.in composite.desktop)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

ADD_SUBDIRECTORY(libs/hydrogen)
ADD_SUBDIRECTORY(gui)
#ADD_SUBDIRECTORY(plugins)
#ADD_SUBDIRECTORY(extra)

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

