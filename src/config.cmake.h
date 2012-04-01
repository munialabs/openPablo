#pragma once

// CMake uses config.cmake.h to generate config.h within the build folder.
#ifndef DARKTABLE_CONFIG_H
#define DARKTABLE_CONFIG_H

#define GETTEXT_PACKAGE "darktable"
#define DARKTABLE_LOCALEDIR "@LOCALE_DIR@"

#define PACKAGE_NAME "@CMAKE_PROJECT_NAME@"
#define PACKAGE_VERSION "@PROJECT_VERSION@"
#define PACKAGE_STRING PACKAGE_NAME " " PACKAGE_VERSION
#define PACKAGE_BUGREPORT "hanatos@gmail.com"

#define DARKTABLE_LIBDIR "@CMAKE_INSTALL_PREFIX@/@LIB_INSTALL@"
#define DARKTABLE_DATADIR "@CMAKE_INSTALL_PREFIX@/@SHARE_INSTALL@"


#endif
