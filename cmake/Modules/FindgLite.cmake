#
# Try to find the gLite build files and libraries
# gLite_FOUND - gLite files have been detected -
# gLite_INCLUDE_DIRS - headers paths -
# gLite_LIBRARIES - libraries paths -

message("-- checking for module 'gLite'")
find_path(gLite_VOMS_INCLUDE_DIR
          NAMES voms/voms_api.h
          PATHS /opt/glite/include/glite/security /usr/include)

find_library(gLite_VOMS_LIBRARY
             NAMES vomsapi_nog
             PATHS /opt/glite/lib /opt/glite/lib64 /usr/lib /usr/lib64)

set(gLite_INCLUDE_DIRS ${gLite_VOMS_INCLUDE_DIR})
set(gLite_LIBRARIES ${gLite_VOMS_LIBRARY})

if(gLite_INCLUDE_DIRS AND gLite_LIBRARIES)
    message("--   found  gLite")
    message("--   gLite_INCLUDE_DIRS: ${gLite_INCLUDE_DIRS}")
    message("--   gLite_LIBRARIES: ${gLite_LIBRARIES}")
    set(glite_FOUND TRUE)
else(gLite_INCLUDE_DIRS AND gLite_LIBRARIES)
    message("--   gLite not found")
    set(glite_FOUND FALSE)
endif(gLite_INCLUDE_DIRS AND gLite_LIBRARIES)
