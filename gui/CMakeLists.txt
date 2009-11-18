
FILE(GLOB_RECURSE hydrogen_UIS src/*.ui)
FILE(GLOB_RECURSE hydrogen_SRCS src/*.cpp)
FILE(GLOB_RECURSE hydrogen_H src/*.h)

QT4_WRAP_UI(hydrogen_UIS_H ${hydrogen_UIS}) # ui_ files are generated in place => ugly
QT4_WRAP_CPP(hydrogen_MOC ${hydrogen_H} OPTIONS -nw) 

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/libs/hydrogen/include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src)

ADD_EXECUTABLE(hydrogen-gui ${hydrogen_SRCS} ${hydrogen_MOC} ${hydrogen_UIS_H})
TARGET_LINK_LIBRARIES(hydrogen-gui hydrogen
    ${LIBTAR_LIBRARIES}
    ${QT_LIBRARIES}
    ${LIBSNDFILE_LIBRARIES}
    ${ALSA_LIBRARIES}
    ${JACK_LIBRARIES}
    ${PORTAUDIO_LIBRARIES}
    ${LASH_LIBRARIES}
    ${FLAC_LIBRARIES}
    ${LRDF_LIBRARIES}
    )

INSTALL(TARGETS hydrogen-gui RUNTIME DESTINATION bin)
