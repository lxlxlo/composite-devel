# FindFLAC
# Try to find libFLAC
#
# Once found, will define:
#
#    FLAC_FOUND
#    FLAC_INCLUDE_DIRS
#    FLAC_LIBRARIES
#

INCLUDE(TritiumPackageHelper)

TPH_FIND_PACKAGE(FLAC flac FLAC/stream_encoder.h FLAC)

### FLAC erroneously ships with flac.pc, and it reports the
### include directory as /usr/include/FLAC.  This usually
### shows up when /usr/include/FLAC/assert.h causes compiler
### probleams.
SET(_FLAC_NEW_LIST "")
FOREACH(J ${FLAC_INCLUDE_DIRS})
  STRING(REGEX REPLACE "/FLAC(\\+\\+)?" "" Z ${J})
  SET(_FLAC_NEW_LIST ${_FLAC_NEW_LIST} ${Z})
ENDFOREACH(J ${FLAC_INCLUDE_DIRS})
SET(FLAC_INCLUDE_DIRS ${_FLAC_NEW_LIST})

INCLUDE(TritiumFindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(FLAC DEFAULT_MSG FLAC_LIBRARIES FLAC_INCLUDE_DIRS)

MARK_AS_ADVANCED(FLAC_INCLUDE_DIRS FLAC_LIBRARIES)

