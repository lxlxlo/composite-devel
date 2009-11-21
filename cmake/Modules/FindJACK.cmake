# - Try to find jack-2.6
# Once done this will define
#
#  JACK_FOUND - system has jack
#  JACK_INCLUDE_DIRS - the jack include directory
#  JACK_LIBRARIES - Link these to use jack
#

INCLUDE(TritiumPackageHelper)

TPH_FIND_PACKAGE(JACK jack jack/midiport.h jack)

INCLUDE(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(FLAC DEFAULT_MSG FLAC_LIBRARIES FLAC_INCLUDE_DIRS)

MARK_AS_ADVANCED(FLAC_INCLUDE_DIRS FLAC_LIBRARIES)
