#
# Try to find the LibNotifymm build files and libraries
# LibNotifymm_FOUND - LibNotifymm files have been detected -
# LibNotifymm_INCLUDE_DIRS - headers paths -
# LibNotifymm_LIBRARIES - libraries paths -

message("-- checking for module 'libnotifymm'")

find_path(LibNotifymm_INCLUDE_DIRS
          NAMES libnotifymm.h
          PATHS_SUFFIXES libnotifymm-1.0)

find_library(LibNotifymm_LIBRARIES
             NAMES libnotifymm-1.0.so)

if(LibNotifymm_INCLUDE_DIRS AND LibNotifymm_LIBRARIES)
    set(LibNotifymm_INCLUDE_DIRS ${LibNotifymm_INCLUDE_DIRS}/libnotifymm-1.0)
    message("--   found  LibNotifymm")
    message("--   LibNotifymm_INCLUDE_DIRS: ${LibNotifymm_INCLUDE_DIRS}")
    message("--   LibNotifymm_LIBRARIES: ${LibNotifymm_LIBRARIES}")
    set(LibNotifymm_FOUND TRUE)
else(LibNotifymm_INCLUDE_DIRS AND LibNotifymm_LIBRARIES)
    message("--   LibNotifymm not found")
    set(LibNotifymm_FOUND FALSE)
endif(LibNotifymm_INCLUDE_DIRS AND LibNotifymm_LIBRARIES)
