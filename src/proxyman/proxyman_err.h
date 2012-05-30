/*
 * LICENSE TO BE DEFINED
 */


#ifndef PROXYMAN_ERR_H
#define PROXYMAN_ERR_H

#include <exception>

#define VOMSERR_BAD_CRED_FILES 1
#define VOMSERR_KEYS_NOACCESS 2
#define VOMSERR_BAD_PWD 3
#define VOMSERR_WRONG_CADIR 4
#define VOMSERR_WRONG_CERTFILE 5
#define VOMSERR_WRONG_KEYFILE 6
#define VOMSERR_WRONG_OUTFILE 7
#define VOMSERR_EXP_CERT 8
#define VOMSERR_VER_FAILED 9
#define VOMSERR_INTERNAL 10
#define VOMSERR_CERT_BEFORE_PRX 11
#define VOMSERR_CANT_CREATE 12
#define VOMSERR_POLICY_UNDEF 13
#define VOMSERR_EXP_PROXY 14

#define VOMSERR_NOT_BLOCKING 100
#define VOMSERR_CANNOT_RENEW 101
#define VOMSERR_SERVER_ERROR 102

class VOMSError : public std::exception{
    public:
    VOMSError(const int err_code);
    virtual ~VOMSError() throw();
    int get_error_code();
    virtual const char* what() const throw ();
    bool is_blocking();
    
    private:
    int code;
};

#endif
