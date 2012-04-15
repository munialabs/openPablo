
include_directories (${EXTRA_INCDIR} ${GTHREAD_INCLUDE_DIRS} ${GOBJECT_INCLUDE_DIRS} ${GLIB2_INCLUDE_DIRS}
    ${GLIBMM_INCLUDE_DIRS} ${IPTCDATA_INCLUDE_DIRS} ${LCMS_INCLUDE_DIRS}
    ${GTKMM_INCLUDE_DIRS} ${GTK_INCLUDE_DIRS})
link_directories (${EXTRA_LIBDIR} ${GTHREAD_LIBRARY_DIRS}
    ${GOBJECT_LIBRARY_DIRS} ${GLIB2_LIBRARY_DIRS} ${GLIBMM_LIBRARY_DIRS}
    ${IPTCDATA_LIBRARY_DIRS} ${LCMS_LIBRARY_DIRS})


# --- add subdirectories
#

  include_directories(tools)
  add_subdirectory(tools)
  include_directories(engines)
  add_subdirectory(engines)
  include_directories(processors)
  add_subdirectory(processors)

  set (openpablo_libs tools engines processors)
  

#RT
# check for libraries
# 	      find_package(PkgConfig)
# 	      pkg_check_modules (GTK     REQUIRED gtk+-2.0>=2.12)
# 	      pkg_check_modules (GLIB2   REQUIRED glib-2.0>=2.16)
# 	      pkg_check_modules (GLIBMM  REQUIRED glibmm-2.4>=2.16)
# 	      pkg_check_modules (GTKMM   REQUIRED gtkmm-2.4>=2.12)
# 	      pkg_check_modules (GIO     REQUIRED gio-2.0>=2.16)
# 	      pkg_check_modules (GIOMM   REQUIRED giomm-2.4>=2.12)
# 	      pkg_check_modules (GTHREAD REQUIRED gthread-2.0>=2.16)
# 	      pkg_check_modules (GOBJECT REQUIRED gobject-2.0>=2.16)
# 	      pkg_check_modules (SIGC    REQUIRED sigc++-2.0)

# include_directories(${GTKMM_INCLUDE_DIRS} )
#   link_directories (${EXTRA_LIBDIR} ${GLIB2_LIBRARY_DIRS} ${GLIBMM_LIBRARY_DIRS}
#         ${GTK_LIBRARY_DIRS} ${GTKMM_LIBRARY_DIRS} ${GIO_LIBRARY_DIRS} ${GIOMM_LIBRARY_DIRS} ${IPTCDATA_LIBRARY_DIRS} ${LCMS_LIBRARY_DIRS} ${GTHREAD_LIBRARY_DIRS} ${GOBJECT_LIBRARY_DIRS})
# message(INFO "########################")
# message(INFO ${GLIBMM_INCLUDE_DIRS})
# message(INFO "########################")

#RT


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
#    TARGET_LINK_LIBRARIES(openpablo engines processors ${common_libs})
#    set_target_properties(openpablo PROPERTIES VERSION ${openpablo_LIB_VERSION})
ENDIF (${OPENPABLO_SHARED_LIBS})



# --- add openpablo executable
#

add_executable (openpablo main.cpp) # add config.h

set(LINK_FLAGS "${LINK_FLAGS} -mwindows -mdebug -mconsole")
set(EXTRA_LINK_LIBS ${RawTherapeeEngine_LIBRARY})



 include_directories (${EXTRA_INCDIR} ${GLIB2_INCLUDE_DIRS} ${GLIBMM_INCLUDE_DIRS}
        ${GTK_INCLUDE_DIRS} ${GTKMM_INCLUDE_DIRS} ${GIO_INCLUDE_DIRS} ${GIOMM_INCLUDE_DIRS} ${IPTCDATA_INCLUDE_DIRS} ${LCMS_INCLUDE_DIRS} ${GTHREAD_INCLUDE_DIRS} ${GOBJECT_INCLUDE_DIRS} )
    link_directories (${EXTRA_LIBDIR} ${GLIB2_LIBRARY_DIRS} ${GLIBMM_LIBRARY_DIRS}
        ${GTK_LIBRARY_DIRS} ${GTKMM_LIBRARY_DIRS} ${GIO_LIBRARY_DIRS} ${GIOMM_LIBRARY_DIRS} ${IPTCDATA_LIBRARY_DIRS} ${LCMS_LIBRARY_DIRS} ${GTHREAD_LIBRARY_DIRS} ${GOBJECT_LIBRARY_DIRS})


 include_directories (${GTHREAD_INCLUDE_DIRS} ${GOBJECT_INCLUDE_DIRS} ${GLIB2_INCLUDE_DIRS}
     ${GLIBMM_INCLUDE_DIRS} ${IPTCDATA_INCLUDE_DIRS} ${LCMS_INCLUDE_DIRS}
     ${GTKMM_INCLUDE_DIRS} ${GTK_INCLUDE_DIRS})


target_link_libraries(openpablo ${EXTRA_LINK_LIBS}  ${EXTRA_LIBDIR} ${openpablo_libs} 
  ${LOGOG_LIBRARY} ${LIBRAW_LIBRARIES} ${QT_LIBRARIES}  ${LibMagic_LIBRARY} 
  ${LENSFUN_LIBRARIES} ${OpenCV_LIBS} 
  ${LIBPODOFO_LIBRARY} ${ImageMagick_LIBRARIES} ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY} ${EXIV2_LIBRARIES})



# --- install openpablo
#

#INSTALL(TARGETS celeste_standalone DESTINATION ${BINDIR}) 
#INSTALL(FILES ${CELESTE_MODEL} DESTINATION ${INSTALL_DATA_DIR})

