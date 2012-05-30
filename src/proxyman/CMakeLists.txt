project(voms-gnome-applet)

find_package(OpenSSL REQUIRED)
find_package(gLite REQUIRED)

add_definitions(-DNOGLOBUS
                -DNO_GLOBUS_CONFIG_H)

include_directories(${OPENSSL_INCLUDE_DIR}
                    ${gLite_INCLUDE_DIRS}
                    ${CMAKE_BINARY_DIR}/src)

add_library(proxyman proxyman.cpp
                     proxyman_err.cpp
                     credentials.c
                     proxycertinfo.c
                     sslutils.c
                     globus_oldgaa_utils.c
                     oldgaa_alloc.c
                     oldgaa_api.c
                     oldgaa_gl_internal_err.c
                     oldgaa_policy_evaluator.c
                     oldgaa_release.c
                     oldgaa_utils.c
                     rfc1779.c)

