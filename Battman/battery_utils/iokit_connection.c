//
//  iokit_connection.c
//  Battman
//
//  Created by Torrekie on 2025/2/9.
//

#include "iokit_connection.h"

#include <stdbool.h>
#include <stdint.h>

hvc_menu_t *convert_hvc(CFDictionaryRef dict, size_t *size, int8_t *index) {
    if (!dict || !size || !index) return NULL;

    CFArrayRef usbHvcMenu = CFDictionaryGetValue(dict, CFSTR("UsbHvcMenu"));
    if (!usbHvcMenu || CFGetTypeID(usbHvcMenu) != CFArrayGetTypeID()) {
        *size = 0;
    }
    CFNumberRef usbHvcHvcIndex = CFDictionaryGetValue(dict, CFSTR("UsbHvcHvcIndex"));
    if (!usbHvcHvcIndex || CFGetTypeID(usbHvcHvcIndex) != CFNumberGetTypeID()) {
        *index = -1;
    } else {
        CFNumberGetValue(usbHvcHvcIndex, kCFNumberSInt8Type, index);
    }

    if (*size == 0 || !usbHvcMenu) return NULL;

    CFIndex count = CFArrayGetCount(usbHvcMenu);
    *size = (size_t)count;

    if (count == 0) {
        return NULL;
    }
    
    // consider use static hvc_menu_t[7] ?
    hvc_menu_t *menu = malloc(count * sizeof(hvc_menu_t));
    if (!menu) {
        *size = 0;
        return NULL;
    }
    
    for (CFIndex i = 0; i < count; i++) {
        CFDictionaryRef entry = CFArrayGetValueAtIndex(usbHvcMenu, i);
        if (!entry || CFGetTypeID(entry) != CFDictionaryGetTypeID()) {
            free(menu);
            *size = 0;
            return NULL;
        }
        
        CFNumberRef currentNum = CFDictionaryGetValue(entry, CFSTR("MaxCurrent"));
        CFNumberRef voltageNum = CFDictionaryGetValue(entry, CFSTR("MaxVoltage"));
        if (!currentNum || CFGetTypeID(currentNum) != CFNumberGetTypeID() ||
            !voltageNum || CFGetTypeID(voltageNum) != CFNumberGetTypeID()) {
            free(menu);
            *size = 0;
            return NULL;
        }
        
        int32_t currentVal = 0, voltageVal = 0;
        if (!CFNumberGetValue(currentNum, kCFNumberSInt32Type, &currentVal) ||
            !CFNumberGetValue(voltageNum, kCFNumberSInt32Type, &voltageVal)) {
            free(menu);
            *size = 0;
            return NULL;
        }
        
        menu[i].current = (uint16_t)currentVal;
        menu[i].voltage = (uint16_t)voltageVal;
    }

    return menu;
}
