
FILE(GLOB_RECURSE composite_UIS src/*.ui)
FILE(GLOB_RECURSE composite_SRCS src/*.cpp)
FILE(GLOB_RECURSE composite_H src/*.h)

QT4_WRAP_UI(composite_UIS_H ${composite_UIS}) # ui_ files are generated in place => ugly
QT4_WRAP_CPP(composite_MOC ${composite_H} OPTIONS -nw) 

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/libs/hydrogen/include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR})

ADD_EXECUTABLE(composite-gui ${composite_SRCS} ${composite_MOC} ${composite_UIS_H} ${CMAKE_BINARY_DIR}/version.cpp)
TARGET_LINK_LIBRARIES(composite-gui tritium
    ${LIBTAR_LIBRARIES}
    ${QT_LIBRARIES}
    ${LIBSNDFILE_LIBRARIES}
    ${ALSA_LIBRARIES}
    ${JACK_LIBRARIES}
    ${PORTAUDIO_LIBRARIES}
    ${LASH_LIBRARIES}
    ${FLAC_LIBRARIES}
    ${FLACPP_LIBRARIES}
    ${LRDF_LIBRARIES}
    )

INSTALL(TARGETS composite-gui RUNTIME DESTINATION bin)
