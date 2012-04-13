
#
# Add files for openpablo
#

SET(ENGINES_SOURCE
  Engine.cpp
  EngineFactory.cpp
  BauhausEngine.cpp
  MagickEngine.cpp
  )


SET(ENGINES_HEADER
  Engine.hpp
  EngineFactory.hpp
  BauhausEngine.hpp
  MagickEngine.hpp
)


IF (${OPENPABLO_SHARED_LIBS})
  add_library(engines SHARED ${ENGINES_SOURCE} ${ENGINES_HEADER})
  #target_link_libraries(engines ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
  #set_target_properties(engines PROPERTIES VERSION ${HUGIN_LIB_VERSION})
  IF(WIN32)
    install(TARGETS engines RUNTIME DESTINATION ${BINDIR})
  ELSEIF(${HUGIN_LIBS_PRIVATE_DIR})
    install(TARGETS engines LIBRARY DESTINATION ${LIBDIR}/hugin NAMELINK_SKIP)
  ELSE(WIN32)
    install(TARGETS engines LIBRARY DESTINATION ${LIBDIR} NAMELINK_SKIP)
  ENDIF(WIN32)
ELSE (${OPENPABLO_SHARED_LIBS})
  add_library(engines STATIC ${ENGINES_SOURCE} ${ENGINES_HEADER})
ENDIF (${OPENPABLO_SHARED_LIBS})

target_link_libraries(engines ${QT_LIBRARIES}) # ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
install(TARGETS engines DESTINATION lib)        

