/*
 * LICENSE TO BE DEFINED
 */

#ifndef VOMS_APPLET_H
#define VOMS_APPLET_H

#include "config.h"
#include "voms-applet-config.h"
#include "voms-server-list.h"
#include "voms-client-engine.h"
#include "voms-dbus-manager.h"

#include "gtkmm/menu.h"
#include "gtkmm/statusicon.h"
#include "glibmm/refptr.h"
#include "gtkmm/tooltip.h"
#include "gtkmm/aboutdialog.h"

class VOMS_applet : public VOMS_client_listener {

    public:
    VOMS_applet();
    bool init();
    void show_ctx_menu(guint, guint32);
    void show_vomses_menu();
    virtual void notify_ren_attempt(Glib::ustring, Glib::ustring, int);
    virtual void notify_gen_error(Glib::ustring);
    virtual void notify_renewed(Glib::ustring, Glib::ustring, int);
    bool update_proxy_view();
    bool query_tooltip(int, int, bool, const Glib::RefPtr<Gtk::Tooltip>&);
    void about_response(int);
    void prefs_response(int);
    virtual ~VOMS_applet();
    
    private:
    VOMS_client_engine* engine;
    VOMS_server_list* vomses_menu;
    Gtk::Menu::Menu* context_menu;
    Gtk::AboutDialog* voms_about;
    Glib::RefPtr<Gtk::StatusIcon> voms_icon;
    VOMS_config* config;
    sigc::connection icon_timer_conn;
    VOMS_icon_file icon_file;
    int err_notified;

#ifdef USE_DBUSMAN
    private:
    bool init_dbus_manager();
    void destroy_dbus_manager();
    void run_dbus_manager();
    void stop_dbus_manager();
    
    bool already_running;
    DBus::Connection* d_conn;
    VOMS_dbus_server* dbus_server;
    DBus::BusDispatcher* dispatcher;
    Glib::Thread* dbus_dispatcher_thr;
#endif
};

#endif
