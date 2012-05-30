/*
 * LICENSE TO BE DEFINED
 */

#include "voms-server-list.h"
#include "gtkmm/box.h"
#include "gtkmm/label.h"

#include <iostream>
#include <cstdlib>

VOMS_menu_item::VOMS_menu_item(Glib::ustring item_label, 
                               VOMS_server_list* parent) :
    Gtk::MenuItem::MenuItem(),
    contact_data_vector() {
    
    label = item_label;
    
    Gtk::HBox *hb = new Gtk::HBox();
    Gtk::Label *lab1 = new Gtk::Label(label);
    lab1->show();
    voms_ac_icon = new Gtk::Image(VOMS_icon_factory::disabled_icon());
    voms_ac_icon->show();
    hb->pack_start(*voms_ac_icon, false, true, 0);
    hb->pack_end(*lab1, true, true, 5);
    hb->show();
    
    add(*hb);
    show();
    
    sig_conn = this->signal_activate().connect(
                     sigc::mem_fun(*parent, &VOMS_server_list::activate_handler));
}

Glib::ustring VOMS_menu_item::get_VOMS_label(){
    return Glib::ustring(label);
}

void VOMS_menu_item::set_icon(Glib::ustring icon_path){
    voms_ac_icon->set(icon_path);
}

VOMS_menu_item::~VOMS_menu_item() {
    sig_conn.disconnect();
}




VOMS_server_list::VOMS_server_list(const VOMS_config& config,
                                   VOMS_client_engine& engine) :
    Gtk::Menu::Menu() {
        
    client_engine = &engine;
    client_config = &config;
    current_vo = engine.get_VO();
    icon_file = VOMS_icon_file();
    
    contacts_map = v_menu_map();
    last_list_modify = 0;
    
    reload();
    
    show();  
}

VOMS_server_list::~VOMS_server_list(){
}

void VOMS_server_list::activate_handler(){
    VOMS_menu_item *m_item = dynamic_cast<VOMS_menu_item*>(get_active());
    if ( m_item ) {
        client_engine->renew(*m_item);
    }
}

void VOMS_server_list::notify_ren_attempt(Glib::ustring voname, Glib::ustring host, int port) {
}

void VOMS_server_list::notify_gen_error(Glib::ustring err_msg) {
}

void VOMS_server_list::notify_renewed(Glib::ustring voname, Glib::ustring host, int port){

    if ( !current_vo.empty() ){
        contacts_map[current_vo]->set_icon(VOMS_icon_factory::disabled_icon());
    }
    
    current_vo = voname;
    contacts_map[current_vo]->set_icon(VOMS_icon_factory::first_icon());
}

void VOMS_server_list::notify_status(proxy_status_t status) {

    if ( contacts_map.find(current_vo) != contacts_map.end() 
         && icon_file.update(status)) {
        VOMS_menu_item *m_item = contacts_map[current_vo];
        m_item->set_icon(icon_file.path());
    }

}

bool VOMS_server_list::contains(Glib::ustring vo) {
    return contacts_map.find(vo) != contacts_map.end();
}

void VOMS_server_list::clean_all() {
    v_menu_map::iterator item_iter = contacts_map.begin();
    for(; item_iter != contacts_map.end(); item_iter++) {
        VOMS_menu_item* tmp_menu = item_iter->second;
        contacts_map.erase(item_iter);
        delete tmp_menu;
    }
}

void VOMS_server_list::reload(bool force) {

    if ( client_config->contacts_changes_since(last_list_modify) ) {
        last_list_modify = time(NULL) + timezone;
    } else if ( force ) {
        last_list_modify = time(NULL) + timezone;
        current_vo = client_engine->get_VO();
    } else {
        return;
    }
    
    if ( contacts_map.size()>0 ) {
        clean_all();
    }
    
    std::cout << "Reloading server list" << std::endl;
    
    contact_data_vector vomses_buffer = contact_data_vector();
    /*
     * TODO missing error handling
     */
    client_config->get_contacts(vomses_buffer);
    
    std::cout << "Recreating menu" << std::endl;
    
    for ( contact_data_vector::iterator buff_item = vomses_buffer.begin();
          buff_item != vomses_buffer.end();
          buff_item++ ) {
          
        if ( contacts_map.find(buff_item->vo) == contacts_map.end() ) {
            VOMS_menu_item* tmp_menu = new VOMS_menu_item(buff_item->vo, this);
            contacts_map[buff_item->vo] = tmp_menu;
            
            if ( !current_vo.empty() && current_vo == buff_item->vo ) {
                if ( icon_file.update(client_engine->get_proxy_status()) ) {
                    tmp_menu->set_icon(icon_file.path());
                }
            }
            
            this->items().push_back(*tmp_menu);
        }
        
        contacts_map[buff_item->vo]->push_back(*buff_item);
    }
    
    std::cout << "Menu ok" << std::endl;
    
}


