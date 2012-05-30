#ifndef MAIN_VOMS_APPLET_CONFIG_H
#define MAIN_VOMS_APPLET_CONFIG_H

#cmakedefine XML_UI_DIR "@XML_UI_DIR@"
#cmakedefine ICON22_DIR "@ICON22_DIR@"
#cmakedefine ICON48_DIR "@ICON48_DIR@"

#ifdef __cplusplus
#include <new>
#endif

#ifdef __GNUC__
#define UNUSED(z)  z __attribute__ ((unused))
#define PRIVATE    __attribute__ ((visibility ("hidden")))
#define PUBLIC     __attribute__ ((visibility ("default")))
#else
#define UNUSED
#define PRIVATE
#define PUBLIC
#endif

#define SSLEAY_VERSION_NUMBER 0x00904100L

#endif

