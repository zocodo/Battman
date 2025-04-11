//
//  common.h
//  Battman
//
//  Created by Torrekie on 2025/1/21.
//

#ifndef common_h
#define common_h

#include <stdbool.h>
#include <dlfcn.h>
#include <os/base.h>
#include "main.h"

#ifdef DEBUG
#define DBGLOG(...) NSLog(__VA_ARGS__)
#define DBGALT(x, y, z) show_alert(x, y, z)
#else
#define DBGLOG(...)
#define DBGALT(x, y, z)
#endif

#define LICENSE_MIT 'MIT'
#define LICENSE_GPL 'GPL'
#define LICENSE_NONFREE 'NONF'

#ifndef LICENSE
#define LICENSE LICENSE_MIT
#endif

#define IOS_CONTAINER_FMT "^/private/var/mobile/Containers/Data/Application/[0-9A-Fa-f\\-]{36}$"
#define MAC_CONTAINER_FMT "^/Users/[^/]+/Library/Containers/[^/]+/Data$"
#define SIM_CONTAINER_FMT "^/Users/[^/]+/Library/Developer/CoreSimulator/Devices/[0-9A-Fa-f\\-]{36}/data/Containers/Data/Application/[0-9A-Fa-f\\-]{36}$"

__BEGIN_DECLS

bool show_alert(const char *title, const char *message, const char *cancel_button_title);
void show_alert_async(const char *title, const char *message, const char *button, void (^completion)(bool result));

char *preferred_language(void);
bool libintl_available(void);
bool gtk_available(void);

void app_exit(void);
bool is_carbon(void);

bool match_regex(const char *string, const char *pattern);

__END_DECLS

#endif /* common_h */
