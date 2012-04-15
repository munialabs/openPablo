
#
# Add files for openpablo
#

SET(TOOLS_SOURCE
  FileLogger.cpp
  HTMLLogger.cpp
  )


SET(TOOLS_HEADER
  FileLogger.hpp
  HTMLLogger.hpp
)


IF (${OPENPABLO_SHARED_LIBS})
  add_library(tools SHARED ${TOOLS_SOURCE} ${TOOLS_HEADER})
  #target_link_libraries(tools ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
  #set_target_properties(tools PROPERTIES VERSION ${HUGIN_LIB_VERSION})
  IF(WIN32)
    install(TARGETS tools RUNTIME DESTINATION ${BINDIR})
  ELSEIF(${HUGIN_LIBS_PRIVATE_DIR})
    install(TARGETS tools LIBRARY DESTINATION ${LIBDIR}/hugin NAMELINK_SKIP)
  ELSE(WIN32)
    install(TARGETS tools LIBRARY DESTINATION ${LIBDIR} NAMELINK_SKIP)
  ENDIF(WIN32)
ELSE (${OPENPABLO_SHARED_LIBS})
  add_library(tools STATIC ${TOOLS_SOURCE} ${TOOLS_HEADER})
ENDIF (${OPENPABLO_SHARED_LIBS})

target_link_libraries(tools ${LOGOG_LIBRARY})# ${QT_LIBRARIES}) # ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
install(TARGETS tools DESTINATION lib)        

