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

#include "config.h"
#include "proxyman.h"

extern "C" {
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>

#include "credentials.h"

#include "replace.h"
}
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <string>

extern "C" 
{
#include "myproxycertinfo.h"
}

/*
 * Imported from fqan.c
 */
std::string FQANParse(std::string fqan) {
    std::stringstream parsed;
    
    if(fqan == "all" || fqan == "ALL") {
    
        parsed << "A";
        
    } else {

        std::string::size_type cap_pos = fqan.find("/Capability=");
        std::string::size_type role_pos = fqan.find("/Role=");

        if (role_pos != std::string::npos && role_pos > 0) {
        
            parsed << "B" << fqan.substr(0, role_pos) << ":" << fqan.substr(role_pos+6, cap_pos);
            
        } else if (role_pos==0) {
        
            parsed << "R" << fqan.substr(role_pos+6, cap_pos);
            
        } else if (fqan[0] == '/') {
        
            parsed << "G" << fqan.substr(0, cap_pos);
            
        }
    }

    return parsed.str();
}

std::string build_cmd_from_fqan(const std::vector<std::string>& fqans) {
    std::stringstream parsed;
  
    for(std::vector<std::string>::const_iterator i = fqans.begin(); i != fqans.end(); ++i) {
        std::string tmps = FQANParse(*i);
        if( tmps.empty() ) continue;
        if ( tmps == "A" ) return tmps;
        
        if ( i != (fqans.end() - 1) ){
            parsed << parsed << "," << tmps;
        } else {
            parsed << tmps;
        }
    }
    
    return parsed.str();
}





/* global variable for output control */

bool debug = true;

extern "C" {

/*
 * TODO improve passwd management
 */
static char* curr_pwd = NULL;

static int pwd_callback(char * buf, int num, int w) {
    memset(buf, 0, num);
    strncpy(buf, curr_pwd, num-1);
    return strlen(buf);
}
  
static void kpcallback(int p, int n) 
{
  char c='B';
    
  if (p == 0) c='.';
  if (p == 1) c='+';
  if (p == 2) c='*';
  if (p == 3) c='\n';
  if (!debug) c = '.';
  fputc(c,stderr);
}
  
extern int proxy_verify_cert_chain(X509 * ucert, STACK_OF(X509) * cert_chain, proxy_verify_desc * pvd);
extern void proxy_verify_ctx_init(proxy_verify_ctx_desc * pvxd);
}

static bool checkstats(char *file, int mode, bool debug);

proxy_manager::proxy_manager(std::string certfilestr,
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
                             bool nokey) :
    cacertfile(NULL),
    certdir(NULL),
    certfile(NULL),
    keyfile(NULL),
    starttime(0),
    endtime(0),
    bits(bitlen),
    hours(cred_hours),
    minutes(cred_minutes),
    ac_hours(cred_hours),
    ac_minutes(cred_minutes),
    limit_proxy(false),
    proxyver(proxy_type),
    policyfile(p_data),
    policylang(p_lang),
    pathlength(-1),
    globus_version(0),
    ordering(""),
    targetlist(""),
    voID(""),
    cert_chain(NULL),
    ucert(NULL),
    private_key(NULL),
    timeout(to) {
    
    std::vector<std::string> order;
    std::vector<std::string> targets;

    if(curr_pwd) {
        free(curr_pwd);
    }    
    curr_pwd = strdup(passphrase.c_str());
  
    /* set globus version */

    globus_version = globus(globus_version);
    if (globus_version == 0) {
        globus_version = 22;
        if(debug) std::cout << "Unable to discover Globus version: trying for 2.2" << std::endl;
    }
    else if(debug) std::cout << "Detected Globus version: " << globus_version << std::endl;
  
    /* PCI extension option */ 
  
    if(proxyver >= 3){
        if(debug) {
            std::cout << "PCI extension info: " << std::endl << " Path length: " << pathlength << std::endl;
            if(policylang.empty()) {
                std::cout << " Policy language not specified." << policylang << std::endl;
            } else { 
                std::cout << " Policy language: " << policylang << std::endl;
            }
            if(policyfile.empty()) {
                std::cout << " Policy file not specified." << std::endl;
            } else { 
                std::cout << " Policy file: " << policyfile << std::endl;
            }
        }
    }
  
    /* controls that number of bits for the key is appropiate */

    if((bits!=512) && (bits!=1024) && (bits!=2048) && (bits!=4096)) {
    
        std::cerr << "Error: number of bits in key must be one of 512, 1024, 2048, 4096." << std::endl;
        std::cerr << "Selected 1024" << std::endl;
        bits = 1024;
        
    }else if(debug) std::cout << "Number of bits in key :" << bits << std::endl; 
  
    /* parse order and target vector to a comma-separated list */
    for (std::vector<std::string>::iterator i = order.begin(); i != order.end(); i++)
        ordering += (i == order.begin() ? std::string("") : std::string(",")) + FQANParse(*i).substr(1);

    for (std::vector<std::string>::iterator i = targets.begin(); i != targets.end(); i++)
        targetlist += (i == targets.begin() ? ("") : std::string(",")) + *i;  

    cacertfile = NULL;
    if ( certdirstr.empty() ) {
        throw VOMSError(VOMSERR_WRONG_CADIR);
    } else {
        certdir = strdup(certdirstr.c_str());
    }
    
    if ( outfilestr.empty() ) {
    
        std::stringstream tmpproxyname;
        tmpproxyname << "/tmp/x509up_u" << getuid();
        proxyfile = tmpproxyname.str();

    } else {
    
        proxyfile = outfilestr;

    }
    
    if ( certfilestr.empty() ) {
        throw VOMSError(VOMSERR_WRONG_CERTFILE);
    } else {
        certfile = strdup(certfilestr.c_str());
    }
    
    if ( keyfilestr.empty() ) {
        throw VOMSError(VOMSERR_WRONG_KEYFILE);
    } else {
        keyfile = strdup(keyfilestr.c_str());
    }

    /*
     * TODO verify if it is required
     */
    if (!certdirstr.empty())
        setenv("X509_CERT_DIR", certdirstr.c_str(), 1);

    ERR_load_prxyerr_strings(0);
    SSLeay_add_ssl_algorithms();

    if (!checkstats(certfile, S_IXUSR | S_IWGRP | S_IXGRP | S_IWOTH | S_IXOTH, debug) ||
        !checkstats(keyfile, S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IRGRP |
                  S_IWOTH | S_IXOTH, debug)) {
        Error();
        throw VOMSError(VOMSERR_KEYS_NOACCESS);
    }
  
    if ( nokey ) {
        if ( !load_credentials(certfile, NULL, &ucert, &cert_chain, NULL, NULL) ) {
            Error();
            throw VOMSError(VOMSERR_KEYS_NOACCESS);
        }
    } else {
        if ( !load_credentials(certfile, keyfile, &ucert, &cert_chain, &private_key, 
                        (int (*)())(pwd_callback)) ) {
            Error();
            throw VOMSError(VOMSERR_BAD_PWD);
        }
    }
    
    char* s = X509_NAME_oneline(X509_get_subject_name(ucert),NULL,0);
    std::cout << "Your identity: " << s << std::endl;
    OPENSSL_free(s);
        
    if ( get_cert_lifetime() < 0 ) {
        throw VOMSError(VOMSERR_EXP_CERT);
    }


}

proxy_manager::~proxy_manager() {

    if (cert_chain)
        sk_X509_pop_free(cert_chain, X509_free);
    if (ucert)
        X509_free(ucert);
    if (private_key)
        EVP_PKEY_free(private_key);
    free(cacertfile);
    free(certdir);
    free(certfile);
    free(keyfile);

    if(curr_pwd) {
        free(curr_pwd);
        curr_pwd = NULL;
    }    

    /*
     * TODO this is the cause of the sigfault
     */
    //OBJ_cleanup();

}

void proxy_manager::build_proxy(contactdata vomssrv_data) throw(VOMSError) {

    contactdata currsrv_data = vomssrv_data;
    std::string buffer;
    std::string serror;
    std::string cerror;
    bool status = false;
    
    /*
     * TODO verify memleak with ac list
     */
    std::vector<AC*> acvector;

    vomsdata v_data("",certdir);
    v_data.SetVerificationType(VERIFY_FULL);
    v_data.SetLifetime(ac_hours * 3600 + ac_minutes * 60);
    v_data.Order(ordering);
    v_data.AddTarget(targetlist);

    /* will contain all fqans requested for the vo */
    std::vector<std::string> fqans;

    /*
     * The vo field is used also for carrying groups and roles
     * format: <vo>:<groups>/ROLE=<role>
     */    
    std::string::size_type pos = currsrv_data.vo.find(':');
    if (pos != std::string::npos) {
        
        voID = currsrv_data.vo.substr(0, pos);
        fqans.push_back("/" + voID + currsrv_data.vo.substr(pos+1));  
            
    } else {
    
        voID = currsrv_data.vo;
        fqans.push_back("/" + voID);
        
    }
    
    /* parse fqans vector to build the command to send to the server */
    std::string command = build_cmd_from_fqan(fqans);
    if( command.empty() ){
        std::cerr << "Unable to post command to server" << std::endl;
        throw VOMSError(VOMSERR_INTERNAL);
    }
    
    /* and contact them */

    v_data.LoadCredentials(ucert, private_key, cert_chain);

    if ( get_cert_lifetime() < ac_hours*60*60 + ac_minutes*60 ) {
        throw VOMSError(VOMSERR_CERT_BEFORE_PRX);
    }

    /* contact server */
    std::cout << "Contacting " << " " << currsrv_data.host << ":" << currsrv_data.port
              << " [" << currsrv_data.contact << "] \"" << currsrv_data.vo 
              << "\" " << command << std::flush;
                           
    status = v_data.Contact(currsrv_data.host, currsrv_data.port, 
                            currsrv_data.contact, command, timeout);
                  
    serror = v_data.ServerErrors();
    cerror = v_data.ErrorMessage();

    if ( status ) {
    
        for( std::vector<voms>::iterator v_item = v_data.data.begin();
                v_item != v_data.data.end();
                v_item++ ) {
            AC *tmpAC = v_item->GetAC();
            acvector.push_back(tmpAC);  
        }
    
        std::cout << " Done" << std::endl;
    
        if ( !serror.empty() )
            std::cerr << std::endl << "Warning: " << serror << std::endl;
    
    } else {
        std::cout << " Failed" << std::endl;
        if (v_data.error == VERR_NOSOCKET ) Error();
    
        if ( !serror.empty() )
            std::cerr << std::endl << "Error: " << serror << std::endl;
    
        if ( serror.empty() && !cerror.empty() )
            std::cerr << std::endl << "Error: " << cerror << std::endl;
        throw VOMSError(VOMSERR_SERVER_ERROR);
    }
        
    std::cout << "Creating proxy " << std::flush; 
    if(debug) std::cout << "to " << proxyfile << " " << std::flush;
    if(CreateProxy(acvector, proxyver)) {
        Error();
        throw VOMSError(VOMSERR_CANT_CREATE);
    }
    
    verify_proxy();

}

bool proxy_manager::CreateProxy(std::vector<AC*>& acvector, int version) {

    std::string confstr = "digitalSignature: hu, keyEncipherment: hu, dataEncipherment: hu";
    std::string proxy_certinfo_str;
    X509_ext_list_ptr extensions;

    X509 * ncert = NULL;    
    EVP_PKEY * npkey = NULL;
    X509_REQ * req = NULL;

    BIO * bp = NULL;
    FILE * fpout = NULL;
    int fdout = -1;
    int try_counter = 3;  /* try 3 times in case of asynchrounous calls */

    bool addext_res = false;

    static int init = 0;

    if (!init) {
        InitProxyCertInfoExtension(1);
        init = 1;
    }
  
    if (proxy_genreq(ucert, &req, &npkey, bits, NULL, (int (*)())kpcallback))
        goto err;


    /* AC extension  */

    if (acvector.size()>0) {
        char **tmplist = (char **)malloc((acvector.size()+1) * sizeof(AC*));
    
        for(unsigned int k=0; k<acvector.size(); k++){
            tmplist[k] = (char *)acvector[k];
        }
        tmplist[acvector.size()] = NULL;

        addext_res = extensions.add(NULL, OBJ_txt2nid("acseq"), (char *)tmplist, false);

        free(tmplist);
    
    }

    addext_res = extensions.add(NULL, NID_key_usage, confstr.c_str(), true);

    /* vo extension */
/*  
    if (!voID.empty()) {
  
        if ((ex4 = CreateProxyExtension("vo", voID)) == NULL) {
            PRXYerr(PRXYERR_F_PROXY_SIGN,PRXYERR_R_CLASS_ADD_EXT);
            goto err;
        }
    
        if (!sk_X509_EXTENSION_push(extensions, ex4)) {
            PRXYerr(PRXYERR_F_PROXY_SIGN,PRXYERR_R_CLASS_ADD_EXT);
            goto err;
        }
    
        vo = true;
    }
*/
  
    /* PCI extension */
  
    if (version>=3) {

        std::string policy;
        ASN1_OBJECT* policy_language;
    
        /* getting contents of policy file */
  
        std::ifstream fp;
        if(!policyfile.empty()) {
            fp.open(policyfile.c_str());
            if(!fp) {
                std::cerr << std::endl << "Error: can't open policy file" << std::endl;
                goto err;
            }
            fp.unsetf(std::ios::skipws);
            char c;
            while(fp.get(c)) {
                policy += c;
            }
        }
    
        /* setting policy language field */
    
        if(policylang.empty()) {
            if(policyfile.empty()) {
                policylang = IMPERSONATION_PROXY_OID;
                if(debug) std::cout << "No policy language specified, Gsi impersonation proxy assumed." << std::endl;
            } else {
                policylang = GLOBUS_GSI_PROXY_GENERIC_POLICY_OID;
                if(debug) std::cout << "No policy language specified with policy file, assuming generic." << std::endl;
            }
        }
    
        /* predefined policy language can be specified with simple name string */
    
        else if(policylang == IMPERSONATION_PROXY_SN)
            policylang = IMPERSONATION_PROXY_OID;
        else if(policylang == INDEPENDENT_PROXY_SN)
            policylang = INDEPENDENT_PROXY_OID;
    
        /* does limited prevale on others? don't know what does grid-proxy_init since if pl is given with
           limited options it crash */
        if(limit_proxy)
            policylang = LIMITED_PROXY_OID;

        OBJ_create((char *)policylang.c_str(), (char *)policylang.c_str(), (char *)policylang.c_str());
    
        if(!(policy_language = OBJ_nid2obj(OBJ_sn2nid(policylang.c_str())))) {
            PRXYerr(PRXYERR_F_PROXY_SIGN, PRXYERR_R_CLASS_ADD_OID);
            goto err;
        }
    
        int nativeopenssl = proxynative();

        if (version == 3 || (version == 4 && !nativeopenssl)) {
    
            myPROXYPOLICY *proxypolicy = myPROXYPOLICY_new();
            if(policy.size()>0) {
                myPROXYPOLICY_set_policy(proxypolicy, (unsigned char *)policy.c_str(), policy.size());
            }
            myPROXYPOLICY_set_policy_language(proxypolicy, policy_language);

            myPROXYCERTINFO *proxycertinfo = myPROXYCERTINFO_new();
            myPROXYCERTINFO_set_version(proxycertinfo, version);
            myPROXYCERTINFO_set_proxypolicy(proxycertinfo, proxypolicy);
            if(pathlength>=0) {
                myPROXYCERTINFO_set_path_length(proxycertinfo, pathlength);
            }
            
            proxy_certinfo_str = (char *)proxycertinfo;
            
            myPROXYPOLICY_free(proxypolicy);
            myPROXYCERTINFO_free(proxycertinfo);
            
        } else {
            std::ostringstream os;

            os << "language:" << policylang;

            if (pathlength != -1) 
                os << ",pathlen:" << pathlength;

            if (!policy.empty())
                os << ",policy:text:" << policy;
            proxy_certinfo_str = os.str();
        }

        if (version == 3){
        
            addext_res = extensions.add(NULL, OBJ_obj2nid(OBJ_txt2obj(PROXYCERTINFO_V3,1)),
                                        proxy_certinfo_str.c_str(), false);
            
        }else if (nativeopenssl){
        
            X509V3_CTX ctx;
            X509V3_set_ctx(&ctx, NULL, NULL, NULL, NULL, 0L);
            ctx.db = (void*)&ctx;
            X509V3_CONF_METHOD method = { NULL, NULL, NULL, NULL };
            ctx.db_meth = &method;
            addext_res = extensions.add(&ctx, OBJ_obj2nid(OBJ_txt2obj(PROXYCERTINFO_V4,1)),
                                        proxy_certinfo_str.c_str(), (version == 4));
            
        }else{
        
            addext_res = extensions.add(NULL, OBJ_obj2nid(OBJ_txt2obj(PROXYCERTINFO_V4,1)),
                                        proxy_certinfo_str.c_str(), (version == 4));
        }

    }


    if (proxy_sign(ucert,
                   private_key,
                   req,
                   &ncert,
                   hours*60*60 + minutes*60,
                   extensions.list(),
                   limit_proxy,
                   version,
                   NULL)) {
        goto err;
    }
    
  
    while ( (try_counter > 0) && (fdout < 0) ) {
        /* We always unlink the file first; it is the only way to be
         * certain that the file we open has never in its entire lifetime
         * had the world-readable bit set.  
         */
        unlink(proxyfile.c_str());
    
        /* Now, we must open w/ O_EXCL to make certain that WE are 
         * creating the file, so we know that the file was BORN w/ mode 0600.
         * As a bonus, O_EXCL flag will cause a failure in the precense
         * of a symlink, so we are safe from zaping a file due to the
         * presence of a symlink.
         */
        fdout = open(proxyfile.c_str(), O_WRONLY|O_EXCL|O_CREAT,0600);
        try_counter--;
    }
 
 
    /* Now, make a call to set proper permissions, just in case the
     * user's umask is stupid.  Note this call to fchmod will also
     * fail if our fdout is still -1 because our open failed above.
     */
#ifndef WIN32
    if(fchmod(fdout, S_IRUSR|S_IWUSR) < 0) {
        PRXYerr(PRXYERR_F_LOCAL_CREATE, PRXYERR_R_PROBLEM_PROXY_FILE);
        ERR_add_error_data(2, "\nchmod failed for file ", proxyfile.c_str());
        goto err;
    }
#endif 
  
    /* Finally, we have a safe fd.  Make it a stream like ssl wants. */
    fpout = fdopen(fdout, "w");


    if ((bp = BIO_new(BIO_s_file())) != NULL) {
    
        BIO_set_fp(bp, fpout, BIO_NOCLOSE);
  
        if (proxy_marshal_bp(bp, ncert, npkey, ucert, cert_chain)) {
            BIO_free(bp);
            fclose(fpout);
            goto err;
        } else {
            BIO_free(bp);
        }
    }
    
    if ( fpout ) {
        fclose(fpout);
    }
    
    std::cout << " Done" << std::endl;

    return false;






err:

    if (ncert)
        X509_free(ncert);
    if(req) {
        X509_REQ_free(req);
    }
  
    if(npkey) EVP_PKEY_free(npkey);
    
    
    return true;

}

/*
X509_EXTENSION * proxy_manager::CreateProxyExtension(std::string name, std::string data, bool crit) 
{

  X509_EXTENSION *                    ex = NULL;
  ASN1_OBJECT *                       ex_obj = NULL;
  ASN1_OCTET_STRING *                 ex_oct = NULL;
  
  if(!(ex_obj = OBJ_nid2obj(OBJ_txt2nid((char *)name.c_str())))) {
    PRXYerr(PRXYERR_F_PROXY_SIGN,PRXYERR_R_CLASS_ADD_OID);
    goto err;
  }
  
  if(!(ex_oct = ASN1_OCTET_STRING_new())) {
    PRXYerr(PRXYERR_F_PROXY_SIGN,PRXYERR_R_CLASS_ADD_EXT);
    goto err;
  }
  
  ex_oct->data = (unsigned char *)data.c_str();
  ex_oct->length = data.size();
  
  if (!(ex = X509_EXTENSION_create_by_OBJ(NULL, ex_obj, crit, ex_oct))) {
    PRXYerr(PRXYERR_F_PROXY_SIGN,PRXYERR_R_CLASS_ADD_EXT);
    goto err;
  }
	
  //  ASN1_OCTET_STRING_free(ex_oct);
  //  ASN1_OBJECT_free(ex_obj);
  ex_oct = NULL;
	
  return ex;
  
 err:
  
  if (ex_oct)
    ASN1_OCTET_STRING_free(ex_oct);
  
  if (ex_obj)
    ASN1_OBJECT_free(ex_obj);
  
  return NULL;
  
}
*/

void proxy_manager::verify_proxy() throw(VOMSError) {

    proxy_verify_desc pvd;
    proxy_verify_ctx_desc pvxd;
    X509_cert_ptr cert_wrapper;
    X509_stack_ptr chain_wrapper;
      
    if ( cert_wrapper.load(proxyfile.c_str()) ) {
        /*
         * TODO throw error
         */
    }
    
    if ( chain_wrapper.load(proxyfile.c_str()) ) {
        /*
         * TODO throw error
         */
    }

    proxy_verify_ctx_init(&pvxd);
    proxy_verify_init(&pvd, &pvxd);
    pvxd.certdir = this->certdir;
    
    if ( proxy_verify_cert_chain(cert_wrapper.cert(), chain_wrapper.stack(), &pvd) ) {

        vomsdata v_data;

        if ( ! v_data.Retrieve(cert_wrapper.cert(), chain_wrapper.stack(), RECURSE_CHAIN) ) {
            if ( v_data.error != VERR_NOEXT ) {
                std::cerr << "Error: verify failed: " << v_data.ErrorMessage() << std::endl;
                throw VOMSError(VOMSERR_VER_FAILED);
            }
        }
        
        /*
         * TODO load voname and start/endtime
         * start/endtime is the minimun between proxy and AC time
         */
        if ( v_data.data.size()>0 ) {
            std::cout << "  AC Not Before: " << v_data.data[0].date1 << std::endl;
            std::cout << "  AC Not After: " << v_data.data[0].date2 << std::endl;
            if ( voID.empty() ) {
                voID = v_data.data[0].voname;
            }
        }
        
        starttime = ASN1_UTCTIME_mktime(X509_get_notBefore(cert_wrapper.cert()));
        endtime = ASN1_UTCTIME_mktime(X509_get_notAfter(cert_wrapper.cert()));
        std::cout << "Your proxy is valid until " << asctime(gmtime(&endtime)) << std::endl;
        
    } else {
        //Error();
        std::vector<int> err_reasons;
        get_ssl_error(err_reasons);
        for ( std::vector<int>::iterator reason = err_reasons.begin();
              reason != err_reasons.end();
              reason++ ) {
            if ( (*reason) == PRXYERR_R_REMOTE_CRED_EXPIRED )
                throw VOMSError(VOMSERR_EXP_PROXY);
        } 
        throw VOMSError(VOMSERR_VER_FAILED);
    }
}

time_t proxy_manager::get_cert_lifetime() {

    ASN1_UTCTIME * asn1_time = ASN1_UTCTIME_new();
    X509_gmtime_adj(asn1_time, 0);
    time_t time_now = ASN1_UTCTIME_mktime(asn1_time);
    ASN1_UTCTIME_free(asn1_time);
    time_t time_after = ASN1_UTCTIME_mktime(X509_get_notAfter(ucert));
    
    return time_after - time_now ;
}

time_t proxy_manager::get_proxy_starttime() {
    return starttime;
}

time_t proxy_manager::get_proxy_endtime() {
    return endtime;
}

std::string proxy_manager::get_vo_id() {
    return voID;
}

static bool checkstats(char *file, int mode, bool debug)
{
  struct stat stats;

  if (stat(file, &stats) == -1) {
    std::cerr << "Unable to find user certificate or key" << std::endl;
    return false;
  }

  if (stats.st_mode & mode) {
    std::cerr << std::endl << "ERROR: Couldn't find valid credentials to generate a proxy." << std::endl 
              << "Use --debug for further information." << std::endl;
    if(debug)
      std::cout << "Wrong permissions on file: " << file << std::endl;

    return false;
  }
  return true;
}

void proxy_manager::Error() {

  unsigned long l;
  char buf[256];
#if SSLEAY_VERSION_NUMBER  >= 0x00904100L
  const char *file;
#else
  char *file;
#endif
  char *dat;
  int line;
    
  /* WIN32 does not have the ERR_get_error_line_data */ 
  /* exported, so simulate it till it is fixed */
  /* in SSLeay-0.9.0 */
  
  while ( ERR_peek_error() != 0 ) {
    
    int i;
    ERR_STATE *es;
      
    es = ERR_get_state();
    i = (es->bottom+1)%ERR_NUM_ERRORS;
    
    if (es->err_data[i] == NULL)
      dat = strdup("");
    else
      dat = strdup(es->err_data[i]);

    if (dat) {
      l = ERR_get_error_line(&file, &line);

      if (debug)
        std::cerr << ERR_error_string(l,buf) << ":"
                  << file << ":" << line << dat << std::endl;
      else
        std::cerr << ERR_reason_error_string(l) << dat
                  << "\nFunction: " << ERR_func_error_string(l) << std::endl;
    }
    
    free(dat);
  }  
}

void proxy_manager::get_ssl_error(std::vector<int>& err_reasons) {
    while ( ERR_peek_error() != 0 ) {
        unsigned long l = ERR_get_error();
        err_reasons.push_back(ERR_GET_REASON(l));
    }
}








X509_cert_ptr::X509_cert_ptr() {
    cert_ptr = NULL;
}

X509_cert_ptr::~X509_cert_ptr() {
    if ( cert_ptr ) {
        X509_free(cert_ptr);
    }
}

X509* X509_cert_ptr::cert() {
    return cert_ptr;
}

int X509_cert_ptr::load(const char *certname) {
    unsigned long hSession = 0;
    return proxy_load_user_cert(certname, &cert_ptr, NULL, NULL, &hSession);
}



X509_stack_ptr::X509_stack_ptr() {
    stack_ptr = sk_X509_new_null();
}

X509_stack_ptr::~X509_stack_ptr() {
    if ( stack_ptr ) {
        sk_X509_pop_free(stack_ptr, X509_free);
    }
}

STACK_OF(X509)* X509_stack_ptr::stack() {
    return stack_ptr;
}

int X509_stack_ptr::load(const char *certname) {
    return proxy_load_user_proxy(stack_ptr, certname, NULL);
}

X509_ext_list_ptr::X509_ext_list_ptr() {
    ext_list_ptr = sk_X509_EXTENSION_new_null();
    
    if ( ext_list_ptr == NULL ) {
        PRXYerr(PRXYERR_F_PROXY_SIGN, PRXYERR_R_CLASS_ADD_EXT);
        /*
         * TODO throw exception
         */
    }
}

X509_ext_list_ptr::~X509_ext_list_ptr() {
    sk_X509_EXTENSION_pop_free(ext_list_ptr, X509_EXTENSION_free);
    std::cout << "Freeing extension list (verify memleak)" << std::endl;
    /*
     * TODO check memleak for contained extensions
     */
}

STACK_OF(X509_EXTENSION)* X509_ext_list_ptr::list() {
    return ext_list_ptr;
}

bool X509_ext_list_ptr::add(X509V3_CTX *ctx, int ext_nid, const char *value, bool critical) {
    X509_EXTENSION *ext = NULL;

    /*
     * TODO check the cast to char*
     */
    if ((ext = X509V3_EXT_conf_nid(NULL, ctx, ext_nid, (char*)value)) == NULL) {
        PRXYerr(PRXYERR_F_PROXY_SIGN, PRXYERR_R_CLASS_ADD_EXT);
        return false;        
    }
    
    if ( critical ) {
        X509_EXTENSION_set_critical(ext, 1);
    }
    
    if (!sk_X509_EXTENSION_push(ext_list_ptr, ext)) {
        PRXYerr(PRXYERR_F_PROXY_SIGN, PRXYERR_R_CLASS_ADD_EXT);
        return false;
    }
    
    return true;
}




