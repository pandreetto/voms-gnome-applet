/*
 * LICENSE TO BE DEFINED
 */

#ifndef VOMS_ICONS_FACTORY_H
#define VOMS_ICONS_FACTORY_H

#include "config.h"
#include "voms-global-defs.h"
#include <glibmm/ustring.h>

#define VOMS_MAIN_ICON ICON22_DIR "/voms-ac-100-22x22.png"
#define VOMS_INIT_ICON ICON22_DIR "/voms-ac-gray-22x22.png"
#define VOMS_ACTIVE_ICON ICON22_DIR "/voms-ac-100-22x22.png"
#define VOMS_DISABLED_ICON ICON22_DIR "/voms-ac-gray-22x22.png"

#define VOMS_LOGO_ICON ICON48_DIR "/voms-ac-100-48x48.png"

class VOMS_icon_factory {

    public:
    
    static Glib::ustring& icon(proxy_status_t);
    static Glib::ustring& first_icon();
    static Glib::ustring& init_icon();
    static Glib::ustring& disabled_icon();
    static Glib::ustring& active_icon();
    
    private:

    static Glib::ustring icons_array[6];
    
};

class VOMS_icon_file {

    public:
    
    VOMS_icon_file();
    bool update(proxy_status_t);
    Glib::ustring& path();
    
    private:
    
    proxy_status_t current_status;
};


#endif
