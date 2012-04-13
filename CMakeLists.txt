
# openPablo CMakeLists.txt

# --- cmake settings
#
  cmake_minimum_required (VERSION 2.8)


# --- project settings
#
  PROJECT (openpablo)
    set (openpablo_VERSION_MAJOR 0)
    set (openpablo_VERSION_MINOR 1)
    set (openpablo_LIB_VERSION 1)

    SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules)

    # should doc-generation (doxygen, manual) be run? 
    set (compile_docs OFF)


# --- extra options
#
# option(USE_CAMERA_SUPPORT "Detect and use camera support if available." ON)



# --- specific os things
#
  IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    # linux specific code
    SET(OperatingSystem "Linux")
    message("-- Linux build detected.")
  ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux") 


  IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    # OSX specific code
    SET(OperatingSystem "Mac OS X")
    message("-- OSX build detected.")
  ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin") 


  if(WIN32)
    # windows specific code
    SET(OperatingSystem "Win")
    message("-- Windows build detected.")
  endif(WIN32)





# --- avoid source tree pollution
#
	If(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
		message(FATAL_ERROR "In-source builds are not permitted. Make a separate folder for building:\nmkdir build; cd build; cmake ..\nBefore that, remove the files already created:\nrm -rf CMakeCache.txt CMakeFiles")
	endif(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
	set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")


# --- required packages
#
	find_package(OpenCV REQUIRED)
	find_package(Exiv2 REQUIRED)
	find_package(LCMS2 REQUIRED)
	find_package(ImageMagick COMPONENTS Magick++ REQUIRED)
	find_package(ImageMagick COMPONENTS MagickCore REQUIRED)
	find_package(Qt COMPONENTS QtOpenGL QtXml REQUIRED)
	find_package(LIBPODOFO REQUIRED)
	find_package(LibRaw REQUIRED)
	find_package(Boost COMPONENTS system filesystem REQUIRED)
	find_package(Qt4 4.7 REQUIRED QtCore QtGui QtXml)
	find_package(Exiv2 REQUIRED)
	find_package(Lensfun REQUIRED)
	find_package(LibMagic REQUIRED)
#	find_package(RawTherapeeEngine REQUIRED)



# --- find dependencies
#
	
	IF (UNIX)
	  FIND_PACKAGE(PkgConfig)
	ENDIF (UNIX)
	
	IF(WIN32)
	    set(OPENPABLO_SHARED "0" CACHE STRING "Build shared internal libraries" )
	ELSE(WIN32)
	    set(OPENPABLO_SHARED "1" CACHE STRING "Build shared internal libraries" )
	ENDIF(WIN32)




	add_definitions(-Wall -W -ggdb)
	include_directories(${ImageMagick_INCLUDE_DIRS})
	include_directories (${LIBPODOFO_INCLUDE_DIR})
	INCLUDE_DIRECTORIES(${EXIV2_INCLUDE_DIR})
	INCLUDE_DIRECTORIES(${LENSFUN_INCLUDE_DIR})
	INCLUDE_DIRECTORIES(${LibMagic_INCLUDE_DIR})
#	include_directories (${RawTherapeeEngine_INCLUDE_DIR})
	
	include_directories(${CMAKE_SOURCE_DIR}/src)
	include_directories(${CMAKE_BINARY_DIR}/src)
	#include_directories(${CMAKE_SOURCE_DIR}/src/processors)
	
	include(${QT_USE_FILE})



# --- add subdirectories
	add_subdirectory(src)
	add_subdirectory(data)
	add_subdirectory(doc)





# --- add 3rd party libraries
#
	#add_subdirectory(3rdparty/json_spirit)
	#include_directories (${JSONSPIRIT_SOURCE_DIR}/json_spirit)
	#link_directories (${JSONSPIRIT_BINARY_DIR}/json_spirit)
	
	#SET(SRC openpablo)
	
	#install (FILES openpablo DESTINATION "${PROJECT_BINARY_DIR}/bin")
	
	set(PROJECT_BINARY_DIR ${PROJECT_SOURCE_DIR}/bin)
	set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})



# --- build a CPack driven installer package
#
	include (InstallRequiredSystemLibraries)
	set (CPACK_RESOURCE_FILE_LICENSE  
	     "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
	set (CPACK_PACKAGE_VERSION_MAJOR "${Tutorial_VERSION_MAJOR}")
	set (CPACK_PACKAGE_VERSION_MINOR "${Tutorial_VERSION_MINOR}")
	include (CPack)




# --- copy over some files
#
	file(COPY ${PROJECT_SOURCE_DIR}/data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
	file(COPY ${PROJECT_SOURCE_DIR}/tests DESTINATION ${CMAKE_CURRENT_BINARY_DIR} FILES_MATCHING PATTERN *)



#
# ----------------------------------------------------------------------------
# add a target to generate API documentation with Doxygen
# ----------------------------------------------------------------------------#
# 

  find_package(Doxygen)

  if(DOXYGEN_FOUND)
    message("Doxygen template file is ${PROJECT_SOURCE_DIR}/doc/Doxyfile.in")

    configure_file(${PROJECT_SOURCE_DIR}/doc/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
    if(compile_docs)
      add_custom_target(doxygen ALL ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile 
	  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	  COMMENT "Generating API documentation with Doxygen" VERBATIM)
    else(compile_docs)
#      add_custom_target(doxygen ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile 
#	  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
#	  COMMENT "Generating API documentation with Doxygen" VERBATIM)
    endif(compile_docs)
  endif(DOXYGEN_FOUND)

