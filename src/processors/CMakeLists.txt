
#
# Add files for openpablo
#

SET(PROCESSORS_SOURCE
  Processor.cpp
  ProcessorFactory.cpp
  ImageProcessor.cpp
  PDFProcessor.cpp
  PSDProcessor.cpp
  RAWProcessor.cpp
  )


SET(PROCESSORS_HEADER
  Processor.hpp
  ProcessorFactory.hpp
  ImageProcessor.hpp
  PDFProcessor.hpp
  PSDProcessor.hpp
  RAWProcessor.hpp
)


IF (${OPENPABLO_SHARED_LIBS})
  add_library(processors SHARED ${PROCESSORS_SOURCE} ${PROCESSORS_HEADER})
  #target_link_libraries(processors ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
  #set_target_properties(processors PROPERTIES VERSION ${HUGIN_LIB_VERSION})
  IF(WIN32)
    install(TARGETS processors RUNTIME DESTINATION ${BINDIR})
  ELSEIF(${HUGIN_LIBS_PRIVATE_DIR})
    install(TARGETS processors LIBRARY DESTINATION ${LIBDIR}/hugin NAMELINK_SKIP)
  ELSE(WIN32)
    install(TARGETS processors LIBRARY DESTINATION ${LIBDIR} NAMELINK_SKIP)
  ENDIF(WIN32)
ELSE (${OPENPABLO_SHARED_LIBS})
  add_library(processors STATIC ${PROCESSORS_SOURCE} ${PROCESSORS_HEADER})
ENDIF (${OPENPABLO_SHARED_LIBS})

target_link_libraries(processors ${QT_LIBRARIES}) # ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
install(TARGETS processors DESTINATION lib)        

