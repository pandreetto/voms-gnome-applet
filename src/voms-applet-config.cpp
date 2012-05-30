/*
 * LICENSE TO BE DEFINED
 */

#include "voms-applet-config.h"
#include "gtkmm/dialog.h"
#include "gtkmm/filechooserbutton.h"
#include "gtkmm/combobox.h"
#include "gtkmm/spinbutton.h"
#include "gtkmm/treeview.h"
#include "gtkmm/notebook.h"

#include <glibmm/i18n.h>

#include <iostream>
#include <unistd.h>

extern "C" {
static int idx2bitlen(int idx) {
    switch (idx) {
    case 0:
        return 512;
    case 1:
        return 1024;
    case 2:
        return 2048;
    case 3:
        return 4096;
    }
    return 1024;
}

static int bitlen2idx(int bitlen) {
    switch (bitlen) {
    case 512:
        return 0;
    case 1024:
        return 1;
    case 2048:
        return 2;
    case 4096:
        return 3;
    }
    return 1;
}
}




VOMSBrowserDialog::VOMSBrowserDialog(Gtk::Window& parent, Gtk::Entry& b_entry) :
    Gtk::FileChooserDialog(parent, "Save As", Gtk::FILE_CHOOSER_ACTION_SAVE) {
    
    cancel_btn = add_button("Cancel", 0);
    saveas_btn = add_button("Save as", 1);
    bound_entry = &b_entry;
    
    this->signal_response().connect(
          sigc::mem_fun(*this, &VOMSBrowserDialog::on_response));
    
}

VOMSBrowserDialog::~VOMSBrowserDialog(){
    delete saveas_btn;
    delete cancel_btn;
}

void VOMSBrowserDialog::on_response(int response_id) {
    if ( response_id == 1 ) {
        bound_entry->set_text(get_filename());
    }
    
    hide();
}




VOMS_config::VOMS_config(bool standalone) {
    Gnome::Conf::init();
    
    g_client = Gnome::Conf::Client::get_default_client();
    prefs_dialog = Gnome::Glade::Xml::create(XML_UI_DIR "/" XMLPREF_PREFS_FILE);
    
    wizard_mode = !configured();
    
    Gtk::Window *top_dialog = 0;
    Gtk::Button *apply_btn = 0;
    Gtk::Button *fwd_btn = 0;
    Gtk::Button *prev_btn = 0;
    Gtk::Button *canc_btn = 0;
    Gtk::Button * browse_btn = 0;
    Gtk::Entry* proxy_entry = 0;
    
    prefs_dialog->get_widget(XMLPREF_APPLYBTN_LABEL, apply_btn);
    apply_btn->signal_clicked().connect(sigc::mem_fun(*this, &VOMS_config::apply_changes));
    prefs_dialog->get_widget(XMLPREF_CANCBTN_LABEL, canc_btn);
    canc_btn->signal_clicked().connect(sigc::mem_fun(*this, &VOMS_config::cancel_changes));
    
    prefs_dialog->get_widget(XMLPREF_FWDBTN_LABEL, fwd_btn);
    prefs_dialog->get_widget(XMLPREF_PREVBTN_LABEL, prev_btn);
    
    if ( !wizard_mode ) {
        Gtk::Label *descr_label = 0;
        prefs_dialog->get_widget(XMLPREF_GENDESCR_LABEL, descr_label);
        descr_label->hide();

        fwd_btn->hide();
        prev_btn->hide();
    } else {
        Gtk::Notebook *notebook = 0;
        prefs_dialog->get_widget(XMLPREF_NOTEBOOK_LABEL, notebook);
        notebook->set_show_tabs(false);
        moveto_tab(0);
        
        apply_btn->set_sensitive(false);
        prev_btn->set_sensitive(false);
        fwd_btn->signal_clicked().connect(sigc::mem_fun(*this, &VOMS_config::next_tab));
        prev_btn->signal_clicked().connect(sigc::mem_fun(*this, &VOMS_config::prev_tab));
    }
    prefs_dialog->get_widget(XMLPREF_PROXY_FILE_TAG, proxy_entry);
    proxy_entry->set_text(get_proxy_path());
    prefs_dialog->get_widget(XMLPREF_DIALOG_TAG, top_dialog);
    browser_dialog = new VOMSBrowserDialog(*top_dialog, *proxy_entry);
    prefs_dialog->get_widget(XMLPREF_PROXY_BROW_TAG, browse_btn);
    browse_btn->signal_clicked().connect(
                sigc::mem_fun(*browser_dialog, &VOMSBrowserDialog::show));
    
    init_vodir();

    init_contacts();
    
    init_policies();
    
    is_standalone = standalone;
    if ( is_standalone ) {
        top_dialog->signal_delete_event().connect(sigc::mem_fun(*this, &VOMS_config::quit_standalone));
        edit_prefs();
    }
    
    err_dialog = new Gtk::MessageDialog(*top_dialog, "", false, 
                                        Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
    err_dialog->signal_response().connect(sigc::mem_fun(*this, &VOMS_config::on_msg_response)); 
    
}

VOMS_config::~VOMS_config(){
    delete err_dialog;
    delete browser_dialog;
}

Glib::ustring VOMS_config::get_user_cert_path() const {
    return g_client->get_string(VOMS_GCONF_LOCATION "/" VOMS_GCONF_USER_CERT);
}

Glib::ustring VOMS_config::get_user_key_path() const {
    return g_client->get_string(VOMS_GCONF_LOCATION "/" VOMS_GCONF_USER_KEY);
}

Glib::ustring VOMS_config::get_ca_dir() const {
    return g_client->get_string(VOMS_GCONF_LOCATION "/" VOMS_GCONF_CA_DIR);
}

Glib::ustring VOMS_config::get_vo_dir() const {
    return g_client->get_string(VOMS_GCONF_LOCATION "/" VOMS_GCONF_VO_DIR);
}

Glib::ustring VOMS_config::get_proxy_path() const {
    return g_client->get_string(VOMS_GCONF_LOCATION "/" VOMS_GCONF_PROXY);
}

int VOMS_config::get_ac_hours() const {
    return g_client->get_int(VOMS_GCONF_LOCATION "/" VOMS_GCONF_AC_HOURS);
}

int VOMS_config::get_ac_minutes() const {
    return g_client->get_int(VOMS_GCONF_LOCATION "/" VOMS_GCONF_AC_MINUTES);
}

int VOMS_config::get_timeout() const {
    return g_client->get_int(VOMS_GCONF_LOCATION "/" VOMS_GCONF_TIMEOUT);
}

int VOMS_config::get_key_bit_len() const {
    return g_client->get_int(VOMS_GCONF_LOCATION "/" VOMS_GCONF_BITS);
}

int VOMS_config::get_proxy_type() const {
    return g_client->get_int(VOMS_GCONF_LOCATION "/" VOMS_GCONF_PROXY_TYPE);
}

bool VOMS_config::configured() {
    return !get_user_cert_path().empty() &&
           !get_user_key_path().empty() &&
           !get_ca_dir().empty() &&
           !get_vo_dir().empty() &&
           !get_proxy_path().empty();
}

void VOMS_config::edit_prefs() {
    Gtk::Dialog* p_dialog = 0;
    Gtk::FileChooserButton* cert_entry = 0;
    Gtk::FileChooserButton* key_entry = 0;
    Gtk::FileChooserButton* cadir_entry = 0;
    Gtk::FileChooserButton* vodir_entry = 0;
    Gtk::Entry* proxy_entry = 0;
    Gtk::ComboBox* hours_combo = 0;
    Gtk::ComboBox* minutes_combo = 0;
    Gtk::ComboBox* proxy_type_combo = 0;
    Gtk::ComboBox* bit_len_combo = 0;
    Gtk::SpinButton* timeout_entry = 0;
    
    prefs_dialog->get_widget(XMLPREF_DIALOG_TAG, p_dialog);
    
    prefs_dialog->get_widget(XMLPREF_CERT_TAG, cert_entry);
    cert_entry->set_filename(get_user_cert_path());
    
    prefs_dialog->get_widget(XMLPREF_KEY_TAG, key_entry);
    key_entry->set_filename(get_user_key_path());
    
    prefs_dialog->get_widget(XMLPREF_CADIR_TAG, cadir_entry);
    cadir_entry->set_filename(get_ca_dir());
    
    prefs_dialog->get_widget(XMLPREF_VODIR_TAG, vodir_entry);
    vodir_entry->set_filename(get_vo_dir());
    
    prefs_dialog->get_widget(XMLPREF_PROXY_FILE_TAG, proxy_entry);
    proxy_entry->set_text(get_proxy_path());
    
    prefs_dialog->get_widget(XMLPREF_ACHOURS_TAG, hours_combo);
    hours_combo->set_active(get_ac_hours());
    
    prefs_dialog->get_widget(XMLPREF_ACMINUTES_TAG, minutes_combo);
    minutes_combo->set_active(get_ac_minutes() / MINUTES_SLOT);
    
    prefs_dialog->get_widget(XMLPREF_TIMEOUT_TAG, timeout_entry);
    timeout_entry->set_value(get_timeout());

    prefs_dialog->get_widget(XMLPREF_BITS_TAG, bit_len_combo);
    bit_len_combo->set_active(bitlen2idx(get_key_bit_len()));

    prefs_dialog->get_widget(XMLPREF_PROXY_TYPE_TAG, proxy_type_combo);
    proxy_type_combo->set_active(get_proxy_type()-2);
    
    edit_auth_vo_prefs();
    edit_contacts_prefs();
    edit_policies_prefs();

    p_dialog->show();
        
}

bool VOMS_config::edit_prefs_remote() {
    edit_prefs();
    return false;
}

bool VOMS_config::quit_standalone(GdkEventAny* dummy) {
    Gtk::Main::quit();
    return true;
}

void VOMS_config::moveto_tab(int direction) {
    Gtk::Notebook *notebook = 0;
    Gtk::Button *prev_btn = 0;
    Gtk::Button *fwd_btn = 0;
    Gtk::Button *apply_btn = 0;
    Gtk::Label *descr_label = 0;

    prefs_dialog->get_widget(XMLPREF_NOTEBOOK_LABEL, notebook);
    prefs_dialog->get_widget(XMLPREF_PREVBTN_LABEL, prev_btn);
    prefs_dialog->get_widget(XMLPREF_FWDBTN_LABEL, fwd_btn);
    prefs_dialog->get_widget(XMLPREF_APPLYBTN_LABEL, apply_btn);
    prefs_dialog->get_widget(XMLPREF_GENDESCR_LABEL, descr_label);
    
    if ( direction > 0 ) {
        if ( notebook->get_current_page() == 0 ) {
            prev_btn->set_sensitive(true);
        }
        notebook->next_page();
        if ( notebook->get_current_page() == (notebook->get_n_pages() - 1) ) {
            fwd_btn->set_sensitive(false);
            apply_btn->set_sensitive(true);
        }
    
    } else if ( direction < 0 ) {
        if ( notebook->get_current_page() == (notebook->get_n_pages() - 1) ) {
            fwd_btn->set_sensitive(true);
            apply_btn->set_sensitive(false);
        }
        notebook->prev_page();
        if ( notebook->get_current_page() == 0 ) {
            prev_btn->set_sensitive(false);
        }
    }
  
    std::stringstream buff;
    buff << "<span size=\"larger\">";
    if ( notebook->get_current_page() == 0 ) {
        buff << _("User Credentials");
    } else if ( notebook->get_current_page() == 1 ){
        buff << _("Basic configuration parameters");
    } else if ( notebook->get_current_page() == 2 ){
        buff << _("VOMS server list and roles");
    }else{
        buff << _("Additional policy definitions");
    }
    buff << "</span>";
    descr_label->set_label(buff.str());
    
}

void VOMS_config::next_tab() {
    moveto_tab(1);
}
void VOMS_config::prev_tab(){
    moveto_tab(-1);
}


void VOMS_config::apply_changes() {
    Gtk::Dialog* p_dialog = 0;
    Gtk::FileChooserButton* cert_entry = 0;
    Gtk::FileChooserButton* key_entry = 0;
    Gtk::FileChooserButton* cadir_entry = 0;
    Gtk::FileChooserButton* vodir_entry = 0;
    Gtk::Entry* proxy_entry = 0;
    Gtk::ComboBox* hours_combo = 0;
    Gtk::ComboBox* minutes_combo = 0;
    Gtk::ComboBox* proxy_type_combo = 0;
    Gtk::ComboBox* bit_len_combo = 0;
    Gtk::SpinButton* timeout_entry = 0;
    
    prefs_dialog->get_widget(XMLPREF_DIALOG_TAG, p_dialog);
    
    prefs_dialog->get_widget(XMLPREF_CERT_TAG, cert_entry);
    Glib::ustring candidate_cert = cert_entry->get_filename();
    /*
     * TODO check the certificate
     */
    
    prefs_dialog->get_widget(XMLPREF_KEY_TAG, key_entry);
    Glib::ustring candidate_key = key_entry->get_filename();
    /*
     * TODO check the key
     */
    
    prefs_dialog->get_widget(XMLPREF_CADIR_TAG, cadir_entry);
    Glib::ustring candidate_cadir = cadir_entry->get_filename();
    /*
     * TODO check the CA directory
     */
    
    prefs_dialog->get_widget(XMLPREF_VODIR_TAG, vodir_entry);
    Glib::ustring candidate_vodir = vodir_entry->get_filename();
    /*
     * TODO check the VO directory
     */
    
    prefs_dialog->get_widget(XMLPREF_PROXY_FILE_TAG, proxy_entry);
    Glib::ustring candidate_proxy = proxy_entry->get_text();
    /*
     * TODO check the output file
     */
    
    prefs_dialog->get_widget(XMLPREF_ACHOURS_TAG, hours_combo);
    int candidate_hours = hours_combo->get_active_row_number();
    
    prefs_dialog->get_widget(XMLPREF_ACMINUTES_TAG, minutes_combo);
    int candidate_minutes = minutes_combo->get_active_row_number() * MINUTES_SLOT;
    if ( (candidate_hours + candidate_minutes) == 0 ) {
        err_dialog->set_message(_("Wrong validity time"));
        err_dialog->show();
        return;
    }

    prefs_dialog->get_widget(XMLPREF_BITS_TAG, bit_len_combo);
    int candidate_bitlen = idx2bitlen(bit_len_combo->get_active_row_number());

    prefs_dialog->get_widget(XMLPREF_PROXY_TYPE_TAG, proxy_type_combo);
    int candidate_type = proxy_type_combo->get_active_row_number()+2;

    prefs_dialog->get_widget(XMLPREF_TIMEOUT_TAG, timeout_entry);
    int candidate_to = timeout_entry->get_value_as_int();
    
    try {
        
        g_client->set(VOMS_GCONF_LOCATION "/" VOMS_GCONF_USER_CERT, candidate_cert);
        g_client->set(VOMS_GCONF_LOCATION "/" VOMS_GCONF_USER_KEY, candidate_key);
        g_client->set(VOMS_GCONF_LOCATION "/" VOMS_GCONF_CA_DIR, candidate_cadir);
        g_client->set(VOMS_GCONF_LOCATION "/" VOMS_GCONF_VO_DIR, candidate_vodir);
        g_client->set(VOMS_GCONF_LOCATION "/" VOMS_GCONF_PROXY, candidate_proxy);
        g_client->set(VOMS_GCONF_LOCATION "/" VOMS_GCONF_AC_HOURS, candidate_hours);
        g_client->set(VOMS_GCONF_LOCATION "/" VOMS_GCONF_AC_MINUTES, candidate_minutes);
        g_client->set(VOMS_GCONF_LOCATION "/" VOMS_GCONF_BITS, candidate_bitlen);
        g_client->set(VOMS_GCONF_LOCATION "/" VOMS_GCONF_PROXY_TYPE, candidate_type);
        g_client->set(VOMS_GCONF_LOCATION "/" VOMS_GCONF_TIMEOUT, candidate_to);
        
        apply_auth_vo_prefs();
        
        apply_contacts_prefs();
        
        apply_policies_prefs();
        
    } catch ( Gnome::Conf::Error error ) {
        /*
         * TODO improve message per error code
         */
        std::stringstream tmpbuff;
        tmpbuff << _("Cannot configure applet") << " (" << error.code() << ")";
        err_dialog->set_message(tmpbuff.str().c_str());
        err_dialog->show();
    }
    
    if ( is_standalone ) {
        Gtk::Main::quit();
    }
    
    p_dialog->hide();
}

void VOMS_config::cancel_changes() {
    Gtk::Dialog* p_dialog = 0;
    prefs_dialog->get_widget(XMLPREF_DIALOG_TAG, p_dialog);
    
    if ( wizard_mode ) {
        /*
         * TODO roll-back to empty configuration
         */
    }
    
    if ( is_standalone ) {
        Gtk::Main::quit();
    }
    p_dialog->hide();
}

void VOMS_config::on_msg_response(int response_id) {
    err_dialog->hide();
}

Glib::SignalProxy1< void,int > VOMS_config::signal_response() {
    /*
     * TODO verify memleak
     */
    Gtk::Dialog *top_dialog = 0;
    prefs_dialog->get_widget(XMLPREF_DIALOG_TAG, top_dialog);
    return top_dialog->signal_response();
}


