/*
 * LICENSE TO BE DEFINED
 */

#include "proxyman_err.h"

#include <libintl.h>
#define  _(String) gettext (String)

VOMSError::VOMSError(const int err_code) {
    code = err_code;
}

VOMSError::~VOMSError() throw() {}
    
int VOMSError::get_error_code() {
    return code;
}

const char* VOMSError::what() const throw () {

    switch (code) {
    case VOMSERR_BAD_CRED_FILES: return _("Error reading credential files");
    case VOMSERR_KEYS_NOACCESS: return _("Wrong permissions for credential files");
    case VOMSERR_BAD_PWD: return _("Wrong password");
    case VOMSERR_CANNOT_RENEW: return _("Cannot renew credentials");
    case VOMSERR_WRONG_CADIR: return _("Wrong CA directory");
    case VOMSERR_WRONG_CERTFILE: return _("Wrong certificate path");
    case VOMSERR_WRONG_KEYFILE: return _("Wrong key path");
    case VOMSERR_WRONG_OUTFILE: return _("Wrong proxy path");
    case VOMSERR_EXP_CERT: return _("Certificate expired");
    case VOMSERR_VER_FAILED: return _("Verification failed");
    case VOMSERR_INTERNAL: return _("Internal client error");
    case VOMSERR_CERT_BEFORE_PRX: return _("User certificate expiration date within proxy lifetime");
    case VOMSERR_CANT_CREATE: return _("Cannot create proxy");
    case VOMSERR_SERVER_ERROR: return _("Cannot download VOMS credentials");
    case VOMSERR_POLICY_UNDEF: return _("Cannot find policy");
    case VOMSERR_EXP_PROXY: return _("Proxy is expired");
    }
        
    return _("Generic error");

}

bool VOMSError::is_blocking() {
    return ((code / VOMSERR_NOT_BLOCKING) != 1);
}
