/*
 * LICENSE TO BE DEFINED
 */

#ifndef VOMS_APPLET_CONFIG_H
#define VOMS_APPLET_CONFIG_H

#include "config.h"

#include "glibmm/refptr.h"
#include "gconfmm.h"
#include <libglademm.h>

#include "gtkmm/button.h"
#include "gtkmm/entry.h"
#include "gtkmm/treemodel.h"
#include "gtkmm/treemodelcolumn.h"
#include "gtkmm/liststore.h"
#include "gtkmm/filechooserdialog.h"
#include "gtkmm/messagedialog.h"
#include "gtkmm/main.h"

#include "voms/voms_api.h"

#define VOMS_GCONF_LOCATION "/apps/voms-gnome-applet"

#define VOMS_GCONF_USER_CERT "usercert"
#define VOMS_GCONF_USER_KEY "userkey"
#define VOMS_GCONF_CA_DIR "cadir"
#define VOMS_GCONF_VO_DIR "vodir"
#define VOMS_GCONF_PROXY "proxy"
#define VOMS_GCONF_AC_HOURS "achours"
#define VOMS_GCONF_AC_MINUTES "acminutes"
#define VOMS_GCONF_TIMEOUT "timeout"
#define VOMS_GCONF_BITS "bits"
#define VOMS_GCONF_PROXY_TYPE "proxy_type"

#define VOMS_GCONF_CONTACTS "contacts"

#define VOMS_GCONF_POLICIES "policies"

#define XMLPREF_PREFS_FILE "preferences.glade"
#define XMLPREF_CONTACT_FILE "contact.glade"
#define XMLPREF_POLICY_FILE "policy.glade"
#define XMLPREF_AUTHVO_FILE "authvochooser.glade"

#define XMLPREF_DIALOG_TAG "preferences_dialog"
#define XMLPREF_CERT_TAG "usercert_entry"
#define XMLPREF_KEY_TAG "userkey_entry"
#define XMLPREF_CADIR_TAG "cadir_entry"
#define XMLPREF_VODIR_TAG "vomsdir_entry"
#define XMLPREF_PROXY_FILE_TAG "proxy_file_entry"
#define XMLPREF_PROXY_BROW_TAG "proxy_browser_btn"
#define XMLPREF_ACHOURS_TAG "hours_combo"
#define XMLPREF_ACMINUTES_TAG "minutes_combo"
#define XMLPREF_TIMEOUT_TAG "timeout_entry"
#define XMLPREF_BITS_TAG "bits_combo"
#define XMLPREF_PROXY_TYPE_TAG "proxy_type_combo"

#define XMLPREF_CONTACTS_TAG "contact_list"
#define XMLPREF_CNTC_DIAL_TAG "contact_dialog"
#define XMLPREF_CNTC_ADD_TAG "add_srv_btn"
#define XMLPREF_CNTC_EDIT_TAG "edit_srv_btn"
#define XMLPREF_CNTC_DEL_TAG "del_srv_btn"
#define XMLPREF_CNTC_OK_TAG "ok_btn"
#define XMLPREF_CNTC_CANC_TAG "canc_btn"
#define XMLPREF_CNTC_ROLE_TAG "roleEntry"
#define XMLPREF_CNTC_VODN_TAG "serviceCombo"
#define XMLPREF_CNTC_HOST_TAG "hostEntry"
#define XMLPREF_CNTC_PORT_TAG "port_btn"

#define XMLPREF_POLICIES_TAG "policy_list"
#define XMLPREF_POLI_DIAL_TAG "policy_dialog"
#define XMLPREF_POLI_OK_TAG "ok_btn"
#define XMLPREF_POLI_CANC_TAG "canc_btn"
#define XMLPREF_POLI_ADD_TAG "add_pol_btn"
#define XMLPREF_POLI_EDIT_TAG "edit_pol_btn"
#define XMLPREF_POLI_DEL_TAG "del_pol_btn"
#define XMLPREF_POLI_NAME_TAG "name_entry"
#define XMLPREF_POLI_LANG_TAG "lang_entry"
#define XMLPREF_POLI_DATA_TAG "policy_text"

#define XMLPREF_AUTHVO_TAG "vo_list"
#define XMLPREF_AUTHVO_LSC_TAG "import_lsc_btn"
#define XMLPREF_AUTHVO_PEM_TAG "import_pem_btn"
#define XMLPREF_AUTHVO_DEL_TAG "del_vo_btn"
#define XMLPREF_AUTHVO_DIAL_TAG "auth_vo_chooser"
#define XMLPREF_AUTHVO_CANC_TAG "authvo_canc_btn"
#define XMLPREF_AUTHVO_OPEN_TAG "authvo_open_btn"
#define XMLPREF_AUTHVO_ENTRY_TAG "authvo_entry"


#define XMLPREF_APPLYBTN_LABEL "apply_button"
#define XMLPREF_CANCBTN_LABEL "close_button"
#define XMLPREF_FWDBTN_LABEL "forward_button"
#define XMLPREF_PREVBTN_LABEL "prev_button"
#define XMLPREF_NOTEBOOK_LABEL "prefs-notebook"
#define XMLPREF_GENDESCR_LABEL "descr_label"

#define MINUTES_SLOT 5
#define GLIST_SEP '|'
#define INHERITALL_LABEL "Inherit-all"
#define INHERITALL_OID "1.3.6.1.5.5.7.21.1"
#define INDEPENDENT_LABEL "Independent"
#define INDEPENDENT_OID "1.3.6.1.5.5.7.21.2"

class contacts_modelcol : public Gtk::TreeModel::ColumnRecord {
    public:
    
    contacts_modelcol() {
        add(attr);
        add(host);
        add(port);
        add(dn);
        add(version);
    }
    
    Gtk::TreeModelColumn<Glib::ustring> attr;
    Gtk::TreeModelColumn<Glib::ustring> host;
    Gtk::TreeModelColumn<int> port;
    Gtk::TreeModelColumn<Glib::ustring> dn;
    Gtk::TreeModelColumn<int> version;
};

class vodn_modelcol : public Gtk::TreeModel::ColumnRecord {
    public:
    
    vodn_modelcol() {
        add(vo);
        add(dn);
    }
    
    Gtk::TreeModelColumn<Glib::ustring> vo;
    Gtk::TreeModelColumn<Glib::ustring> dn;
};

class policies_modelcol : public Gtk::TreeModel::ColumnRecord {
    public:
    
    policies_modelcol() {
        add(name);
        add(language);
        add(policy);
    }
    
    Gtk::TreeModelColumn<Glib::ustring> name;
    Gtk::TreeModelColumn<Glib::ustring> language;
    Gtk::TreeModelColumn<Glib::ustring> policy;
};

class policy_data {
    public:
    
    Glib::ustring oid;
    Glib::ustring content;
    
};

class authvo_modelcol : public Gtk::TreeModel::ColumnRecord {
    public:
    
    authvo_modelcol() {
        add(vo_name);
        add(vo_dn);
        add(issuer_dn);
        add(lsc_file);
    }

    Gtk::TreeModelColumn<Glib::ustring> vo_name;
    Gtk::TreeModelColumn<Glib::ustring> vo_dn;
    Gtk::TreeModelColumn<Glib::ustring> issuer_dn;
    Gtk::TreeModelColumn<Glib::ustring> lsc_file;
};

class VOMSBrowserDialog : public Gtk::FileChooserDialog {
    public:
    
    VOMSBrowserDialog(Gtk::Window& parent, Gtk::Entry& b_entry);
    virtual ~VOMSBrowserDialog();
    
    protected:
    
    void on_response(int response_id);
    
    private:
    
    Gtk::Button* saveas_btn;
    Gtk::Button* cancel_btn;
    Gtk::Entry* bound_entry;
};

class VOMS_config {

    public:
    
    VOMS_config(bool standalone=false);
    virtual ~VOMS_config();
    Glib::ustring get_user_cert_path() const;
    Glib::ustring get_user_key_path() const;
    Glib::ustring get_ca_dir() const;
    Glib::ustring get_vo_dir() const;
    Glib::ustring get_proxy_path() const;
    int get_ac_hours() const;
    int get_ac_minutes() const;
    int get_timeout() const;
    int get_key_bit_len() const;
    int get_proxy_type() const;
    void get_contacts(std::vector<contactdata>&) const;
    void get_policies(std::map<Glib::ustring, policy_data>&) const;
    void on_msg_response(int);
    
    bool configured();
    void edit_prefs();
    bool edit_prefs_remote();
    void apply_changes();
    void cancel_changes();
    bool quit_standalone(GdkEventAny*);
    void next_tab();
    void prev_tab();
    void moveto_tab(int);
    Glib::SignalProxy1< void,int > signal_response();
    
    /*
     * Contacts management (see voms-applet-contacts.cpp)
     */
    void popup_new_contact();
    void popup_selected_contact();
    void del_selected_contact();
    void insert_contact();
    bool contacts_changes_since(double) const;

    /*
     * Policies management (see voms-applet-policies.cpp)
     */
    void popup_new_policy();
    void popup_selected_policy();
    void del_selected_policy();
    void insert_policy();
    
    /*
     * VODir management (see voms-applet-vodir.cpp)
     */
    void import_lsc_file();
    void del_selected_auth_vo();
    
    private:
    
    /*
     * Contacts management (see voms-applet-contacts.cpp)
     */
    void init_contacts();
    void edit_contacts_prefs();
    void apply_contacts_prefs();
    void fillin_contact_popup();
    int fillin_vodn_combo();
    
    bool list_contacts_modify;
    double contacts_modify_ts;
    contacts_modelcol contact_columns;
    vodn_modelcol vodn_columns;
    Glib::RefPtr<Gtk::ListStore> contact_store;
    Glib::RefPtr<Gtk::ListStore> vodn_store;
    Glib::RefPtr<Gnome::Glade::Xml> contact_dialog;
    bool cntc_add_mode;
    
    /*
     * Policies management (see voms-applet-policies.cpp)
     */
    void init_policies();
    void edit_policies_prefs();
    void apply_policies_prefs();
    void fillin_policy_popup();

    bool list_policies_modify;
    policies_modelcol policy_columns;
    Glib::RefPtr<Gtk::ListStore> policy_store;
    Glib::RefPtr<Gnome::Glade::Xml> policy_dialog;
    bool poli_add_mode;
    
    /*
     * VODir management (see voms-applet-vodir.cpp)
     */
    void init_vodir();
    void edit_auth_vo_prefs();
    void apply_auth_vo_prefs();
    
    Glib::RefPtr<Gtk::ListStore> auth_vo_store;
    Glib::RefPtr<Gnome::Glade::Xml> auth_vo_dialog;
    authvo_modelcol auth_vo_columns;
    Glib::ustring internal_vodir; 
    
    /*
     * Class wide definitions
     */    
    Glib::RefPtr<Gnome::Conf::Client> g_client;
    Glib::RefPtr<Gnome::Glade::Xml> prefs_dialog;
    Gtk::MessageDialog *err_dialog;
    bool is_standalone;
    bool wizard_mode;

    VOMSBrowserDialog* browser_dialog;    
};


#endif
