/*********************************************************************
 *
 * Authors: Vincenzo Ciaschini - Vincenzo.Ciaschini@cnaf.infn.it 
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

#ifndef VOMS_CREDENTIALS_H
#define VOMS_CREDENTIALS_H

#include <openssl/x509.h>
#include <openssl/evp.h>

#include "gssapi_compat.h"

extern int globus(int);
extern X509 *get_real_cert(X509 *base, STACK_OF(X509) *stk);
extern int get_issuer(X509 *cert, char **buffer);
extern EVP_PKEY *get_private_key(void *credential);
extern char *get_peer_serial(X509 *);

extern X509 *decouple_cred(gss_cred_id_t credential, STACK_OF(X509) **stk);
extern EVP_PKEY *get_delegated_public_key(gss_ctx_id_t context, int globusver);
extern int get_own_data(gss_cred_id_t credential, EVP_PKEY **key, char **issuer, X509 **pcert);

X509 *
load_cert_name(const char *filename, STACK_OF(X509) **stack, EVP_PKEY **key);

#endif
