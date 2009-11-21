# FindLRDF
# Try to find libLRDF
#
# Once found, will define:
#
#    LRDF_FOUND
#    LRDF_INCLUDE_DIRS
#    LRDF_LIBRARIES
#

INCLUDE(TritiumPackageHelper)

TPH_FIND_PACKAGE(LRDF lrdf lrdf.h lrdf)

INCLUDE(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(LRDF DEFAULT_MSG LRDF_LIBRARIES LRDF_INCLUDE_DIRS)

MARK_AS_ADVANCED(LRDF_INCLUDE_DIRS LRDF_LIBRARIES)

