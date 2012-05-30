/*
 * LICENSE TO BE DEFINED
 */

#ifndef VOMS_CLIENT_ENGINE_H
#define VOMS_CLIENT_ENGINE_H

#include "config.h"
#include <libglademm.h>
#include <glibmm/thread.h>
#include "voms-applet-config.h"
#include "voms-global-defs.h"
#include <vector>

#include <ctime>
#include "voms/voms_api.h"

#include "proxyman/proxyman.h"

#define XML_DIALOG_LABEL "voms_dialog"
#define XML_RENEWBTN_LABEL "voms_renew_button"
#define XML_CANCBTN_LABEL "voms_cancel_button"
#define XML_PWDENTRY_LABEL "voms_entry"
#define XML_MESSAGE_LABEL "voms_message_label"
#define XML_TIMER_LABEL "voms_timer_label"
#define XML_POLICY_LABEL "policy_combo"
#define XML_ACHOURS_LABEL "hours_combo"
#define XML_ACMINUTES_LABEL "minutes_combo"

#define CHK_PRX_RELOADED   1
#define CHK_PRX_NOCHANGES  0
#define CHK_PRX_ERROR     -1
#define CHK_PRX_NOENT     -2
#define CHK_PRX_EXPIRED   -3

class policy_modelcol : public Gtk::TreeModel::ColumnRecord {
    public:
    
    policy_modelcol(){
        add(name);
    }
    
    Gtk::TreeModelColumn<Glib::ustring> name; 
};

class VOMS_proxy_info {
    public:
    VOMS_proxy_info();
    VOMS_proxy_info(contactdata&, time_t, time_t);
    VOMS_proxy_info& operator=(const VOMS_proxy_info&);
    
    time_t get_start_time();
    time_t get_end_time();
    Glib::ustring get_vo();
    Glib::ustring get_host();
    int get_port();
    proxy_status_t get_status();
    time_t get_timeleft();
    
    private:
    Glib::Mutex mutex;
    time_t start_time;
    time_t end_time;
    Glib::ustring vo_name;
    Glib::ustring host;
    int port;
};

class VOMS_client_listener {

    public:
    virtual void notify_ren_attempt(Glib::ustring, Glib::ustring, int) = 0;
    virtual void notify_gen_error(Glib::ustring) = 0;
    virtual void notify_renewed(Glib::ustring, Glib::ustring, int) = 0;
    
};

typedef std::vector<VOMS_client_listener*> listener_list;
typedef std::vector<contactdata> contact_data_vector;

class VOMS_client_engine {

    public:
    VOMS_client_engine(const VOMS_config&);
    bool hideDialog(GdkEventAny*);
    void toggleDialog();
    bool processing();
    void apply_renew();
    void cancel_renew();
    int renew(contact_data_vector&);
    void add_listener(VOMS_client_listener&);
    time_t get_timeleft();
    Glib::ustring get_VO();
    Glib::ustring get_host();
    int get_port();
    proxy_status_t get_proxy_status();
    int check_proxy();
    
    virtual ~VOMS_client_engine();
    
    private:
    void notify_error(Glib::ustring);
    
    private:
    bool dialog_visible;
    bool proc_lock;
    Glib::RefPtr<Gnome::Glade::Xml> xml_dialog;
    listener_list listeners;

    contact_data_vector candidate_srvs;
    int current_srv;
    VOMS_proxy_info infos;
    time_t last_proxy_update;
    
    const VOMS_config *config;
    
};
#endif
