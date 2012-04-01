set(CPACK_PACKAGE_NAME "${CMAKE_PROJECT_NAME}")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "The digital darkroom")
set(CPACK_PACKAGE_CONTACT "http://www.darktable.org/")
set(CPACK_SOURCE_IGNORE_FILES
   "/.gitignore"
   "${CMAKE_BINARY_DIR}/"
   "/.git/"
   "/.deps/"
   "/.build/"
)
set(CPACK_PACKAGE_EXECUTABLES darktable)
set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_GENERATOR "TGZ")
SET(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
		
if("${CMAKE_BUILD_TYPE}" MATCHES "Release")
	set(CPACK_STRIP_FILES TRUE)
endif("${CMAKE_BUILD_TYPE}" MATCHES "Release")

# Set package generator for MacOSX
if(APPLE)
	make_directory(${CMAKE_BINARY_DIR}/packaging/macosx)
	configure_file( ${CMAKE_SOURCE_DIR}/cmake/macosx/Info.plist.in ${CMAKE_BINARY_DIR}/packaging/macosx/Info.plist )
	configure_file( ${CMAKE_SOURCE_DIR}/cmake/macosx/start.in ${CMAKE_BINARY_DIR}/packaging/macosx/start )
	set(CPACK_GENERATOR "Bundle")
	set(CPACK_BUNDLE_PLIST ${CMAKE_BINARY_DIR}/packaging/macosx/Info.plist)
	set(CPACK_BUNDLE_ICON ${CMAKE_SOURCE_DIR}/cmake/macosx/darktable.icns)
	set(CPACK_BUNDLE_NAME "darktable")
	set(CPACK_BUNDLE_STARTUP_COMMAND ${CMAKE_BINARY_DIR}/packaging/macosx/start)
	set(CPACK_PACKAGE_EXECUTABLES "darktable" "Darktable - Raw Editor")
endif(APPLE)

# Set package for unix
if(UNIX)
	# Try to find architecture
	execute_process(COMMAND uname -m OUTPUT_VARIABLE CPACK_PACKAGE_ARCHITECTURE)
	string(STRIP "${CPACK_PACKAGE_ARCHITECTURE}" CPACK_PACKAGE_ARCHITECTURE)
	# Try to find distro name and distro-specific arch
	execute_process(COMMAND lsb_release -is OUTPUT_VARIABLE LSB_ID)
	execute_process(COMMAND lsb_release -rs OUTPUT_VARIABLE LSB_RELEASE)
	string(STRIP "${LSB_ID}" LSB_ID)
	string(STRIP "${LSB_RELEASE}" LSB_RELEASE)
	set(LSB_DISTRIB "${LSB_ID}${LSB_RELEASE}")
	if(NOT LSB_DISTRIB)
		set(LSB_DISTRIB "unix")
	endif(NOT LSB_DISTRIB)
	
	if("${LSB_DISTRIB}" MATCHES "Fedora|Mandriva")
		make_directory(${CMAKE_BINARY_DIR}/packaging/rpm)
		set(CPACK_GENERATOR "RPM")
		set(CPACK_RPM_PACKAGE_ARCHITECTURE ${CPACK_PACKAGE_ARCHITECTURE})
		set(CPACK_RPM_PACKAGE_RELEASE "1")
		
	endif("${LSB_DISTRIB}" MATCHES "Fedora|Mandriva")
	
	
	# For Debian-based distros we want to create DEB packages.
	if("${LSB_DISTRIB}" MATCHES "Ubuntu|Debian")
		make_directory(${CMAKE_BINARY_DIR}/packaging/debian)
		set(GTK_UPDATE_ICON_CACHE "gtk-update-icon-cache -f -t ${THEME_DIRECTORY}")
		configure_file( ${CMAKE_SOURCE_DIR}/cmake/debian/postinst.in ${CMAKE_BINARY_DIR}/packaging/debian/postinst )
		configure_file( ${CMAKE_SOURCE_DIR}/cmake/debian/prerm.in ${CMAKE_BINARY_DIR}/packaging/debian/prerm )
	
		set(CPACK_GENERATOR "DEB")
		set(CPACK_DEBIAN_PACKAGE_PRIORITY "extra")
		set(CPACK_DEBIAN_PACKAGE_SECTION "photos")
		set(CPACK_DEBIAN_PACKAGE_RECOMMENDS "")
		set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_BINARY_DIR}/packaging/debian/postinst;${CMAKE_BINARY_DIR}/packaging/debian/prerm;" )
		
		# We need to alter the architecture names as per distro rules
		if("${CPACK_PACKAGE_ARCHITECTURE}" MATCHES "i[3-6]86")
			set(CPACK_PACKAGE_ARCHITECTURE i386)
		endif("${CPACK_PACKAGE_ARCHITECTURE}" MATCHES "i[3-6]86")
		if("${CPACK_PACKAGE_ARCHITECTURE}" MATCHES "x86_64")
			set(CPACK_PACKAGE_ARCHITECTURE amd64)
		endif("${CPACK_PACKAGE_ARCHITECTURE}" MATCHES "x86_64")

		# Set the dependencies based on the distro version

		# Ubuntu
		if("${LSB_DISTRIB}" MATCHES "Ubuntu9.10")
			set(CPACK_DEBIAN_PACKAGE_DEPENDS "libgtk2.0-0, libsqlite3-0, libglade2-0, libgegl-0.0-0, libcairo2, libexiv2-5, liblcms1, libjpeg62, libpng12-0, liblensfun0, liblensfun-data")
		endif("${LSB_DISTRIB}" MATCHES "Ubuntu9.10")

		if("${LSB_DISTRIB}" MATCHES "Ubuntu10.04")
			set(CPACK_DEBIAN_PACKAGE_DEPENDS "libgtk2.0-0, libsqlite3-0, libglade2-0, libgegl-0.0-0, libcairo2, libexiv2-5, liblcms1, libjpeg62, libpng12-0, liblensfun0, liblensfun-data")
		endif("${LSB_DISTRIB}" MATCHES "Ubuntu10.04")

		if("${LSB_DISTRIB}" MATCHES "Ubuntu10.10")
			set(CPACK_DEBIAN_PACKAGE_DEPENDS "libgtk2.0-0, libsqlite3-0, libglade2-0, libgegl-0.0-0, libcairo2, libexiv2-6, liblcms1, libjpeg62, libpng12-0, liblensfun0, liblensfun-data")
		endif("${LSB_DISTRIB}" MATCHES "Ubuntu10.10")

		# Debian
		if("${LSB_DISTRIB}" MATCHES "Debian5.*")
			set(CPACK_DEBIAN_PACKAGE_DEPENDS "libgtk2.0-0, libsqlite3-0, libglade2-0, libgegl-0.0-0, libcairo2, libexiv2-5, liblcms1, libjpeg62, libpng12-0, liblensfun0, liblensfun-data")
		endif("${LSB_DISTRIB}" MATCHES "Debian5.*")

		if("${LSB_DISTRIB}" MATCHES "Debiantesting")
                	set(CPACK_DEBIAN_PACKAGE_DEPENDS "libgtk2.0-0, libsqlite3-0, libglade2-0, libgegl-0.0-0, libcairo2, libexiv2-5, liblcms1, libjpeg62, libpng12-0, liblensfun0, liblensfun-data")
		endif("${LSB_DISTRIB}" MATCHES "Debiantesting")

		if(NOT CPACK_DEBIAN_PACKAGE_DEPENDS)
			message("WARNING: ${LSB_DISTRIB} not supported yet.\nPlease set deps in cmake/darktable-packaging.cmake before packaging.")
		endif(NOT CPACK_DEBIAN_PACKAGE_DEPENDS)
		string(TOLOWER "${CPACK_PACKAGE_NAME}_${CPACK_PACKAGE_VERSION}-${LSB_DISTRIB}_${CPACK_PACKAGE_ARCHITECTURE}" CPACK_PACKAGE_FILE_NAME)
	endif("${LSB_DISTRIB}" MATCHES "Ubuntu|Debian")
	
endif(UNIX)

if(WIN32)
  set(CPACK_GENERATOR "NSIS")
  set(CPACK_PACKAGE_EXECUTABLES "darktable" "Darktable - Raw Editor")
  set(CPACK_PACKAGE_INSTALL_DIRECTORY "${CMAKE_PROJECT_NAME}")
  # There is a bug in NSI that does not handle full unix paths properly. Make
  # sure there is at least one set of four (4) backlasshes.
  #SET(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/themes/default\\\\icon.bmp")
  #SET(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/themes/default\\\\icon.ico")
  SET(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\${CMAKE_PROJECT_NAME}.exe")
  SET(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} Darktable")
  SET(CPACK_NSIS_HELP_LINK "http:\\\\\\\\darktable.sourceforge.net")
  SET(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\darktable.sourceforge.net")
  SET(CPACK_NSIS_MODIFY_PATH OFF)

endif(WIN32)

include(CPack)

ADD_CUSTOM_TARGET(pkgsrc
	COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/version.cmake ${CMAKE_SOURCE_DIR}/cmake/version.cmake
	COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target package_source
	COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_SOURCE_DIR}/cmake/version.cmake
)
