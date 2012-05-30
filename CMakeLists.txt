project(voms-gnome-applet CXX)
cmake_minimum_required(VERSION 2.6)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

option(SYS_INSTALL_PREFIX "Root of the installation")
if ( NOT SYS_INSTALL_PREFIX )
    set(SYS_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
endif ( NOT SYS_INSTALL_PREFIX )

set(ICON22_SUBDIR "usr/share/icons/hicolor/22x22/apps")
set(ICON22_DIR "${SYS_INSTALL_PREFIX}/${ICON22_SUBDIR}")

set(ICON48_SUBDIR "usr/share/icons/hicolor/48x48/apps")
set(ICON48_DIR "${SYS_INSTALL_PREFIX}/${ICON48_SUBDIR}")

set(PIXMAP_DIR "usr/share/pixmaps")

set(XML_UI_SUBDIR "usr/share/voms-gnome-applet")
set(XML_UI_DIR "${SYS_INSTALL_PREFIX}/${XML_UI_SUBDIR}")

set(DESKTOP_DIR "usr/share/applications")

add_subdirectory(src)
add_subdirectory(icons)
add_subdirectory(misc)
