/*
 * LICENSE TO BE DEFINED
 */

#ifndef VOMS_SERVER_LIST_H
#define VOMS_SERVER_LIST_H

#include "config.h"
#include "voms-global-defs.h"
#include "voms-applet-config.h"
#include "voms-client-engine.h"
#include "voms-icons-factory.h"
#include "gtkmm/menu.h"
#include "glibmm/ustring.h"
#include "gtkmm/image.h"
#include "gtkmm/widget.h"
#include <vector>
#include <map>
#include "voms/voms_api.h"

typedef std::vector<contactdata> contact_data_vector;
class VOMS_server_list;

class VOMS_menu_item : 
    public Gtk::MenuItem::MenuItem,
    public contact_data_vector {

    public:
    VOMS_menu_item(Glib::ustring, VOMS_server_list*);
    virtual ~VOMS_menu_item();
    Glib::ustring get_VOMS_label();
    void set_icon(Glib::ustring);
    
    private:
    Glib::ustring label;
    Gtk::Image *voms_ac_icon;
    sigc::connection sig_conn;
};

typedef std::map<Glib::ustring, VOMS_menu_item*> v_menu_map;

class VOMS_server_list : 
    public Gtk::Menu::Menu, public VOMS_client_listener {

    public:
    VOMS_server_list(const VOMS_config&, VOMS_client_engine&);
    void activate_handler();
    virtual void notify_ren_attempt(Glib::ustring, Glib::ustring, int);
    virtual void notify_gen_error(Glib::ustring);
    virtual void notify_renewed(Glib::ustring, Glib::ustring, int);
    void notify_status(proxy_status_t);
    
    void clean_all();
    void reload(bool force = false);
    bool contains(Glib::ustring);
    virtual ~VOMS_server_list();
    
    private:
    VOMS_client_engine *client_engine;
    v_menu_map contacts_map;
    double last_list_modify;
    
    Glib::ustring current_vo;
    VOMS_icon_file icon_file;
    const VOMS_config *client_config;
};

#endif

