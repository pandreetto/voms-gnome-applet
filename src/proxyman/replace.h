/*********************************************************************
 *
 * Authors: Vincenzo Ciaschini - Vincenzo.Ciaschini@cnaf.infn.it 
 *          Valerio Venturi    - Valerio.Venturi@cnaf.infn.it
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
#ifndef VOMS_REPLACES_H
#define VOMS_REPLACES_H
#include "config.h"

#ifndef HAVE_GLOBUS_OFF_T
#ifdef HAVE_LONG_LONG_T
#define GLOBUS_OFF_T long long
#else
#define GLOBUS_OFF_T long
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif
#ifndef HAVE_DAEMON
extern int daemon(int, int);
#endif
#ifndef HAVE_SETENV
extern int setenv(const char *, const char *, int);
//extern void unsetenv(const char *);
#endif
#ifndef HAVE_STRNDUP
#include <string.h>
extern char *strndup(const char *, size_t);
#endif
#ifdef __cplusplus
}
#endif
#endif /* REPLACES_H */
