# FindLibTar
# Try to find libLibTar
#
# Once found, will define:
#
#    LibTar_FOUND
#    LibTar_INCLUDE_DIRS
#    LibTar_LIBRARIES
#

INCLUDE(TritiumPackageHelper)

TPH_FIND_PACKAGE(LibTar "" libtar.h tar)

INCLUDE(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(LibTar DEFAULT_MSG LibTar_LIBRARIES LibTar_INCLUDE_DIRS)

MARK_AS_ADVANCED(LibTar_INCLUDE_DIRS LibTar_LIBRARIES)


