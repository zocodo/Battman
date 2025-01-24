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

__BEGIN_DECLS

bool show_alert(const char *title, const char *message, const char *cancel_button_title);

bool libintl_available(void);
bool gtk_available(void);

__END_DECLS

#endif /* common_h */
