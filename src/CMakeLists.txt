project(voms-gnome-applet)

set(CMAKE_C_FLAGS_DEBUG "-g -Wall")
set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall")

add_subdirectory(proxyman)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTKMM gtkmm-2.4)
pkg_check_modules(GCONFMM gconfmm-2.6)
pkg_check_modules(GLADEMM libglademm-2.4)
pkg_check_modules(DBUS1 dbus-c++-1)

if( NOT GTKMM_VERSION STRGREATER "2.12.7" )
    add_definitions(-DNOQUERYTOOLTIP)
    message( "--   No query tooltip" )
endif( NOT GTKMM_VERSION STRGREATER "2.12.7" )

if( DBUS1_FOUND )
    message( "-- Dbus-1 found")
    add_definitions(-DUSE_DBUSMAN)
    set(DBUSMAN_SOURCES "voms-dbus-manager.cpp")
    
    add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/voms-dbus-skelethon.h
                              ${CMAKE_CURRENT_BINARY_DIR}/voms-dbus-stub.h
                              ${CMAKE_CURRENT_BINARY_DIR}/org-freedesktop-DBus-stub.h
                       DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/introspect-service.xml
                       COMMAND dbusxx-xml2cpp ${CMAKE_CURRENT_SOURCE_DIR}/introspect-service.xml 
                                              --adaptor=${CMAKE_CURRENT_BINARY_DIR}/voms-dbus-skelethon.h
                       COMMAND dbusxx-xml2cpp ${CMAKE_CURRENT_SOURCE_DIR}/introspect-service.xml 
                                              --proxy=${CMAKE_CURRENT_BINARY_DIR}/voms-dbus-stub.h
                       COMMAND dbusxx-xml2cpp ${CMAKE_CURRENT_SOURCE_DIR}/org.freedesktop.DBus.xml
                                              --proxy=${CMAKE_CURRENT_BINARY_DIR}/org-freedesktop-DBus-stub.h)

    add_custom_target(introspect ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/voms-dbus-skelethon.h
                                             ${CMAKE_CURRENT_BINARY_DIR}/voms-dbus-stub.h
                                             ${CMAKE_CURRENT_BINARY_DIR}/org-freedesktop-DBus-stub.h)
    
else( DBUS1_FOUND )
    message( "-- Dbus-1 not found")
    set(DBUS1_INCLUDE_DIRS "")
    set(DBUS1_LIBRARIES "")
    set(DBUSMAN_SOURCES "")
endif( DBUS1_FOUND )

find_package(gLite REQUIRED)
find_package(LibNotifymm)
if( LibNotifymm_FOUND )
    add_definitions(-DUSE_NOTIFY)
endif( LibNotifymm_FOUND )

find_package(OpenSSL REQUIRED)

add_executable(voms-gnome-applet voms-applet.cpp 
                                 voms-applet-config.cpp
                                 voms-icons-factory.cpp
                                 voms-applet-contacts.cpp
                                 voms-applet-policies.cpp
                                 voms-applet-vodir.cpp
                                 voms-server-list.cpp
                                 voms-client-engine.cpp
                                 ${DBUSMAN_SOURCES})

add_definitions(-D_REENTRANT -DNOGLOBUS)

configure_file(${CMAKE_SOURCE_DIR}/src/config.h.cmake ${CMAKE_BINARY_DIR}/src/config.h)

include_directories(${GTKMM_INCLUDE_DIRS}
                    ${GCONFMM_INCLUDE_DIRS}
                    ${GLADEMM_INCLUDE_DIRS}
                    ${LibNotifymm_INCLUDE_DIRS}
                    ${DBUS1_INCLUDE_DIRS}
                    ${gLite_INCLUDE_DIRS}
                    ${CMAKE_BINARY_DIR}/src
                    ${CMAKE_SOURCE_DIR}/src/proxyman)
                    
link_directories(${CMAKE_BINARY_DIR}/proxyman)

target_link_libraries(voms-gnome-applet ${GTKMM_LIBRARIES}
                                        ${GCONFMM_LIBRARIES}
                                        ${GLADEMM_LIBRARIES}
                                        ${LibNotifymm_LIBRARIES}
                                        ${DBUS1_LIBRARIES}
                                        ${OPENSSL_LIBRARIES}
                                        ${gLite_LIBRARIES}
                                        proxyman)

install(TARGETS voms-gnome-applet
        RUNTIME DESTINATION usr/bin)
        
install(FILES ${CMAKE_SOURCE_DIR}/src/basic.glade
        DESTINATION ${XML_UI_SUBDIR})

install(FILES ${CMAKE_SOURCE_DIR}/src/contact.glade
        DESTINATION ${XML_UI_SUBDIR})

install(FILES ${CMAKE_SOURCE_DIR}/src/policy.glade
        DESTINATION ${XML_UI_SUBDIR})

install(FILES ${CMAKE_SOURCE_DIR}/src/authvochooser.glade
        DESTINATION ${XML_UI_SUBDIR})

install(FILES ${CMAKE_SOURCE_DIR}/src/preferences.glade
        DESTINATION ${XML_UI_SUBDIR})

