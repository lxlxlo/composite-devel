
FILE(GLOB_RECURSE composite_UIS *.ui)
FILE(GLOB_RECURSE composite_SRCS *.cpp)
FILE(GLOB_RECURSE composite_H *.h)

QT4_WRAP_UI(composite_UIS_H ${composite_UIS})
QT4_WRAP_CPP(composite_MOC ${composite_H} OPTIONS -nw) 

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/Tritium
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR})

ADD_EXECUTABLE(composite-gui ${composite_SRCS} ${composite_MOC} ${composite_UIS_H})
TARGET_LINK_LIBRARIES(composite-gui Tritium
    ${QT_LIBRARIES}
    ${LibSndfile_LIBRARIES}
    ${JACK_LIBRARIES}
    ${FLAC_LIBRARIES}
    ${FLAC++_LIBRARIES}
    ${LRDF_LIBRARIES}
    ${LibTar_LIBRARIES}
    ${ZLIB_LIBRARIES}
    ${LibArchive_LIBRARIES}
    )

INSTALL(TARGETS composite-gui RUNTIME DESTINATION bin)
