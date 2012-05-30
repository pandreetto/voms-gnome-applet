/*
 * LICENSE TO BE DEFINED
 */
 
#include "voms-client-engine.h"
#include "gtkmm/dialog.h"
#include "gtkmm/button.h"
#include "gtkmm/entry.h"
#include "gtkmm/combobox.h"
#include <glibmm/i18n.h>
#include <cstring>
#include <iostream>
#include <iomanip>

#include <fstream>
#include <cstdlib>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>

VOMS_proxy_info::VOMS_proxy_info() :
    mutex() {
    start_time = 0;
    end_time = 0;
    vo_name = "";
    host = "";
    port = 0;
}

VOMS_proxy_info::VOMS_proxy_info(contactdata& cdata, time_t st, time_t et) :
    mutex() {
    start_time = st;
    end_time = et;
    vo_name = cdata.vo;
    host = cdata.host;
    port = cdata.port;
}

VOMS_proxy_info& VOMS_proxy_info::operator=(const VOMS_proxy_info& info) {
    Glib::Mutex::Lock lock(mutex);
    start_time = info.start_time;
    end_time = info.end_time;
    vo_name = info.vo_name;
    host = info.host;
    port = info.port;
    return *this;
}

time_t VOMS_proxy_info::get_start_time() {
    Glib::Mutex::Lock lock(mutex);
    return start_time;
}

time_t VOMS_proxy_info::get_end_time() {
    Glib::Mutex::Lock lock(mutex);
    return end_time;
}

Glib::ustring VOMS_proxy_info::get_vo() {
    Glib::Mutex::Lock lock(mutex);
    return vo_name;
}

Glib::ustring VOMS_proxy_info::get_host() {
    Glib::Mutex::Lock lock(mutex);
    return host;
}

int VOMS_proxy_info::get_port() {
    Glib::Mutex::Lock lock(mutex);
    return port;
}

proxy_status_t VOMS_proxy_info::get_status() {
    Glib::Mutex::Lock lock(mutex);
    time_t now = time(NULL) + timezone;
    if ( end_time == 0 ) return PROXY_STATUS_DISABLED;
    if ( now > end_time ) return PROXY_STATUS_EXPIRED;
    return (now - start_time) * 100 / (end_time - start_time);
}

time_t VOMS_proxy_info::get_timeleft() {
    Glib::Mutex::Lock lock(mutex);
    time_t now = time(NULL) + timezone;
    time_t res =  end_time - now;
    return res >= 0 ? res : 0;
}



VOMS_client_engine::VOMS_client_engine(const VOMS_config& cfg) :
    infos() {

    config = &cfg;
    
    dialog_visible = false;
    proc_lock = false;
    listeners = listener_list();
    xml_dialog = Gnome::Glade::Xml::create(XML_UI_DIR "/basic.glade");
    last_proxy_update = 0;

    Gtk::Button *ren_btn = 0;
    Gtk::Button *canc_btn = 0;
    Gtk::Dialog* pDialog = 0;
    
    xml_dialog->get_widget(XML_DIALOG_LABEL, pDialog);
    pDialog->signal_delete_event().connect(sigc::mem_fun(*this, &VOMS_client_engine::hideDialog));

    xml_dialog->get_widget(XML_RENEWBTN_LABEL, ren_btn);
    ren_btn->signal_clicked().connect(sigc::mem_fun(*this, &VOMS_client_engine::apply_renew));
    
    xml_dialog->get_widget(XML_CANCBTN_LABEL, canc_btn);
    canc_btn->signal_clicked().connect(sigc::mem_fun(*this, &VOMS_client_engine::cancel_renew));

}

VOMS_client_engine::~VOMS_client_engine() {
}

bool VOMS_client_engine::hideDialog(GdkEventAny* dummy){
    cancel_renew();
    return TRUE;
}

void VOMS_client_engine::toggleDialog(){

    using namespace std;
    
    Gtk::Dialog* pDialog = 0;
    Gtk::Entry* pEntry = 0;
    Gtk::Label* pMessage = 0;
    Gtk::Label* pTimer = 0;
    Gtk::ComboBox* polCombo = 0;
    Gtk::ComboBox* hoursCombo = 0;
    Gtk::ComboBox* minutesCombo = 0;
    xml_dialog->get_widget(XML_DIALOG_LABEL, pDialog);
    xml_dialog->get_widget(XML_PWDENTRY_LABEL, pEntry);
    xml_dialog->get_widget(XML_MESSAGE_LABEL, pMessage);
    xml_dialog->get_widget(XML_TIMER_LABEL, pTimer);
    xml_dialog->get_widget(XML_POLICY_LABEL, polCombo);
    xml_dialog->get_widget(XML_ACHOURS_LABEL, hoursCombo);
    xml_dialog->get_widget(XML_ACMINUTES_LABEL, minutesCombo);
    
    if(dialog_visible){
        pDialog->hide();
        pDialog->set_modal(false);
        pEntry->set_text("");
    }else{
        contactdata* attempt_vomssrv = &candidate_srvs[current_srv];
        
        policy_modelcol p_model;
        Glib::RefPtr<Gtk::ListStore> tmp_store = Gtk::ListStore::create(p_model);
        
        int active_row = 0;
        int idx = 0;
        std::map<Glib::ustring, policy_data> policy_map;
        config->get_policies(policy_map);
        for(std::map<Glib::ustring, policy_data>::iterator item = policy_map.begin();
            item != policy_map.end();
            item++) {   
            Gtk::TreeModel::Row row = *(tmp_store->append());
            row[p_model.name] = item->first;
            if ( item->first == INHERITALL_LABEL ){
                active_row += idx;
            }
            idx++;
        }
        
        polCombo->set_model(tmp_store);
        polCombo->set_active(active_row);
        
        
        hoursCombo->set_active(config->get_ac_hours());
        minutesCombo->set_active(config->get_ac_minutes());
        
        pDialog->set_modal(true);
        pDialog->show();
        if ( attempt_vomssrv ) {
            stringstream buff;
            buff << "<span size=\"smaller\">" << _("Renew proxy for VO");
            buff << ": " << attempt_vomssrv->vo.c_str() << "</span>";
            pMessage->set_label(buff.str());

            if ( infos.get_vo() == attempt_vomssrv->vo ) {
                time_t tleft = infos.get_timeleft();
                std::stringstream buff2;
                buff2 << "<span size=\"smaller\">" << _("Timeleft") << ": ";
                buff2 << setfill('0') << setw(2) << ( tleft / 3600 );
                buff2 << ":" << setw(2) << ( (tleft % 3600) / 60 ) << "</span>";
                pTimer->set_label(buff2.str());
            } else {
                pTimer->set_label("");
            }
        }
    }
    dialog_visible = !dialog_visible;
}

bool VOMS_client_engine::processing(){
    return proc_lock;
}

void VOMS_client_engine::cancel_renew(){
    toggleDialog();
    proc_lock = false;
}

void VOMS_client_engine::apply_renew(){

    Gtk::Entry* pEntry = 0;
    Gtk::ComboBox* polCombo = 0;
    Gtk::ComboBox* hoursCombo = 0;
    Gtk::ComboBox* minutesCombo = 0;
    xml_dialog->get_widget(XML_PWDENTRY_LABEL, pEntry);
    xml_dialog->get_widget(XML_POLICY_LABEL, polCombo);
    xml_dialog->get_widget(XML_ACHOURS_LABEL, hoursCombo);
    xml_dialog->get_widget(XML_ACMINUTES_LABEL, minutesCombo);
    
    Glib::ustring curr_pass = pEntry->get_text();
    int current_achours = hoursCombo->get_active_row_number();
    int current_acminutes = minutesCombo->get_active_row_number() * MINUTES_SLOT;
    
    Glib::ustring pol_lang;
    Glib::ustring pol_data;
    Gtk::TreeModel::iterator iter = polCombo->get_active();
    if( iter and *iter ){
    
        policy_modelcol p_model;
        Glib::ustring name = (*iter)[p_model.name];
        std::map<Glib::ustring, policy_data> policy_map;
        config->get_policies(policy_map);
        
        std::map<Glib::ustring, policy_data>::iterator item = policy_map.find(name);
        if ( item == policy_map.end() ){
            throw VOMSError(VOMSERR_POLICY_UNDEF);
        }
        pol_lang = Glib::ustring(item->second.oid);
        pol_data = Glib::ustring(item->second.content);
    }else {
        pol_lang = Glib::ustring(INHERITALL_OID);
        pol_data = Glib::ustring("");
    }
    
    toggleDialog();
    bool success = false;
    
    try {
        proxy_manager tmp_client = proxy_manager(config->get_user_cert_path(),
                                                    config->get_user_key_path(),
                                                    curr_pass,
                                                    config->get_ca_dir(),
                                                    config->get_proxy_path(),
                                                    current_achours,
                                                    current_acminutes,
                                                    config->get_timeout(),
                                                    config->get_key_bit_len(),
                                                    config->get_proxy_type(),
                                                    pol_lang,
                                                    pol_data);
        /*
         * TODO use a random shift register for candidate selection
         */
        contact_data_vector::iterator candidate = candidate_srvs.begin();
        for(; candidate < candidate_srvs.end() && !success; candidate++) {
        
            listener_list::iterator item = listeners.begin();
            listener_list::iterator end_item = listeners.end();
            for(; item < end_item; item++){
                (*item)->notify_ren_attempt(candidate->vo, candidate->host, 
                                            candidate->port);
            }

            try {
                struct stat st;
                
                tmp_client.build_proxy(*candidate);
                infos = VOMS_proxy_info(candidate_srvs[current_srv],
                                        tmp_client.get_proxy_starttime(),
                                        tmp_client.get_proxy_endtime());

                if ( stat(config->get_proxy_path().c_str(), &st) != 0 ) {
                    throw VOMSError(VOMSERR_BAD_CRED_FILES);
                }
                last_proxy_update = st.st_mtime;

                listener_list::iterator item = listeners.begin();
                for(; item < listeners.end(); item++){
                    (*item)->notify_renewed(candidate->vo, candidate->host, 
                                            candidate->port);
                }

                success = true;
                
            } catch(VOMSError err) {
            
                if ( err.is_blocking() ) {
                    throw err;
                }
                current_srv++;
            }
            
        }
    
        if ( !success ) {
            throw VOMSError(VOMSERR_CANNOT_RENEW);
        }
    
    } catch (VOMSError err) {
        Glib::ustring err_msg(err.what());
        listener_list::iterator item = listeners.begin();
        for(; item < listeners.end(); item++){
            (*item)->notify_gen_error(err_msg);
        }

    }

    proc_lock = false;

}

int VOMS_client_engine::renew(contact_data_vector& voms_srv_info){
    /*
     * TODO check list replacement
     */
    candidate_srvs = contact_data_vector(voms_srv_info);
    current_srv = 0;
    proc_lock = true;
    toggleDialog();
    /*
     * TODO check return value
     */
    return 0;
}

void VOMS_client_engine::add_listener(VOMS_client_listener& lstn) {
    listeners.push_back(&lstn);
}


time_t VOMS_client_engine::get_timeleft() {
    return infos.get_timeleft();
}

Glib::ustring VOMS_client_engine::get_VO() {
    return infos.get_vo();
}

Glib::ustring VOMS_client_engine::get_host() {
    return infos.get_host();
}

int VOMS_client_engine::get_port() {
    return infos.get_port();
}

proxy_status_t VOMS_client_engine::get_proxy_status() {
    return infos.get_status();
}

void VOMS_client_engine::notify_error(Glib::ustring err_msg) {
    listener_list::iterator item = listeners.begin();
    for(; item < listeners.end(); item++){
        (*item)->notify_gen_error(err_msg);
    }
}

int VOMS_client_engine::check_proxy() {

    struct stat st;

    if ( stat(config->get_proxy_path().c_str(), &st) != 0 ) {
        /*
         * TODO more specific error codes
         */
        if ( errno == ENOENT ) {
            return CHK_PRX_NOENT;
        }
        
        return CHK_PRX_ERROR;
    }
    
    if ( S_IFREG != (st.st_mode & S_IFREG) ) {
        return CHK_PRX_ERROR;
    }
    
    time_t curr_prx_ts = st.st_mtime;
    
    if ( curr_prx_ts <= last_proxy_update ) return CHK_PRX_NOCHANGES;
    
    try {
        proxy_manager tmp_client = proxy_manager(config->get_user_cert_path(),
                                                 config->get_user_key_path(),
                                                 "",
                                                 config->get_ca_dir(),
                                                 config->get_proxy_path(),
                                                 config->get_ac_hours(),
                                                 config->get_ac_minutes(),
                                                 config->get_timeout(),
                                                 config->get_key_bit_len(),
                                                 config->get_proxy_type(),
                                                 "", "",
                                                 true);
        tmp_client.verify_proxy();
        /*
         * TODO extract host and port from voms.uri
         */
        contactdata tmpdata = {"", "", "", tmp_client.get_vo_id(), 0, 0};
        infos = VOMS_proxy_info(tmpdata,
                                tmp_client.get_proxy_starttime(),
                                tmp_client.get_proxy_endtime());
        last_proxy_update = curr_prx_ts;

    } catch (VOMSError err) {
        std::cerr << "Exception: " << err.what() << std::endl;
        if ( err.get_error_code() == VOMSERR_EXP_PROXY )
            return CHK_PRX_EXPIRED;
        return CHK_PRX_ERROR;
    }
    
    return CHK_PRX_RELOADED;

}


