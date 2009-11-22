######################################################################
### Tritium Build Script (CMake)                                   ###
### http://gabe.is-a-geek.org/composite/                           ###
######################################################################
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

######################################################################
### TEST SOURCES AND BUILD                                         ###
######################################################################

IF(Boost_UNIT_TEST_FRAMEWORK_FOUND)
  FILE(GLOB_RECURSE test_SOURCES *.cpp)
  FILE(GLOB_RECURSE test_HEADERS *.h ../Tritium/*.h ../src/*.h)

  ADD_EXECUTABLE(test_Tritium
    ${test_SOURCES}
    ${test_INCLUDES}
    ${tritium_INCLUDES}
    )

  ADD_DEFINITIONS("-DBOOST_TEST_DYN_LINK")

  TARGET_LINK_LIBRARIES(test_Tritium
    Tritium
    ${QT_LIBRARIES}
    ${LibSndfile_LIBRARIES}
    ${JACK_LIBRARIES}
    ${FLAC_LIBRARIES}
    ${FLAC++_LIBRARIES}
    ${LRDF_LIBRARIES}
    ${LibTar_LIBRARIES}
    ${ZLIB_LIBRARIES}
    ${LibArchive_LIBRARIES}
    ${Boost_UNIT_TEST_FRAMEWORK_LIBRARIES}
    )

  ADD_TEST(testlib test_Tritium)

ENDIF(Boost_UNIT_TEST_FRAMEWORK_FOUND)