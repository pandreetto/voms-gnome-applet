/*
 * LICENSE TO BE DEFINED
 */

#include "voms-applet-config.h"
#include "gtkmm/dialog.h"
#include "gtkmm/filechooserbutton.h"
#include "gtkmm/combobox.h"
#include "gtkmm/spinbutton.h"
#include "gtkmm/treeview.h"

#include <glibmm/i18n.h>

#include <iostream>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <cstring>

void VOMS_config::init_contacts() {

    Gtk::TreeView *contacts_view = 0;
    Gtk::Dialog *c_dialog = 0;
    Gtk::Button *add_btn = 0;
    Gtk::Button *edit_btn = 0;
    Gtk::Button *del_btn = 0;
    Gtk::Button *ok_btn = 0;
    Gtk::Button *canc_btn = 0;
    Gtk::ComboBox *vodnCombo = 0;
    
    prefs_dialog->get_widget(XMLPREF_CONTACTS_TAG, contacts_view);
    contact_store = Gtk::ListStore::create(contact_columns);
    contacts_view->set_model(contact_store);
    contacts_view->append_column("Attribute", contact_columns.attr);
    contacts_view->append_column("Host", contact_columns.host);
    contacts_view->append_column_numeric("Port", contact_columns.port, "%05d");
    contacts_view->append_column("DN", contact_columns.dn);
    contacts_view->append_column_numeric("Version", contact_columns.version, "%02d");
    
    list_contacts_modify = true;
    contacts_modify_ts = 0;
    cntc_add_mode = true;
    
    contact_dialog = Gnome::Glade::Xml::create(XML_UI_DIR "/" XMLPREF_CONTACT_FILE);
    contact_dialog->get_widget(XMLPREF_CNTC_DIAL_TAG, c_dialog);
    prefs_dialog->get_widget(XMLPREF_CNTC_ADD_TAG, add_btn);
    prefs_dialog->get_widget(XMLPREF_CNTC_EDIT_TAG, edit_btn);
    prefs_dialog->get_widget(XMLPREF_CNTC_DEL_TAG, del_btn);
    contact_dialog->get_widget(XMLPREF_CNTC_OK_TAG, ok_btn);
    contact_dialog->get_widget(XMLPREF_CNTC_CANC_TAG, canc_btn);
    contact_dialog->get_widget(XMLPREF_CNTC_VODN_TAG, vodnCombo);
    
    vodn_store = Gtk::ListStore::create(vodn_columns);
    vodnCombo->set_model(vodn_store);
    vodnCombo->pack_start(vodn_columns.vo);
    vodnCombo->pack_start(vodn_columns.dn);
    
    add_btn->signal_clicked().connect(
                              sigc::mem_fun(*this, &VOMS_config::popup_new_contact));
    edit_btn->signal_clicked().connect(
                               sigc::mem_fun(*this, &VOMS_config::popup_selected_contact));
    del_btn->signal_clicked().connect(
                              sigc::mem_fun(*this, &VOMS_config::del_selected_contact));
    ok_btn->signal_clicked().connect(
                             sigc::mem_fun(*this, &VOMS_config::insert_contact));
    canc_btn->signal_clicked().connect(sigc::mem_fun(*c_dialog, &Gtk::Dialog::hide));
}


/*
 * TODO improve parsing checks
 *      missing error handling
 */
void VOMS_config::get_contacts(std::vector<contactdata> &contact_list) const {

    if ( contact_list.size() > 0 ) {
        contact_list.clear();
    }
    
    Gnome::Conf::SListHandle_ValueString g_list 
        = g_client->get_string_list(VOMS_GCONF_LOCATION "/" VOMS_GCONF_CONTACTS);
    for ( Gnome::Conf::SListHandle_ValueString::iterator item = g_list.begin();
          item != g_list.end();
          item++ ){
        Glib::ustring tmps = Glib::ustring(*item);
        contactdata tmp_data;

        std::string::size_type p1 = 0;
        std::string::size_type p2 = 0;
        int idx = 0;
        while ( p2 != std::string::npos ) {
            p2 = tmps.find(GLIST_SEP,p1);
            
            if ( p2 != std::string::npos ) {
                switch ( idx ) {
                case 0:
                    tmp_data.nick = tmps.substr(p1,p2-p1);
                    break;
                case 1:
                    tmp_data.host = tmps.substr(p1,p2-p1);
                    break;
                case 2:
                    tmp_data.port = atoi(tmps.substr(p1,p2-p1).c_str());
                case 3:
                    tmp_data.contact = tmps.substr(p1,p2-p1);
                    break;
                case 4:
                    tmp_data.vo = tmps.substr(p1,p2-p1);
                }
                p1 = p2 + 1;
                idx++;
            }
        }
        tmp_data.version = atoi(tmps.substr(p1,p2-p1).c_str());
        contact_list.push_back(tmp_data);
    }
}

void VOMS_config::edit_contacts_prefs() {
    
    if ( list_contacts_modify ) {
        
        contact_store->clear();
        
        std::vector<contactdata> tmp_list = std::vector<contactdata>();
        get_contacts(tmp_list);
        for ( std::vector<contactdata>::iterator c_item = tmp_list.begin();
              c_item != tmp_list.end();
              c_item++ ) {
            Gtk::TreeModel::Row row = *(contact_store->append());
            row[contact_columns.attr] = c_item->vo;
            row[contact_columns.host] = c_item->host;
            row[contact_columns.port] = c_item->port;
            row[contact_columns.dn] = c_item->contact;
            row[contact_columns.version] = c_item->version; 
        }
        
        list_contacts_modify = false;
    }
}

void VOMS_config::apply_contacts_prefs() {

    std::vector<Glib::ustring> gconf_contacts;
    
    for ( Gtk::TreeModel::iterator item = contact_store->children().begin();
          item != contact_store->children().end();
          item++ ) {
          
        Gtk::TreeModel::Row row = *item;
        std::stringstream buff;
        /*
         * TODO generate a good nickname
         */
        buff << row[contact_columns.attr] << GLIST_SEP;  //nickname
        buff << row[contact_columns.host] << GLIST_SEP;
        buff << row[contact_columns.port] << GLIST_SEP;
        buff << row[contact_columns.dn] << GLIST_SEP;
        buff << row[contact_columns.attr] << GLIST_SEP;
        buff << row[contact_columns.version];
          
        gconf_contacts.push_back(buff.str());
    }
    
    g_client->set_string_list(VOMS_GCONF_LOCATION "/" VOMS_GCONF_CONTACTS, gconf_contacts);

    list_contacts_modify = true;
    contacts_modify_ts = time(NULL) + timezone;
}

void VOMS_config::fillin_contact_popup() {
    Gtk::Dialog *top_dialog = 0;
    Gtk::Dialog *c_dialog = 0;
    Gtk::TreeView* contacts_view = 0;
    Gtk::ComboBox* serviceCombo = 0;
    Gtk::Entry* roleEntry = 0;
    Gtk::Entry* hostEntry = 0;
    Gtk::SpinButton* portBtn = 0;
    
    prefs_dialog->get_widget(XMLPREF_DIALOG_TAG, top_dialog);
    prefs_dialog->get_widget(XMLPREF_CONTACTS_TAG, contacts_view);
    contact_dialog->get_widget(XMLPREF_CNTC_DIAL_TAG, c_dialog);
    contact_dialog->get_widget(XMLPREF_CNTC_VODN_TAG, serviceCombo);
    contact_dialog->get_widget(XMLPREF_CNTC_ROLE_TAG, roleEntry);
    contact_dialog->get_widget(XMLPREF_CNTC_HOST_TAG, hostEntry);
    contact_dialog->get_widget(XMLPREF_CNTC_PORT_TAG, portBtn);
    
    c_dialog->set_modal();
    c_dialog->set_transient_for(*top_dialog);
    
    int num_of_rows = fillin_vodn_combo();
    
    if ( num_of_rows == 0 ) {
        err_dialog->set_message(_("No VO available"));
        err_dialog->show();
        return;
    }

    if ( cntc_add_mode ) {
    
        if ( num_of_rows ) serviceCombo->set_active(0);
        roleEntry->set_text("");
        hostEntry->set_text("");
        portBtn->set_value(15000);
        
        c_dialog->show();
        
    } else {
        Glib::RefPtr<Gtk::TreeSelection> t_sel = contacts_view->get_selection();
        if ( t_sel && t_sel->count_selected_rows() ) {
        
            Gtk::TreeModel::Row row = *(t_sel->get_selected());
            /*
             * TODO show the correct vo-dn in combobox
             */
            roleEntry->set_text(row[contact_columns.attr]);
            hostEntry->set_text(row[contact_columns.host]);
            portBtn->set_value(row[contact_columns.port]);
        
            c_dialog->show();
            
        } else {
            std::cerr << "No selection available" << std::endl;
        }
    }
}

void VOMS_config::popup_new_contact() {
    cntc_add_mode = true;
    fillin_contact_popup();
}

void VOMS_config::popup_selected_contact() {
    cntc_add_mode = false;
    fillin_contact_popup();
}

void VOMS_config::del_selected_contact() {
    Gtk::TreeView* contacts_view = 0;
    prefs_dialog->get_widget(XMLPREF_CONTACTS_TAG, contacts_view);
    
    Glib::RefPtr<Gtk::TreeSelection> t_sel = contacts_view->get_selection();
    if ( !t_sel || t_sel->count_selected_rows() == 0 ) return;
    contact_store->erase(t_sel->get_selected());
}

void VOMS_config::insert_contact() {
    Gtk::Dialog *c_dialog = 0;
    Gtk::TreeView* contacts_view = 0;
    Gtk::ComboBox* vodnCombo = 0;
    Gtk::Entry* roleEntry = 0;
    Gtk::Entry* hostEntry = 0;
    Gtk::SpinButton* portBtn = 0;
    
    prefs_dialog->get_widget(XMLPREF_CONTACTS_TAG, contacts_view);
    contact_dialog->get_widget(XMLPREF_CNTC_DIAL_TAG, c_dialog);
    contact_dialog->get_widget(XMLPREF_CNTC_VODN_TAG, vodnCombo);
    contact_dialog->get_widget(XMLPREF_CNTC_ROLE_TAG, roleEntry);
    contact_dialog->get_widget(XMLPREF_CNTC_HOST_TAG, hostEntry);
    contact_dialog->get_widget(XMLPREF_CNTC_PORT_TAG, portBtn);
    
    /*
     * TODO check input
     */
    
    Glib::RefPtr<Gtk::TreeSelection> t_sel = contacts_view->get_selection();
    if ( ( !t_sel || t_sel->count_selected_rows() == 0 ) && ! cntc_add_mode ) return;
    
    Gtk::TreeModel::Row row = cntc_add_mode ? *(contact_store->append()) : *(t_sel->get_selected());
    Gtk::TreeModel::Row vodn_row = *(vodnCombo->get_active());
    
    if ( roleEntry->get_text().empty() ) {
        row[contact_columns.attr] = (Glib::ustring)vodn_row[vodn_columns.vo];  // improve cast
    } else {
        row[contact_columns.attr] = vodn_row[vodn_columns.vo] + ": " + roleEntry->get_text();
    }
    row[contact_columns.host] = hostEntry->get_text();
    row[contact_columns.port] = portBtn->get_value_as_int();
    row[contact_columns.dn] = (Glib::ustring) vodn_row[vodn_columns.dn];    // improve cast
    row[contact_columns.version] = 24;
    
    
    c_dialog->hide();
}

bool VOMS_config::contacts_changes_since(double tstamp) const {

    return contacts_modify_ts >= tstamp;
}

int VOMS_config::fillin_vodn_combo() {
    if ( vodn_store->children().size() > 0 ) {
        vodn_store->clear();
    }
    
    int num_of_rows = 0;

    Gtk::FileChooserButton* vodir_entry = 0;
    prefs_dialog->get_widget(XMLPREF_VODIR_TAG, vodir_entry);
    Glib::ustring vomsdir = vodir_entry->get_filename();

    DIR *dir = opendir(vomsdir.c_str());
    if ( dir == NULL ) {
        std::cerr << "Error reading voms dir" << std::endl;
        return num_of_rows;
    }
    
    struct dirent *entry;
    while( (entry = readdir(dir)) != NULL ) {
    
        if ( entry->d_name[0] == '.' ) continue;
        
        struct stat st;
        Glib::ustring inner_dir = vomsdir + '/' + entry->d_name;
        if ( stat(inner_dir.c_str(), &st) != 0  
             || S_IFDIR != (st.st_mode & S_IFDIR) ) continue;
        
        DIR *i_dir = opendir(inner_dir.c_str());
        struct dirent *entry2;
        while( (entry2 = readdir(i_dir)) != NULL ) {
            if ( !strstr(entry2->d_name, ".lsc") ) continue;
            
            bool missing_dn = true;
            std::ifstream lsc_file;
            Glib::ustring lsc_filename = inner_dir + '/' + entry2->d_name;
            lsc_file.open(lsc_filename.c_str(), std::ifstream::in);
            while ( lsc_file.good() ) {
                char tmps[1024];
                memset(tmps, 0 , 1024);
                lsc_file.getline(tmps, 1023);
                if ( strlen(tmps) && missing_dn) {
                
                    Gtk::TreeModel::Row row = *(vodn_store->append());
                    row[vodn_columns.vo] = entry->d_name;
                    row[vodn_columns.dn] = tmps;
                    num_of_rows++;
                    missing_dn = false;
                }
            }
        }
        
        closedir(i_dir);
    }

    closedir(dir);
    return num_of_rows;
}



