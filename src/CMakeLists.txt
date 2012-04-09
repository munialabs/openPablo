

# --- add subdirectories
#

  include_directories(processors)
  add_subdirectory(processors)
  set (openpablo_libs processors)


# --- Add files for openpablo main
#

  SET(OPENPABLO_SRC
    main.cpp
  )


# --- compiler flags
  add_definitions("-DHAVE_CONFIG_H")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -fno-strict-aliasing")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl -subsystem,console,debug")

  #if(NOT APPLE)
    # apple build fails on deprecated warnings..
  #  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror")
  #endif(NOT APPLE)

  if(NOT WIN32)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")
  endif(NOT WIN32)



# --- compile lib

IF (${OPENPABLO_SHARED_LIBS})
#     # messages are quite obtrusive in windows cmakesetup.exe
#     IF (NOT WIN32)
#         #message("Using these CMAKE_CXX_FLAGS : ${CMAKE_CXX_FLAGS}")
#     ENDIF (NOT WIN32)
#         ADD_LIBRARY(celeste SHARED ${CELESTE_SRC})
#         TARGET_LINK_LIBRARIES(celeste huginbase ${common_libs})
#         set_target_properties(celeste PROPERTIES VERSION ${HUGIN_LIB_VERSION})
#     IF(WIN32)
#       install(TARGETS celeste RUNTIME DESTINATION ${BINDIR})
#     ELSEIF(${HUGIN_LIBS_PRIVATE_DIR})
#       install(TARGETS celeste LIBRARY DESTINATION ${LIBDIR}/hugin NAMELINK_SKIP)
#     ELSE(WIN32)
#       install(TARGETS celeste LIBRARY DESTINATION ${LIBDIR} NAMELINK_SKIP)
# 
#     IF(WIN32)
#       install(TARGETS celeste RUNTIME DESTINATION ${BINDIR})
#     ELSEIF(${HUGIN_LIBS_PRIVATE_DIR})
#       install(TARGETS celeste LIBRARY DESTINATION ${LIBDIR}/hugin NAMELINK_SKIP)
#     ELSE(WIN32)
#       install(TARGETS celeste LIBRARY DESTINATION ${LIBDIR} NAMELINK_SKIP)
#     ENDIF(WIN32)
ELSE (${OPENPABLO_SHARED_LIBS})
    # messages are quite obtrusive in windows cmakesetup.exe
    IF (NOT WIN32)
        #message("Using these CMAKE_CXX_FLAGS : ${CMAKE_CXX_FLAGS}")
    ENDIF (NOT WIN32)       

#    ADD_LIBRARY(openpablo STATIC ${CELESTE_SRC})
#    TARGET_LINK_LIBRARIES(openpablo processors ${common_libs})
#    set_target_properties(openpablo PROPERTIES VERSION ${openpablo_LIB_VERSION})
ENDIF (${OPENPABLO_SHARED_LIBS})



# --- add openpablo executable
#

add_executable (openpablo main.cpp) # add config.h
set(LINK_FLAGS "${LINK_FLAGS} -mwindows -mdebug -mconsole")
target_link_libraries(openpablo ${openpablo_libs} ${QT_LIBRARIES} ${RawTherapeeEngine_LIBRARY} ${LibMagic_LIBRARY} ${LENSFUN_LIBRARIES} ${OpenCV_LIBS} ${LIBPODOFO_LIBRARY} ${ImageMagick_LIBRARIES} ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY} ${EXIV2_LIBRARIES})



# --- install openpablo
#

#INSTALL(TARGETS celeste_standalone DESTINATION ${BINDIR}) 
#INSTALL(FILES ${CELESTE_MODEL} DESTINATION ${INSTALL_DATA_DIR})

