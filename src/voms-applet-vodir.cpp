/*
 * LICENSE TO BE DEFINED
 */

#include "voms-applet-config.h"
#include "gtkmm/treeview.h"
#include "gtkmm/filefilter.h"
#include "gtkmm/filechooserwidget.h"

#include <glibmm/i18n.h>

#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>

void VOMS_config::init_vodir() {

    Gtk::TreeView* auth_vo_view = 0;
    Gtk::Button *imp_lsc_btn = 0;
    Gtk::Button *imp_pem_btn = 0;
    Gtk::Button *del_vo_btn = 0;
    Gtk::Dialog *top_dialog = 0;
    Gtk::FileChooserDialog *a_dialog = 0;
    Gtk::Button *canc_btn = 0;
    Gtk::Button *open_btn = 0;

    auth_vo_store = Gtk::ListStore::create(auth_vo_columns);
    prefs_dialog->get_widget(XMLPREF_AUTHVO_TAG, auth_vo_view);
    auth_vo_view->set_model(auth_vo_store);
    auth_vo_view->append_column(_("VO name"), auth_vo_columns.vo_name);
    auth_vo_view->append_column(_("VO server DN"), auth_vo_columns.vo_dn);
    
    prefs_dialog->get_widget(XMLPREF_AUTHVO_LSC_TAG, imp_lsc_btn);
    prefs_dialog->get_widget(XMLPREF_AUTHVO_PEM_TAG, imp_pem_btn);
    prefs_dialog->get_widget(XMLPREF_AUTHVO_DEL_TAG, del_vo_btn);
    
    auth_vo_dialog = Gnome::Glade::Xml::create(XML_UI_DIR "/" XMLPREF_AUTHVO_FILE);
    prefs_dialog->get_widget(XMLPREF_DIALOG_TAG, top_dialog);
    auth_vo_dialog->get_widget(XMLPREF_AUTHVO_DIAL_TAG, a_dialog);
    auth_vo_dialog->get_widget(XMLPREF_AUTHVO_CANC_TAG, canc_btn);
    auth_vo_dialog->get_widget(XMLPREF_AUTHVO_OPEN_TAG, open_btn);
    open_btn->signal_clicked().connect(
                               sigc::mem_fun(*this, &VOMS_config::import_lsc_file));
    canc_btn->signal_clicked().connect(
                               sigc::mem_fun(*a_dialog, &Gtk::Dialog::hide));

    imp_lsc_btn->signal_clicked().connect(
                                  sigc::mem_fun(*a_dialog, &Gtk::Dialog::show));
    del_vo_btn->signal_clicked().connect(
                                 sigc::mem_fun(*this, &VOMS_config::del_selected_auth_vo));

    Gtk::FileFilter lsc_filter;
    lsc_filter.set_name(_("LSC files"));
    lsc_filter.add_pattern("*.lsc");
    a_dialog->add_filter(lsc_filter);
    a_dialog->set_transient_for(*top_dialog);

    if ( getenv("VOMSDIR") ) {
    
        internal_vodir = getenv("VOMSDIR");
        
    } else  if ( getenv("HOME") ) {
    
        Glib::ustring top_dir = getenv("HOME");
        top_dir = top_dir + "/.voms-gnome-applet";
        if ( mkdir(top_dir.c_str(), S_IRUSR | S_IWUSR | S_IXUSR) < 0 && errno != EEXIST ) {
            /*
             * TODO handling error
             */
        }
        
        internal_vodir = top_dir + "/vomsdir";
    }
    
    if ( mkdir(internal_vodir.c_str(), S_IRUSR | S_IWUSR | S_IXUSR) < 0 && errno != EEXIST ) {
            /*
             * TODO handling error
             */
    }
    
}

void VOMS_config::import_lsc_file() {
    Gtk::Dialog *a_dialog = 0;
    auth_vo_dialog->get_widget(XMLPREF_AUTHVO_DIAL_TAG, a_dialog);
    a_dialog->hide();
    std::cout << "Import selected vo" << std::endl;
}

void VOMS_config::del_selected_auth_vo() {
    std::cout << "Delete selected vo" << std::endl;
}

void VOMS_config::edit_auth_vo_prefs() {

    auth_vo_store->clear();

    DIR *top_dir = opendir(internal_vodir.c_str());
    if ( top_dir == NULL ) {
        /*
         * TODO error handling
         */
    }
    
    struct dirent *top_entry;
    while( (top_entry = readdir(top_dir)) != NULL ) {
    
        if ( top_entry->d_name[0] == '.' ) continue;
        
        struct stat st;
        Glib::ustring inner_dir = internal_vodir + '/' + top_entry->d_name;
        if ( stat(inner_dir.c_str(), &st) != 0
             || S_IFDIR != (st.st_mode & S_IFDIR) ) continue;
        
        DIR *in_dir = opendir(inner_dir.c_str());
        /*
         * TODO error handling
         */
        struct dirent *in_entry;
        while( (in_entry = readdir(in_dir)) != NULL ) {
        
            if ( !strstr(in_entry->d_name, ".lsc") ) continue;
            
            bool missing_dn = true;
            std::ifstream lsc_file;
            Glib::ustring lsc_filename = inner_dir + '/' + in_entry->d_name;
            lsc_file.open(lsc_filename.c_str(), std::ifstream::in);
            while ( lsc_file.good() ) {
                /*
                 * TODO use >> operator
                 */
                char tmps[1024];
                memset(tmps, 0 , 1024);
                lsc_file.getline(tmps, 1023);
                if ( strlen(tmps) == 0 ) continue;
                
                Glib::ustring vo_dn = tmps;
                
                memset(tmps, 0 , 1024);
                lsc_file.getline(tmps, 1023);
                if ( strlen(tmps) == 0 ) continue;
                
                Glib::ustring issuer_dn = tmps;
                
                Gtk::TreeModel::Row row = *(auth_vo_store->append());
                row[auth_vo_columns.vo_name] = top_entry->d_name;
                row[auth_vo_columns.vo_dn] = vo_dn;
                row[auth_vo_columns.issuer_dn] = issuer_dn;
                row[auth_vo_columns.lsc_file] = in_entry->d_name;
            }
        }
        
        closedir(in_dir);
    
    }
    
    closedir(top_dir);
}

void VOMS_config::apply_auth_vo_prefs() {

}


