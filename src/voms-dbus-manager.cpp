/*
 * LICENSE TO BE DEFINED
 */

#include "voms-applet.h"
#include "voms-dbus-manager.h"
#include <iostream>

/*
 * DBUS
 * VOMS-applet-interface
 * client side stub
 */
VOMS_dbus_client::VOMS_dbus_client(DBus::Connection &connection, const char *path, const char *name) 
    : DBus::ObjectProxy(connection, path, name) {
    
}



/*
 * DBUS
 * VOMS-applet-interface
 * server side skelethon
 */
VOMS_dbus_server::VOMS_dbus_server(VOMS_config* cfg,
                                   VOMS_client_engine *v_engine, 
                                   DBus::Connection &connection,
                                   const char *path) 
    : DBus::ObjectAdaptor(connection, path){
    
    engine = v_engine;
    config = cfg;
    
}

uint32_t VOMS_dbus_server::GetTimeleft() {
    return engine->get_timeleft();
}

std::string VOMS_dbus_server::GetVOName() {
    return engine->get_VO();
}

void VOMS_dbus_server::Configure() {

    Glib::signal_timeout().connect(sigc::mem_fun(*config, &VOMS_config::edit_prefs_remote), 500);

}



/*
 * DBUS
 * DBus interface
 * client stub
 */
dbus_registry_client::dbus_registry_client(DBus::Connection &connection)
    : DBus::ObjectProxy(connection, "/org/freedesktop/DBus", "org.freedesktop.DBus") {
}

void dbus_registry_client::NameOwnerChanged(const std::string& arg1,
                                            const std::string& arg2,
                                            const std::string& arg3) {}

void dbus_registry_client::NameLost(const std::string& name) {}
void dbus_registry_client::NameAcquired(const std::string& name) {}



/*
 * DBUS
 * VOMS-applet-interface
 * Client for configuration dialog
 */
bool call_remote_config() {
    DBus::BusDispatcher tmp_dispatcher;
    DBus::default_dispatcher = &tmp_dispatcher;
    DBus::Connection conn = DBus::Connection::SessionBus();
    
    /*
     * TODO check if the service is running first
     */
    
    VOMS_dbus_client voms_dbus_client(conn, GAPPLET_SERVER_PATH, GAPPLET_SERVER_NAME);
    try {
        voms_dbus_client.Configure();
        return true;
    } catch ( DBus::Error err ) {
        std::cerr << "DBUS error " << err.what() << std::endl;
    }
    return false;
}






bool VOMS_applet::init_dbus_manager() {
    
    DBus::_init_threading();
    already_running = false;
    
    /*
     * TODO investigate different way to pass the dispatcher to the conn
     */
    DBus::BusDispatcher tmp_dispatcher;
    DBus::default_dispatcher = &tmp_dispatcher;
    DBus::Connection conn = DBus::Connection::SessionBus();
    
    dbus_registry_client dbus_registry(conn);
    try {
    
        std::vector< std::string > dbus_srvs = dbus_registry.ListNames();
        for ( std::vector< std::string >::iterator srv_item = dbus_srvs.begin();
              srv_item != dbus_srvs.end();
              srv_item++ ) {
              
            //std::cout << "Found service " << *srv_item << std::endl;
            if ( *srv_item == GAPPLET_SERVER_NAME ) {
                already_running = true;
            }
        }
        
    } catch ( DBus::Error err ) {
        std::cerr << "DBUS error " << err.what() << std::endl;
    }

    if ( !already_running ) {
    
        dispatcher = new DBus::BusDispatcher();
        DBus::default_dispatcher = dispatcher;
        d_conn = new DBus::Connection(DBus::Connection::SessionBus());
        
        d_conn->request_name(GAPPLET_SERVER_NAME);
        dbus_server = new VOMS_dbus_server(config, engine, *d_conn, GAPPLET_SERVER_PATH);
        
        dbus_dispatcher_thr = Glib::Thread::create(
                              sigc::mem_fun(*this, &VOMS_applet::run_dbus_manager), true);

    } else {
    
        d_conn = NULL;
        dbus_server = NULL;
        dispatcher = NULL;
        
    }
    
    return already_running;

}

void VOMS_applet::destroy_dbus_manager() {

    if ( !already_running ) {
        stop_dbus_manager();
    }

    if ( dbus_server ) {
        delete dbus_server;
    }
    
    if ( d_conn ) {
        delete d_conn;
    }
    
    if ( dispatcher ) {
        delete dispatcher;
    }
    
}

void VOMS_applet::run_dbus_manager() {
    
    dispatcher->enter();

}

void VOMS_applet::stop_dbus_manager() {

    dispatcher->leave();
    dbus_dispatcher_thr->join();
    
}

