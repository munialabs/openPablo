cmake_minimum_required (VERSION 2.8)


PROJECT (openpablo)
set (openpablo_VERSION_MAJOR 0)
set (openpablo_VERSION_MINOR 1)


SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules)


FIND_PACKAGE(OpenCV REQUIRED)
find_package(LCMS2 REQUIRED)
find_package(ImageMagick COMPONENTS Magick++ REQUIRED)
find_package(ImageMagick COMPONENTS MagickCore REQUIRED)
find_package(Qt COMPONENTS QtOpenGL QtXml REQUIRED)
find_package(LIBPODOFO REQUIRED)
find_package(Boost COMPONENTS system filesystem REQUIRED)
find_package(Qt4 4.7 REQUIRED QtCore QtGui QtXml)


add_definitions(-Wall -W -ggdb)
include_directories(${ImageMagick_INCLUDE_DIRS})
include_directories (${LIBPODOFO_INCLUDE_DIR})

include(${QT_USE_FILE})


# add 3rd party libraries
#add_subdirectory(3rdparty/json_spirit)
#include_directories (${JSONSPIRIT_SOURCE_DIR}/json_spirit)
#link_directories (${JSONSPIRIT_BINARY_DIR}/json_spirit)

#SET(SRC openpablo)

#install (FILES openpablo DESTINATION "${PROJECT_BINARY_DIR}/bin")

ADD_EXECUTABLE(openpablo src/openpablo.cxx)
TARGET_LINK_LIBRARIES(openpablo ${QT_LIBRARIES} ${OpenCV_LIBS} ${LIBPODOFO_LIBRARY} ${ImageMagick_LIBRARIES} ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY})


#add_subdirectory(src)


# build a CPack driven installer package
include (InstallRequiredSystemLibraries)
set (CPACK_RESOURCE_FILE_LICENSE  
     "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
set (CPACK_PACKAGE_VERSION_MAJOR "${Tutorial_VERSION_MAJOR}")
set (CPACK_PACKAGE_VERSION_MINOR "${Tutorial_VERSION_MINOR}")
include (CPack)
