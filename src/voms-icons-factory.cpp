/*
 * LICENSE TO BE DEFINED
 */

#include "voms-icons-factory.h"

Glib::ustring VOMS_icon_factory::icons_array[] = { ICON22_DIR "/voms-ac-gray-22x22.png",
                                                   ICON22_DIR "/voms-ac-100-22x22.png",
                                                   ICON22_DIR "/voms-ac-75-22x22.png",
                                                   ICON22_DIR "/voms-ac-50-22x22.png",
                                                   ICON22_DIR "/voms-ac-25-22x22.png",
                                                   ICON22_DIR "/voms-ac-expired-22x22.png" } ;


Glib::ustring& VOMS_icon_factory::icon(proxy_status_t status) {
    if ( status == PROXY_STATUS_DISABLED ) return icons_array[0];
    if ( status == PROXY_STATUS_EXPIRED ) return icons_array[5];
    if ( status >= 75 ) return icons_array[4];
    if ( status >= 50 ) return icons_array[3];
    if ( status >= 25 ) return icons_array[2];
    return icons_array[1];
}

Glib::ustring& VOMS_icon_factory::first_icon() {
    return icons_array[1];
}

Glib::ustring& VOMS_icon_factory::init_icon() {
    return icons_array[0];
}

Glib::ustring& VOMS_icon_factory::disabled_icon() {
    return icons_array[0];
}

Glib::ustring& VOMS_icon_factory::active_icon() {
    return icons_array[1];
}





VOMS_icon_file::VOMS_icon_file() {
    current_status = PROXY_STATUS_DISABLED;
}


bool VOMS_icon_file::update(proxy_status_t status) {
    if ( status == current_status ) return false;
    bool result = true;
    if ( ( current_status >= 0 ) && ( current_status <= 100 ) &&
         ( status >= 0 ) && ( status <= 100 ) ) {
        result = ( current_status / 25) != ( status / 25 );
    }
    current_status = status;
    return result;
}
    
Glib::ustring& VOMS_icon_file::path() {
    return VOMS_icon_factory::icon(current_status);
}

    

