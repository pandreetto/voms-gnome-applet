/*
 * LICENSE TO BE DEFINED
 */

#include <iostream>
#include <iomanip>
#include <cstring>
#include <vector>
#include <glibmm/i18n.h>

#include <unistd.h>

#include "gtkmm/main.h"
#include <glibmm/optioncontext.h>
#include <glibmm/optiongroup.h>
#include <glibmm/optionentry.h>

#include "gtkmm/imagemenuitem.h"
#include "gtkmm/stock.h"

#include "voms-applet.h"
#include "voms-icons-factory.h"

#ifdef USE_NOTIFY
#include "libnotifymm/init.h"
#include "libnotifymm/notification.h"
#endif

VOMS_applet::VOMS_applet(){

    Gtk::ImageMenuItem *m_item;

    config = new VOMS_config();
    //is_configured = config->configured();
    
    engine = new VOMS_client_engine(*config);

    vomses_menu = new VOMS_server_list(*config, *engine);
    engine->add_listener(*vomses_menu);
    engine->add_listener(*this);
    
    voms_about = new Gtk::AboutDialog();
    std::vector<Glib::ustring> author_list = std::vector<Glib::ustring>();
    author_list.push_back("Paolo Andreetto");
    voms_about->set_authors(author_list);
    voms_about->set_logo(Gdk::Pixbuf::create_from_file(VOMS_LOGO_ICON));
    voms_about->signal_response().connect(sigc::mem_fun(*this, &VOMS_applet::about_response));
    
    context_menu = new Gtk::Menu::Menu();
    m_item = new Gtk::ImageMenuItem(Gtk::Stock::PREFERENCES);
    m_item->signal_activate().connect(sigc::mem_fun(config, &VOMS_config::edit_prefs));
    m_item->show();
    context_menu->attach(*m_item,0,1,0,1);
    m_item = new Gtk::ImageMenuItem(Gtk::Stock::ABOUT);
    m_item->signal_activate().connect(sigc::mem_fun(voms_about, &Gtk::AboutDialog::show));
    m_item->show();
    context_menu->attach(*m_item,0,1,1,2);
    m_item = new Gtk::ImageMenuItem(Gtk::Stock::QUIT);
    m_item->signal_activate().connect(sigc::ptr_fun(&Gtk::Main::quit));
    m_item->show();
    context_menu->attach(*m_item,0,1,2,3);
    context_menu->show();
    
    voms_icon = Gtk::StatusIcon::create_from_file(VOMS_icon_factory::disabled_icon());

    voms_icon->signal_activate().connect(sigc::mem_fun(*this, &VOMS_applet::show_vomses_menu));
    voms_icon->signal_popup_menu().connect(sigc::mem_fun(*this, &VOMS_applet::show_ctx_menu));
    
    icon_file = VOMS_icon_file();
    /*
    voms_icon->signal_query_tooltip().connect(sigc::mem_fun(*this, &VOMS_applet::query_tooltip));
    */
    
    config->signal_response().connect(sigc::mem_fun(*this, &VOMS_applet::prefs_response));

    err_notified = CHK_PRX_NOCHANGES;
}

bool VOMS_applet::init() {

#ifdef USE_DBUSMAN
    if ( init_dbus_manager() ) {
        std::cerr << "Instance of voms applet is already running" << std::endl;
        return false;
    }
#endif

    icon_timer_conn = Glib::signal_timeout().connect(
                              sigc::mem_fun(*this, &VOMS_applet::update_proxy_view), 60000);

    update_proxy_view();

#ifdef USE_NOTIFY
    if ( ! Notify::is_initted() ) {
        Notify::init("voms-gnome-applet");
    }
#endif

    return true;
}

void VOMS_applet::show_ctx_menu(guint button, guint32 timestamp){
    voms_icon->popup_menu_at_position(*context_menu, button, timestamp);
}

void VOMS_applet::show_vomses_menu(){
    if( !engine->processing() && config->configured() ){
        /*
         * TODO define timestamp
         */
        vomses_menu->reload();
        voms_icon->popup_menu_at_position(*vomses_menu, 1, 0);
    }
}

void VOMS_applet::notify_ren_attempt(Glib::ustring voname, Glib::ustring host, int port) {
#ifdef USE_NOTIFY

    std::stringstream buff;
    buff << _("Trying to contact") << ": " << host << ":" << port;

    Notify::Notification try_srv_notify(_("Contacting voms server"), buff.str(),
                                        VOMS_icon_factory::active_icon(),
                                        voms_icon);
    try_srv_notify.set_timeout(3000);

    if ( !try_srv_notify.show() ) {
        std::cout << "Error displaying notify" << std::endl;
    }
    
#endif
}

void VOMS_applet::notify_renewed(Glib::ustring voname, Glib::ustring host, int port) {

    update_proxy_view();

    voms_icon->set_from_file(VOMS_icon_factory::first_icon());
}

void VOMS_applet::notify_gen_error(Glib::ustring err_msg) {
#ifdef USE_NOTIFY
        
    Notify::Notification try_srv_notify(_("Failure"), err_msg,
                                        VOMS_icon_factory::active_icon(),
                                        voms_icon);
    try_srv_notify.set_timeout(3000);

    if ( !try_srv_notify.show() ) {
        std::cout << "Error displaying notify" << std::endl;
    }
    
#endif
}

bool VOMS_applet::query_tooltip(int x, int y, bool keyboard_mode, const Glib::RefPtr<Gtk::Tooltip>& tooltip) {
    
    return true;
    
}

bool VOMS_applet::update_proxy_view() {
    using namespace std;

/*
 * TODO check and notify if a proxy has been created using the command line
 *      tool with a VO that is not present in the list
 */
    if ( !config->configured() ) {
        std::cout << "Applet not configured" << std::endl;
        return true;
    }

    switch ( engine->check_proxy() ) {
    case CHK_PRX_ERROR:

        if ( err_notified != CHK_PRX_ERROR ) {
            notify_gen_error(_("Error checking proxy"));
            err_notified = CHK_PRX_ERROR;
        }
        return true;
        
    case CHK_PRX_EXPIRED:
        if ( err_notified != CHK_PRX_EXPIRED) {
            notify_gen_error(_("Proxy is expired"));
            voms_icon->set_from_file(VOMS_icon_factory::icon(PROXY_STATUS_EXPIRED));
            err_notified = CHK_PRX_ERROR;
        }
        return true;
    case CHK_PRX_NOENT:
        err_notified = CHK_PRX_NOENT;
        break;
        
    case CHK_PRX_RELOADED:
        if ( ! vomses_menu->contains(engine->get_VO()) ) {
        
            if ( err_notified != CHK_PRX_RELOADED ) {
                notify_gen_error(_("Proxy available but VO unknown"));
                err_notified = CHK_PRX_RELOADED;
            }
        }
        vomses_menu->reload(true);
        break;
    default:
        err_notified = CHK_PRX_NOCHANGES;
    }    
    
    Glib::ustring vo_name = engine->get_VO();
    time_t timeleft = engine->get_timeleft();
    proxy_status_t current_status = engine->get_proxy_status();    

    stringstream buff;
    if ( vo_name.empty() ) {
    
        buff << _("No credentials");
        
    } else if ( timeleft == 0 ) {
    
        buff << vo_name << ": " << _("credentials expired");
        
    } else {
    
        buff << vo_name << ": " << _("lifetime") << " ";
        buff << setfill('0') << setw(2) << (timeleft / 3600);
        buff << ":" << setw(2) << ((timeleft % 3600) / 60);
        
    }
    voms_icon->set_tooltip(buff.str());
    
    if ( icon_file.update(current_status) ) {
        voms_icon->set_from_file(icon_file.path());
    }
    
    vomses_menu->notify_status(current_status);
    
    return true;
}

void VOMS_applet::about_response(int res_id) {
    
    if ( res_id == Gtk::RESPONSE_CLOSE || res_id == Gtk::RESPONSE_CANCEL ) {
        voms_about->hide();
    } else {
        std::cout << "About response: " << res_id << std::endl;
    }
}

void VOMS_applet::prefs_response(int res_id) {
    /*
     * TODO load contact list
     */
}

VOMS_applet::~VOMS_applet(){
#ifdef USE_DBUSMAN
    destroy_dbus_manager();
#endif
    delete context_menu;
    delete voms_about;
    delete vomses_menu;
    delete engine;
    delete config;
}

int main(int argc, char *argv[]) {
    
    Glib::OptionContext opt_ctx;
    
    Glib::OptionGroup main_group("","","");
    
    bool config_only = false;
    Glib::OptionEntry cfg_entry;
    cfg_entry.set_long_name("config-only");
    cfg_entry.set_short_name('c');
    cfg_entry.set_description(_("run only the properties applet"));
    main_group.add_entry(cfg_entry, config_only);
    
    opt_ctx.set_main_group(main_group);
    
    Gtk::Main kit(argc, argv, opt_ctx);

    if ( !config_only ) {
    
        VOMS_applet applet = VOMS_applet();

        if ( applet.init() ) {
        
            Gtk::Main::run();

        }
        
    } else if ( !call_remote_config() ) {
    
        std::cout << "Running stand alone dialog" << std::endl;
        
        VOMS_config config_win(true);
        Gtk::Main::run();
        
    }
    
    return 0;
}

