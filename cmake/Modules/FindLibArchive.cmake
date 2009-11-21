# FindLibArchive
# Try to find libLibArchive
#
# Once found, will define:
#
#    LibArchive_FOUND
#    LibArchive_INCLUDE_DIRS
#    LibArchive_LIBRARIES
#

INCLUDE(TritiumPackageHelper)

TPH_FIND_PACKAGE(LibArchive "" archive.h archive)

INCLUDE(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(LibArchive DEFAULT_MSG LibArchive_LIBRARIES LibArchive_INCLUDE_DIRS)

MARK_AS_ADVANCED(LibArchive_INCLUDE_DIRS LibArchive_LIBRARIES)

