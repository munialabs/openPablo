#
# Find the RawTherapee engine includes and library
#

FIND_PATH(RawTherapeeEngine_INCLUDE_DIR rtengine.h
  /usr/include
  /usr/local/include
  #PATH_SUFFIXES rt
)

FIND_LIBRARY(RawTherapeeEngine_LIBRARY
  NAMES ${RawTherapeeEngine_NAMES} librtengine.so
  PATHS /usr/lib /usr/local/lib
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(RawTherapeeEngine DEFAULT_MSG RawTherapeeEngine_LIBRARY RawTherapeeEngine_INCLUDE_DIR)

IF(RawTherapeeEngine_FOUND)
  SET(RawTherapeeEngine_LIBRARIES ${RawTherapeeEngine_LIBRARY})
  SET(RawTherapeeEngine_INCLUDE_DIRS ${RawTherapeeEngine_INCLUDE_DIR})
ENDIF(RawTherapeeEngine_FOUND)

