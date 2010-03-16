# FindFLAC++
# Try to find libFLAC++
#
# Once found, will define:
#
#    FLAC++_FOUND
#    FLAC++_INCLUDE_DIRS
#    FLAC++_LIBRARIES
#

INCLUDE(TritiumPackageHelper)

TPH_FIND_PACKAGE(FLAC++ flac++ FLAC++/encoder.h FLAC++)

### FLAC erroneously ships with flac.pc, and it reports the
### include directory as /usr/include/FLAC.  This usually
### shows up when /usr/include/FLAC/assert.h causes compiler
### probleams.
SET(_FLAC++_NEW_LIST "")
FOREACH(J ${FLAC++_INCLUDE_DIRS})
  STRING(REGEX REPLACE "/FLAC(\\+\\+)?" "" Z ${J})
  SET(_FLAC++_NEW_LIST ${_FLAC++_NEW_LIST} ${Z})
ENDFOREACH(J ${FLAC++_INCLUDE_DIRS})
SET(FLAC++_INCLUDE_DIRS ${_FLAC++_NEW_LIST})

INCLUDE(TritiumFindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(FLAC++ DEFAULT_MSG FLAC++_LIBRARIES FLAC++_INCLUDE_DIRS)

MARK_AS_ADVANCED(FLAC++_INCLUDE_DIRS FLAC++_LIBRARIES)

