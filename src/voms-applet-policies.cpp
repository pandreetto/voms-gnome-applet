/*
 * LICENSE TO BE DEFINED
 */

#include "voms-applet-config.h"
#include "gtkmm/dialog.h"
#include "gtkmm/filechooserbutton.h"
#include "gtkmm/combobox.h"
#include "gtkmm/spinbutton.h"
#include "gtkmm/treeview.h"
#include "gtkmm/textview.h"

#include <iostream>
#include <unistd.h>


void VOMS_config::init_policies() {

    Gtk::TreeView* policies_view = 0;
    Gtk::Dialog *p_dialog = 0;
    Gtk::Button *ok_btn = 0;
    Gtk::Button *canc_btn = 0;
    Gtk::Button *add_btn = 0;
    Gtk::Button *edit_btn = 0;
    Gtk::Button *del_btn = 0;
    
    prefs_dialog->get_widget(XMLPREF_POLICIES_TAG, policies_view);
    policy_store = Gtk::ListStore::create(policy_columns);
    policies_view->set_model(policy_store);
    policies_view->append_column("Name", policy_columns.name);
    policies_view->append_column("Language", policy_columns.language);
    policies_view->append_column("Policy", policy_columns.policy);
    
    policy_dialog = Gnome::Glade::Xml::create(XML_UI_DIR "/" XMLPREF_POLICY_FILE);
    policy_dialog->get_widget(XMLPREF_POLI_DIAL_TAG, p_dialog);
    policy_dialog->get_widget(XMLPREF_POLI_OK_TAG, ok_btn);
    policy_dialog->get_widget(XMLPREF_POLI_CANC_TAG, canc_btn);
    prefs_dialog->get_widget(XMLPREF_POLI_ADD_TAG, add_btn);
    prefs_dialog->get_widget(XMLPREF_POLI_EDIT_TAG, edit_btn);
    prefs_dialog->get_widget(XMLPREF_POLI_DEL_TAG, del_btn);
    
    ok_btn->signal_clicked().connect(
                             sigc::mem_fun(*this, &VOMS_config::insert_policy));
    canc_btn->signal_clicked().connect(sigc::mem_fun(*p_dialog, &Gtk::Dialog::hide));
    add_btn->signal_clicked().connect(
                              sigc::mem_fun(*this, &VOMS_config::popup_new_policy));
    edit_btn->signal_clicked().connect(
                               sigc::mem_fun(*this, &VOMS_config::popup_selected_policy));
    del_btn->signal_clicked().connect(
                              sigc::mem_fun(*this, &VOMS_config::del_selected_policy));
    
    list_policies_modify = true;
    poli_add_mode = true;
}


/*
 * TODO improve parsing checks
 *      missing error handling
 */
void VOMS_config::get_policies(std::map<Glib::ustring, policy_data> &policy_list) const {
    if ( policy_list.size() > 0 ) {
        policy_list.clear();
    }
    
    policy_data inherit_pol = {INHERITALL_OID, ""};
    policy_data indep_pol = {INDEPENDENT_OID, ""};
    policy_list[INHERITALL_LABEL] = inherit_pol;
    policy_list[INDEPENDENT_LABEL] = indep_pol;
    
    Gnome::Conf::SListHandle_ValueString g_list 
        = g_client->get_string_list(VOMS_GCONF_LOCATION "/" VOMS_GCONF_POLICIES);
    for ( Gnome::Conf::SListHandle_ValueString::iterator item = g_list.begin();
          item != g_list.end();
          item++ ){
        Glib::ustring tmps = Glib::ustring(*item);
        Glib::ustring p_name = "";
        policy_data tmp_data;

        std::string::size_type p1 = 0;
        std::string::size_type p2 = 0;
        int idx = 0;
        while ( p2 != std::string::npos ) {
            p2 = tmps.find(GLIST_SEP,p1);
            
            if ( p2 != std::string::npos ) {
                switch ( idx ) {
                case 0:
                    p_name = tmps.substr(p1,p2-p1);
                    break;
                case 1:
                    tmp_data.oid = tmps.substr(p1,p2-p1);
                }
                p1 = p2 + 1;
                idx++;
            }
        }
        tmp_data.content = tmps.substr(p1,p2-p1);
        policy_list[p_name] = tmp_data;
    }
}

void VOMS_config::edit_policies_prefs() {
    
    if ( list_policies_modify ) {
        
        policy_store->clear();
        
        std::map<Glib::ustring, policy_data> policy_map;
        get_policies(policy_map);
        for(std::map<Glib::ustring, policy_data>::iterator item = policy_map.begin();
            item != policy_map.end();
            item++) {
          
            if( item->first == INHERITALL_LABEL 
                || item->first == INDEPENDENT_LABEL ) continue;
        
            Gtk::TreeModel::Row row = *(policy_store->append());
            row[policy_columns.name] = item->first;
            row[policy_columns.language] = item->second.oid;
            row[policy_columns.policy] = item->second.content;
        }
        
        list_policies_modify = false;
    }
}

void VOMS_config::apply_policies_prefs() {

    std::vector<Glib::ustring> gconf_policies;
    
    for ( Gtk::TreeModel::iterator item = policy_store->children().begin();
          item != policy_store->children().end();
          item++ ) {
          
        Gtk::TreeModel::Row row = *item;
        std::stringstream buff;
        buff << row[policy_columns.name] << GLIST_SEP;
        buff << row[policy_columns.language] << GLIST_SEP;
        buff << row[policy_columns.policy];
        
        gconf_policies.push_back(buff.str());
    }
    
    g_client->set_string_list(VOMS_GCONF_LOCATION "/" VOMS_GCONF_POLICIES, gconf_policies);
    
    list_policies_modify = true;
}

void VOMS_config::insert_policy() {
    Gtk::Dialog *p_dialog = 0;
    Gtk::TreeView* policies_view = 0;
    Gtk::Entry* name_entry = 0;
    Gtk::Entry* lang_entry = 0;
    Gtk::TextView* policy_text = 0;
    
    prefs_dialog->get_widget(XMLPREF_POLICIES_TAG, policies_view);
    policy_dialog->get_widget(XMLPREF_POLI_DIAL_TAG, p_dialog);
    policy_dialog->get_widget(XMLPREF_POLI_NAME_TAG, name_entry);
    policy_dialog->get_widget(XMLPREF_POLI_LANG_TAG, lang_entry);
    policy_dialog->get_widget(XMLPREF_POLI_DATA_TAG, policy_text);

    /*
     * TODO check input
     */

    Glib::RefPtr<Gtk::TreeSelection> t_sel = policies_view->get_selection();
    if ( ( !t_sel || t_sel->count_selected_rows() == 0 ) && ! poli_add_mode ) return;
    
    Gtk::TreeModel::Row row = poli_add_mode ? *(policy_store->append()) : *(t_sel->get_selected());
    
    row[policy_columns.name] = name_entry->get_text();
    row[policy_columns.language] = lang_entry->get_text();
    row[policy_columns.policy] = policy_text->get_buffer()->get_text();
    
    p_dialog->hide();

}

void VOMS_config::fillin_policy_popup() {
    Gtk::Dialog *top_dialog = 0;
    Gtk::Dialog *p_dialog = 0;
    
    prefs_dialog->get_widget(XMLPREF_DIALOG_TAG, top_dialog);
    policy_dialog->get_widget(XMLPREF_POLI_DIAL_TAG, p_dialog);
    
    p_dialog->set_modal();
    p_dialog->set_transient_for(*top_dialog);
    
    p_dialog->show();
}

void VOMS_config::popup_new_policy() {
    poli_add_mode = true;
    fillin_policy_popup();
}

void VOMS_config::popup_selected_policy(){
    poli_add_mode = false;
    fillin_policy_popup();
}

void VOMS_config::del_selected_policy() {
    Gtk::TreeView* policies_view = 0;
    prefs_dialog->get_widget(XMLPREF_POLICIES_TAG, policies_view);
    
    Glib::RefPtr<Gtk::TreeSelection> t_sel = policies_view->get_selection();
    if ( !t_sel || t_sel->count_selected_rows() == 0 ) return;
    policy_store->erase(t_sel->get_selected());
}




