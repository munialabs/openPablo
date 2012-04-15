# - Try to find the logog library
#
#  This module defines the following variables
#
#  LOGOG_FOUND - Was Logog found
#  LOGOG_INCLUDE_DIRS - the Logog include directories
#  LOGOG_LIBRARIES - Link to this
#
#  This module accepts the following variables
#
#  LOGOG_ROOT - Can be set to Logog install path or Windows build path
#
#=============================================================================
#  FindLogog.cmake, adapted from FindBullet.cmake which has the following
#  copyright -
#-----------------------------------------------------------------------------
# Copyright 2009 Kitware, Inc.
# Copyright 2009 Philip Lowman <philip@yhbt.com>
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

if (NOT DEFINED LOGOG_ROOT)
    set (LOGOG_ROOT /usr /usr/local)
endif (NOT DEFINED LOGOG_ROOT)

if(MSVC)
set(LIB_PATHS ${LOGOG_ROOT} ${LOGOG_ROOT}/Release)
else(MSVC)
set (LIB_PATHS ${LOGOG_ROOT} ${LOGOG_ROOT}/lib)
endif(MSVC)

macro(_FIND_LOGOG_LIBRARY _var)
  find_library(${_var}
     NAMES 
        ${ARGN}
     PATHS
		${LIB_PATHS}
     PATH_SUFFIXES lib
  )
  mark_as_advanced(${_var})
endmacro()

macro(_LOGOG_APPEND_LIBRARIES _list _release)
   set(_debug ${_release}_DEBUG)
   if(${_debug})
      set(${_list} ${${_list}} optimized ${${_release}} debug ${${_debug}})
   else()
      set(${_list} ${${_list}} ${${_release}})
   endif()
endmacro()

if(MSVC)
    find_path(LOGOG_INCLUDE_DIR NAMES logog.hpp
        PATHS
		  ${LOGOG_ROOT}/src/windows
          ${LOGOG_ROOT}/src/windows/logog
		  )
else(MSVC)
	# Linux/OS X builds
    find_path(LOGOG_INCLUDE_DIR NAMES logog.hpp
        PATHS
          ${LOGOG_ROOT}/include/logog
		  )
endif(MSVC)

# Find the libraries
if(MSVC)
    _FIND_LOGOG_LIBRARY(LOGOG_LIBRARY                   liblogog.lib)
else(MSVC)
	# Linux/OS X builds
    _FIND_LOGOG_LIBRARY(LOGOG_LIBRARY                   liblogog.a)
endif(MSVC)

message("logog library = " ${LOGOG_LIBRARY})

# handle the QUIETLY and REQUIRED arguments and set LOGOG_FOUND to TRUE if 
# all listed variables are TRUE
include("${CMAKE_ROOT}/Modules/FindPackageHandleStandardArgs.cmake")
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Logog DEFAULT_MSG
    LOGOG_LIBRARY)

if(MSVC)
    string(REGEX REPLACE "/logog$" "" VAR_WITHOUT ${LOGOG_INCLUDE_DIR})
    string(REGEX REPLACE "/windows$" "" VAR_WITHOUT ${VAR_WITHOUT})
    set(LOGOG_INCLUDE_DIRS ${LOGOG_INCLUDE_DIRS} "${VAR_WITHOUT}")
    string(REGEX REPLACE "/liblogog.lib" "" LOGOG_LIBRARY_DIR ${LOGOG_LIBRARY})
else(MSVC)
	# Linux/OS X builds
    set(LOGOG_INCLUDE_DIRS ${LOGOG_INCLUDE_DIR})
    string(REGEX REPLACE "/liblogog.so" "" LOGOG_LIBRARY_DIR ${LOGOG_LIBRARY})
endif(MSVC)

if(LOGOG_FOUND)
#   _LOGOG_APPEND_LIBRARIES(LOGOG LOGOG_LIBRARY)
endif()

