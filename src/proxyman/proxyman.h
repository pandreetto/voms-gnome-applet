/*********************************************************************
 *
 * Authors: Vincenzo Ciaschini - Vincenzo.Ciaschini@cnaf.infn.it 
 *          Valerio Venturi - Valerio.Venturi@cnaf.infn.it 
 *          Paolo Andreetto - paolo.andreetto@pd.infn.it 
 *
 * Copyright (c) Members of the EGEE Collaboration. 2004-2010.
 * See http://www.eu-egee.org/partners/ for details on the copyright holders.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Parts of this code may be based upon or even include verbatim pieces,
 * originally written by other people, in which case the original header
 * follows.
 *
 *********************************************************************/
#ifndef PROXYMAN_H
#define PROXYMAN_H

#include <string>
#include <vector>

extern "C" {
#include <sys/time.h>
#include "openssl/bn.h"
  
#include "sslutils.h"
#include "voms/newformat.h"
  
}

#include "voms/voms_api.h"
#include "proxyman_err.h"

class proxy_manager {

    private:

    // PKI files
    char *             cacertfile;
    char *             certdir;
    char *             certfile;
    char *             keyfile;
    std::string        proxyfile;
  
    // proxy life time
    time_t             starttime;
    time_t             endtime;
  
    // proxy and AC settings */
    int                bits;
    int                hours;
    int                minutes;
    int                ac_hours;
    int                ac_minutes;
    bool               limit_proxy;
    int                proxyver;
    std::string        policyfile;
    std::string        policylang;
    int                pathlength;

    // globus version
    int                globus_version;

    std::string              ordering;
    std::string              targetlist;

    // vo
    std::string voID;
    STACK_OF(X509)           *cert_chain;
    X509                     *ucert;
    EVP_PKEY                 *private_key;
    int                       timeout;
    
    public:
  
    proxy_manager(std::string certfilestr,
                  std::string keyfilestr,
                  std::string passphrase,
                  std::string certdirstr,
                  std::string outfilestr,
                  int cred_hours,
                  int cred_minutes,
                  int to,
                  int bitlen,
                  int proxy_type,
                  std::string p_lang,
                  std::string p_data,
                  bool nokey = false);
    ~proxy_manager();
    void build_proxy(contactdata vomssrv_data) throw(VOMSError);
    time_t get_proxy_starttime();
    time_t get_proxy_endtime();
    std::string get_vo_id();
    void verify_proxy() throw(VOMSError);

    private:
  
    bool CreateProxy(std::vector<AC*> & acvector, int version);
    //X509_EXTENSION * CreateProxyExtension(std::string name, std::string data, bool crit = false);
    time_t get_cert_lifetime();
  
    // get openssl error */
    void Error();
    void get_ssl_error(std::vector<int>&);

};










class X509_cert_ptr {
    public:
    X509_cert_ptr();
    ~X509_cert_ptr();
    X509* cert();
    int load(const char *certname);
    
    private:
    X509 *cert_ptr;
};

class X509_stack_ptr {
    public:
    X509_stack_ptr();
    ~X509_stack_ptr();
    STACK_OF(X509)* stack();
    int load(const char *certname);
    
    private:
    STACK_OF(X509)* stack_ptr;
};

class X509_ext_list_ptr {
    public:
    X509_ext_list_ptr();
    ~X509_ext_list_ptr();
    STACK_OF(X509_EXTENSION)* list();
    bool add(X509V3_CTX*, int, const char*, bool);
    
    private:
    STACK_OF(X509_EXTENSION)* ext_list_ptr;
};













#endif
