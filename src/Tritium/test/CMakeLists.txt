######################################################################
### Tritium Build Script (CMake)                                   ###
### http://gabe.is-a-geek.org/composite/                           ###
######################################################################
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

######################################################################
### TEST SOURCES AND BUILD                                         ###
######################################################################

IF(Boost_UNIT_TEST_FRAMEWORK_FOUND)
  FILE(GLOB_RECURSE test_HEADERS *.h ../Tritium/*.h ../src/*.h)

  LIST(APPEND test_LIST
    t_SeqEvent
    t_SeqScript
    t_SeqScriptPrivate
    t_SimpleTransportMaster
    t_songhelpers
    t_TransportPosition
    )

  ADD_DEFINITIONS("-DBOOST_TEST_DYN_LINK")
  ENABLE_TESTING()

  ADD_LIBRARY(test_utils_Tritium
    test_utils.cpp
    )

  FOREACH(T ${test_LIST})
    ADD_EXECUTABLE(${T}
      ${T}.cpp
      ${test_INCLUDES}
      ${tritium_INCLUDES}
      )
    SET_SOURCE_FILES_PROPERTIES(${T}
      PROPERTIES
      COMPILE_FLAGS "-DBOOST_TEST_MAIN"
      )
    TARGET_LINK_LIBRARIES(${T}
      Tritium
      test_utils_Tritium
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

    ADD_TEST(${T} ${T})
  ENDFOREACH()

ENDIF(Boost_UNIT_TEST_FRAMEWORK_FOUND)
