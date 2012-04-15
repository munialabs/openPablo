include_directories (${EXTRA_INCDIR} ${GTHREAD_INCLUDE_DIRS} ${GOBJECT_INCLUDE_DIRS} ${GLIB2_INCLUDE_DIRS}
    ${GLIBMM_INCLUDE_DIRS} ${IPTCDATA_INCLUDE_DIRS} ${LCMS_INCLUDE_DIRS}
    ${GTKMM_INCLUDE_DIRS} ${GTK_INCLUDE_DIRS})
link_directories (${CMAKE_CURRENT_SOURCE_DIR}/../rtexif ${EXTRA_LIBDIR} ${GTHREAD_LIBRARY_DIRS}
    ${GOBJECT_LIBRARY_DIRS} ${GLIB2_LIBRARY_DIRS} ${GLIBMM_LIBRARY_DIRS}
    ${IPTCDATA_LIBRARY_DIRS} ${LCMS_LIBRARY_DIRS})

set (RTENGINESOURCEFILES safegtk.cc colortemp.cc curves.cc flatcurves.cc diagonalcurves.cc dcraw.cc iccstore.cc
    dfmanager.cc ffmanager.cc rawimage.cc image8.cc image16.cc imagefloat.cc imagedata.cc imageio.cc improcfun.cc init.cc dcrop.cc
    loadinitial.cc procparams.cc rawimagesource.cc demosaic_algos.cc shmap.cc simpleprocess.cc refreshmap.cc
    stdimagesource.cc myfile.cc iccjpeg.cc hlmultipliers.cc improccoordinator.cc
    processingjob.cc rtthumbnail.cc utils.cc labimage.cc slicer.cc
    iplab2rgb.cc ipsharpen.cc iptransform.cc ipresize.cc ipvibrance.cc
	jpeg_memsrc.cc jdatasrc.cc paramsedited.cc options.cc multilangmgr.cc guiutils.cc rtimage.cc
	PF_correct_RT.cc
    dirpyrLab_denoise.cc dirpyrLab_equalizer.cc dirpyr_equalizer.cc
    calc_distort.cc dcp.cc
    klt/convolve.cc klt/error.cc klt/klt.cc klt/klt_util.cc klt/pnmio.cc klt/pyramid.cc klt/selectGoodFeatures.cc
    klt/storeFeatures.cc klt/trackFeatures.cc klt/writeFeatures.cc
    )







add_library (rtengineH ${RTENGINESOURCEFILES})
#It may be nice to store library version too
IF (BUILD_SHARED_LIBS)
    install (TARGETS rtengineH DESTINATION ${LIBDIR})
ENDIF (BUILD_SHARED_LIBS)

set_target_properties (rtengineH PROPERTIES COMPILE_FLAGS "${RTENGINE_CXX_FLAGS}")

target_link_libraries (rtengineH rtexif ${EXTRA_LIB} ${GOBJECT_LIBRARIES} ${GTHREAD_LIBRARIES}
    ${GLIB2_LIBRARIES} ${GLIBMM_LIBRARIES} ${LCMS_LIBRARIES} ${IPTCDATA_LIBRARIES}
    ${JPEG_LIBRARIES} ${GLIB2_LIBRARIES} ${GLIBMM_LIBRARIES} ${PNG_LIBRARIES} ${TIFF_LIBRARIES} ${ZLIB_LIBRARIES} ${GTKMM_LIBRARIES} ${GIO_LIBRARIES} ${GIOMM_LIBRARIES})




# set(GIO_LIBRARIES "/usr/lib/x86_64-linux-gnu/libgio-2.0.so")
# set(GIOMM_LIBRARIES "/usr/lib/x86_64-linux-gnu/libgiomm-2.4.so")
# set(GTKMM_LIBRARIES "libgtkmm-3.0.so")
#libgiomm-2.4.a
      message(STATUS "${ZLIB_LIBRARIES}")


add_executable (example ex1simple.cc)

 include_directories (${EXTRA_INCDIR} ${GLIB2_INCLUDE_DIRS} ${GLIBMM_INCLUDE_DIRS}
        ${GTK_INCLUDE_DIRS} ${GTKMM_INCLUDE_DIRS} ${GIO_INCLUDE_DIRS} ${GIOMM_INCLUDE_DIRS} ${IPTCDATA_INCLUDE_DIRS} ${LCMS_INCLUDE_DIRS} ${GTHREAD_INCLUDE_DIRS} ${GOBJECT_INCLUDE_DIRS} )
    link_directories (${EXTRA_LIBDIR} ${GLIB2_LIBRARY_DIRS} ${GLIBMM_LIBRARY_DIRS}
        ${GTK_LIBRARY_DIRS} ${GTKMM_LIBRARY_DIRS} ${GIO_LIBRARY_DIRS} ${GIOMM_LIBRARY_DIRS} ${IPTCDATA_LIBRARY_DIRS} ${LCMS_LIBRARY_DIRS} ${GTHREAD_LIBRARY_DIRS} ${GOBJECT_LIBRARY_DIRS})

set_target_properties (example PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS}" OUTPUT_NAME example2)
#target_link_libraries (rth rtengine ${JPEG_LIBRARIES} ${PNG_LIBRARIES} ${ZLIB_LIBRARIES} ${TIFF_LIBRARIES} ${EXTRA_LIB} ${GOBJECT_LIBRARIES} ${GTHREAD_LIBRARIES}
#   ${GLIB2_LIBRARIES} ${GLIBMM_LIBRARIES} ${GTK_LIBRARIES} ${GTKMM_LIBRARIES} ${GIO_LIBRARIES} ${GIOMM_LIBRARIES} ${LCMS_LIBRARIES} ${IPTCDATA_LIBRARIES})
target_link_libraries (example rtengineH ${GIO_PROCESS_LIBS} ${JPEG_LIBRARIES} ${PNG_LIBRARIES} ${ZLIB_LIBRARIES} ${TIFF_LIBRARIES} ${GOBJECT_LIBRARIES} ${GTHREAD_LIBRARIES}
   ${GLIB2_LIBRARIES} ${GLIBMM_LIBRARIES} ${GTK_LIBRARIES} ${GTKMM_LIBRARIES} ${GIO_LIBRARIES} ${GIOMM_LIBRARIES} ${LCMS_LIBRARIES} ${IPTCDATA_LIBRARIES} ${EXTRA_LIB_RTGUI})
#install (TARGETS rth DESTINATION ${BINDIR})

