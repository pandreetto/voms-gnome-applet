/*
 * LICENSE TO BE DEFINED
 */

#ifndef VOMS_DBUS_MANAGER_H
#define VOMS_DBUS_MANAGER_H

#ifdef USE_DBUSMAN

#include "config.h"

#include <dbus-c++/interface.h>
#include <dbus-c++/dbus.h>
#include "voms-dbus-skelethon.h"
#include "voms-dbus-stub.h"
#include <glibmm/thread.h>
#include "org-freedesktop-DBus-stub.h"

class VOMS_client_engine;

class dbus_registry_client :
    public org::freedesktop::DBus_proxy,
    public DBus::IntrospectableProxy,
    public DBus::ObjectProxy {
    
    public:
    dbus_registry_client(DBus::Connection&);
    virtual void NameOwnerChanged(const std::string&,
                                  const std::string&,
                                  const std::string&);
    virtual void NameLost(const std::string&);
    virtual void NameAcquired(const std::string&);
};

#define GAPPLET_SERVER_NAME "org.glite.voms.GApplet"
#define GAPPLET_SERVER_PATH "/org/glite/voms/GApplet"

class VOMS_dbus_client : 
    public org::glite::voms::GApplet_proxy,
    public DBus::IntrospectableProxy,
    public DBus::ObjectProxy {
    
    public:
    VOMS_dbus_client(DBus::Connection&, const char*, const char*);    
};

class VOMS_dbus_server :
    public org::glite::voms::GApplet_adaptor,
    public DBus::IntrospectableAdaptor,
    public DBus::ObjectAdaptor {
    
    public:
    VOMS_dbus_server(VOMS_config*, VOMS_client_engine*, DBus::Connection&, const char*);

    virtual uint32_t GetTimeleft();
    virtual std::string GetVOName();
    virtual void Configure();

    private:
    VOMS_client_engine* engine;
    VOMS_config* config;

};

bool call_remote_config();

#endif /* USE_DBUSMAN */

#endif /* VOMS_DBUS_MANAGER_H */
