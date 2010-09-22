######################################################################
### Tritium Build Script (CMake)                                   ###
### http://gabe.is-a-geek.org/composite/                           ###
######################################################################
CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

######################################################################
### REQUIRED LIBRARIES                                             ###
######################################################################

###
### LADSPA
###
add_definitions(-DLADSPA_SUPPORT)

###
### Qt 4 http://qt.nokia.com/
###

SET(QT_USE_QTXML true)
SET(QT_DONT_USE_QTGUI true)
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
set(LIBS ${LIBS} ${FLAC_LIBRARIES})
add_definitions(-DFLAC_SUPPORT)

find_package(FLAC++ REQUIRED)
include_directories(${FLAC++_INCLUDE_DIRS})
set(LIBS ${LIBS} ${FLAC++_LIBRARIES})
add_definitions(-DFLAC_SUPPORT)

###
### libsndfile http://www.mega-nerd.com/libsndfile/
###
find_package(LibSndfile REQUIRED)
include_directories(${LibSndfile_INCLUDE_DIRS})
set(LIBS ${LIBS} ${LibSndfile_LIBRARIES})

### ...................................................
### Either libtar and libz, or libarchive are required.
### ...................................................

###
### libtar http://www.feep.net/libtar/
### Zlib http://www.zlib.net/
###
IF(NOT WANT_LIBARCHIVE)
  find_package(LibTar)
  find_package(ZLIB)
ENDIF(NOT WANT_LIBARCHIVE)
IF(LibTar_FOUND AND ZLIB_FOUND)
	include_directories(${LibTar_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS})
	set(LIBS ${LIBS} ${LibTar_LIBRARIES} ${ZLIB_LIBRARIES})
ENDIF(LibTar_FOUND AND ZLIB_FOUND)

###
### libarchive http://code.google.com/p/libarchive/
###
IF(LibTar_FOUND AND ZLIB_FOUND)
ELSE(LibTar_FOUND AND ZLIB_FOUND)
       find_package(LibArchive REQUIRED)
       include_directories(${LibArchive_INCLUDE_DIRS})
       set(LIBS ${LIBS} ${LibArchive_LIBRARIES})
       add_definitions(-DLIBARCHIVE_SUPPORT)
ENDIF(LibTar_FOUND AND ZLIB_FOUND)

###
### Boost Framework http://www.boost.org/
### Used for unit testing
###
find_package(Boost 1.33 COMPONENTS unit_test_framework REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
IF(${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} LESS 2.6)
	# CMake 2.4 didn't support module detection.  Turn it on
	# and hope for the best.  Don't know about 2.5.
	set(Boost_UNIT_TEST_FRAMEWORK_FOUND ON)
	set(Boost_UNIT_TEST_FRAMEWORK_LIBRARIES -lboost_unit_test_framework)
ENDIF(${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} LESS 2.6)

######################################################################
### OPTIONAL LIBRARIES                                             ###
######################################################################

IF(WANT_LRDF)
  find_package(LRDF REQUIRED)
  include_directories(${LRDF_INCLUDE_DIRS})
  set(LIBS ${LIBS} ${LRDF_LIBRARIES})
  add_definitions(-DLRDF_SUPPORT)
ENDIF(WANT_LRDF)

######################################################################
### LIBRARY SOURCES AND BUILD                                      ###
######################################################################

LIST(APPEND tritium_SOURCES
  ${CMAKE_BINARY_DIR}/version.cpp
  src/ADSR.cpp
  src/Action.cpp
  src/AudioPortImpl.cpp
  src/BeatCounter.cpp
  src/DataPath.cpp
  src/DefaultMidiImplementation.cpp
  src/Engine.cpp
  src/EventQueue.cpp
  src/FLACFile.cpp
  src/IO/DiskWriterDriver.cpp
  src/IO/FakeDriver.cpp
  src/IO/JackClient.cpp
  src/IO/JackMidiDriver.cpp
  src/IO/JackOutput.cpp
  src/IO/MidiInput.cpp
  src/IO/NullDriver.cpp
  src/Instrument.cpp
  src/InstrumentLayer.cpp
  src/InstrumentList.cpp
  src/LocalFileMng.cpp
  src/Logger.cpp
  src/MidiMap.cpp
  src/MixerImpl.cpp
  src/Note.cpp
  src/Pattern.cpp
  src/Playlist.cpp
  src/Preferences.cpp
  src/Presets.cpp
  src/SampleBank.cpp
  src/Sampler.cpp
  src/SeqEvent.cpp
  src/SeqScript.cpp
  src/SeqScriptIterator.cpp
  src/SeqScriptPrivate.cpp
  src/Serialization.cpp
  src/SimpleSample.cpp
  src/SimpleStereoSample.cpp
  src/Song.cpp
  src/SongSequencer.cpp
  src/SoundLibrary.cpp
  src/TritiumXml.cpp
  src/WorkerThread.cpp
  src/fx/Effects.cpp
  src/fx/LadspaFX.cpp
  src/smf/SMF.cpp
  src/smf/SMFEvent.cpp
  src/timehelper.cpp
  src/transport/H2Transport.cpp
  src/transport/JackTimeMaster.cpp
  src/transport/JackTransportMaster.cpp
  src/transport/SimpleTransportMaster.cpp
  src/transport/TransportPosition.cpp
  src/util.cpp
  )

LIST(APPEND tritium_INCLUDES
  ${CMAKE_SOURCE_DIR}/version.h
  ${CMAKE_SOURCE_DIR}/config.h
  Tritium/ADSR.hpp
  Tritium/Action.hpp
  Tritium/AudioPort.hpp
  Tritium/AudioPortManager.hpp
  Tritium/DataPath.hpp
  Tritium/DefaultMidiImplementation.hpp
  Tritium/Engine.hpp
  Tritium/EngineInterface.hpp
  Tritium/EventQueue.hpp
  Tritium/H2Exception.hpp
  Tritium/IO/AudioOutput.hpp
  Tritium/IO/JackOutput.hpp
  Tritium/IO/MidiInput.hpp
  Tritium/IO/NullDriver.hpp
  Tritium/Instrument.hpp
  Tritium/InstrumentLayer.hpp
  Tritium/InstrumentList.hpp
  Tritium/JackTimeMasterEvents.hpp
  Tritium/LocalFileMng.hpp
  Tritium/Logger.hpp
  Tritium/MidiImplementation.hpp
  Tritium/MidiImplementationBase.hpp
  Tritium/MidiMap.hpp
  Tritium/Mixer.hpp
  Tritium/MixerImpl.hpp
  Tritium/Note.hpp
  Tritium/ObjectBundle.hpp
  Tritium/Pattern.hpp
  Tritium/Playlist.hpp
  Tritium/Preferences.hpp
  Tritium/Presets.hpp
  Tritium/RingBuffer.hpp
  Tritium/RingBufferNPT.hpp
  Tritium/Sample.hpp
  Tritium/SampleBank.hpp
  Tritium/Sampler.hpp
  Tritium/SeqEvent.hpp
  Tritium/SeqScript.hpp
  Tritium/SeqScriptIterator.hpp
  Tritium/Serialization.hpp
  Tritium/SimpleSample.hpp
  Tritium/SimpleStereoSample.hpp
  Tritium/Song.hpp
  Tritium/SoundLibrary.hpp
  Tritium/Transport.hpp
  Tritium/TransportPosition.hpp
  Tritium/fx/Effects.hpp
  Tritium/fx/LadspaFX.hpp
  Tritium/fx/ladspa.h
  Tritium/globals.hpp
  Tritium/memory.hpp
  Tritium/smf/SMF.hpp
  Tritium/smf/SMFEvent.hpp
  Tritium/timehelper.hpp
  Tritium/util.hpp
  )

LIST(APPEND tritium_src_INCLUDES
  src/AudioPortImpl.hpp
  src/BeatCounter.hpp
  src/EnginePrivate.hpp
  src/FLACFile.hpp
  src/IO/DiskWriterDriver.hpp
  src/IO/FakeDriver.hpp
  src/IO/JackClient.hpp
  src/IO/JackMidiDriver.hpp
  src/InstrumentPrivate.hpp
  src/LoggerPrivate.hpp
  src/MixerImplPrivate.hpp
  src/PatternModeList.hpp
  src/PatternModeManager.hpp
  src/SamplerPrivate.hpp
  src/SeqScriptPrivate.hpp
  src/SerializationPrivate.hpp
  src/SongPrivate.hpp
  src/SongSequencer.hpp
  src/TritiumXml.hpp
  src/WorkerThread.hpp
  src/table/TableExponential.hpp
  src/table/getTableValue.hpp
  src/transport/H2Transport.hpp
  src/transport/JackTimeMaster.hpp
  src/transport/JackTransportMaster.hpp
  src/transport/SimpleTransportMaster.hpp
  )

INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR} .)
ADD_LIBRARY(Tritium SHARED
  ${tritium_SOURCES}
  ${tritium_INCLUDES}
  ${tritium_src_INCLUDES}
  )

TARGET_LINK_LIBRARIES(Tritium
  ${QT_LIBRARIES}
  ${LIBS}
  )

SET_TARGET_PROPERTIES(Tritium
  PROPERTIES
  LINK_INTERFACE_LIBRARIES ""
  )

INSTALL(TARGETS Tritium LIBRARY DESTINATION lib)

######################################################################
### CONFIGURATION SUMMARY                                          ###
######################################################################

MESSAGE("\n"
"Configuration Summary for Tritium\n"
"---------------------------------\n"
)

MACRO(lib_report name)
  IF(${name}_FOUND)
    message("ENABLED..... ${name}")
  ELSE(${name}_FOUND)
    message("disabled.... ${name}")
  ENDIF(${name}_FOUND)
ENDMACRO(lib_report)

lib_report(QT4)
lib_report(JACK)
lib_report(LibSndfile)
lib_report(LRDF)
lib_report(FLAC)
lib_report(FLAC++)
lib_report(LibTar)
lib_report(ZLIB)
lib_report(LibArchive)
lib_report(Boost)
lib_report(Boost_UNIT_TEST_FRAMEWORK)

######################################################################
### UNIT TESTS                                                     ###
######################################################################

ADD_SUBDIRECTORY(test)
ENABLE_TESTING()

