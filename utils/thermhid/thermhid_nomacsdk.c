#include <stdio.h>
#include <getopt.h>
#include <ctype.h>

#include <CoreFoundation/CoreFoundation.h>

typedef void *IOHIDEventSystemClientRef;
typedef void *IOHIDServiceClientRef;
typedef unsigned long long IOHIDEventType;
typedef unsigned long long IOHIDEventField;

// Sadly we still have to extern these, the public IOHIDEventSystemClient.h has no such def
extern IOHIDEventSystemClientRef IOHIDEventSystemClientCreate(CFAllocatorRef allocator);
extern void IOHIDEventSystemClientSetMatching(IOHIDEventSystemClientRef, CFDictionaryRef);

// Which header should this belong? IOHIDEvent.h? The OSS one contains only C++
typedef struct __IOHIDEvent *IOHIDEventRef;
extern CFArrayRef IOHIDEventGetChildren(IOHIDEventRef event);
extern IOHIDEventType IOHIDEventGetType(IOHIDEventRef event);
extern IOHIDEventRef IOHIDServiceClientCopyEvent(IOHIDServiceClientRef service, int64_t type, int32_t options, int64_t timestamp);
extern double IOHIDEventGetFloatValue(IOHIDEventRef event, IOHIDEventField field);

// IOHIDEventQueue.h
typedef struct __IOHIDEventQueue *IOHIDEventQueueRef;

// IOHIDEventSystemClientPrivate.h
typedef void(*IOHIDEventSystemClientEventCallback)(void* target, void* refcon, IOHIDEventQueueRef queue, IOHIDEventRef event);
extern void IOHIDEventSystemClientScheduleWithRunLoop(IOHIDEventSystemClientRef client, CFRunLoopRef runloop, CFStringRef mode);
extern void IOHIDEventSystemClientRegisterEventCallback(IOHIDEventSystemClientRef client, IOHIDEventSystemClientEventCallback callback, void *target, void *refcon);

extern void IOHIDServiceClientSetProperty(void*,CFStringRef,CFNumberRef);
extern CFTypeRef IOHIDEventSystemClientCopyServices(void*);
extern CFTypeRef IOHIDServiceClientCopyProperty(void*,CFStringRef);

typedef struct {
	uint32_t page;
	uint32_t usage;
	IOHIDEventType event;
	IOHIDEventField field;
	char *sensor;
	char *value_type;
} usage_grp;

#define kHIDPage_AppleVendor 0xff00
#define kHIDUsage_AppleVendor_TemperatureSensor 5
#define kIOHIDEventTypeTemperature 15
#define kIOHIDEventFieldTemperatureLevel 15<<16
#define kHIDPage_AppleVendorPowerSensor 0xff08
#define kHIDUsage_AppleVendorPowerSensor_Power 1
#define kIOHIDEventTypePower 25
#define kIOHIDEventFieldPowerMeasurement 25<<16
#define kHIDUsage_AppleVendorPowerSensor_Current 2
#define kHIDUsage_AppleVendorPowerSensor_Voltage 3

static usage_grp temp_grp = {
    kHIDPage_AppleVendor,
    kHIDUsage_AppleVendor_TemperatureSensor,
    kIOHIDEventTypeTemperature,
    kIOHIDEventFieldTemperatureLevel,
    "temperature",
    "TEMP (C)"
};

static usage_grp pwr_grp = {
    kHIDPage_AppleVendorPowerSensor,
    kHIDUsage_AppleVendorPowerSensor_Power,
    kIOHIDEventTypePower,
    kIOHIDEventFieldPowerMeasurement,
    "power",
    "POWER (W)"
};

static usage_grp curr_grp = {
    kHIDPage_AppleVendorPowerSensor,
    kHIDUsage_AppleVendorPowerSensor_Current,
    kIOHIDEventTypePower,
    kIOHIDEventFieldPowerMeasurement,
    "current",
    "CURRENT (A)"
};

static usage_grp volt_grp = {
    kHIDPage_AppleVendorPowerSensor,
    kHIDUsage_AppleVendorPowerSensor_Voltage,
    kIOHIDEventTypePower,
    kIOHIDEventFieldPowerMeasurement,
    "voltage",
    "VOLTAGE (V)"
};

void usage(void)
{
    fprintf(stderr, "usage: %s [-b] [-t type] [locID]\n", getprogname());
    fwrite("\t-b\ttry callbacks for unpollable sensors\n", 41, 1, stderr);
    fwrite("\t-t\tspecify sensor type:\n", 25, 1, stderr);
    fwrite("\t\tt - temperature (default)\n", 28, 1, stderr);
    fwrite("\t\tv - voltage\n", 14, 1, stderr);
    fwrite("\t\ti - current\n", 14, 1, stderr);
    fwrite("\t\tp - power\n", 12, 1, stderr);

    exit(EXIT_FAILURE);
}

int format_print(CFNumberRef id, CFStringRef prod) {
    int32_t ptr;
    char buffer[40];

    if (id && CFNumberGetValue(id, kCFNumberSInt32Type, &ptr) && ptr != -1) {
        if (iscntrl(ptr >> 24))
            printf("%3ld ", (long)ptr);
        else
            printf("%c%c%c%c", ptr >> 0x18, ptr >> 0x10, ptr >> 0x08, ptr >> 0x00);
    } else {
        printf("%3s ", "--");
    }

    if (!prod || !CFStringGetCString(prod, buffer, 40, kCFStringEncodingUTF8))
        strcpy(buffer, "--");

    return printf("   %-40s ", buffer);
}

void set_interval(IOHIDServiceClientRef service, int interval) {
    int buf;
    CFNumberRef number;

    buf = interval;
    number = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &buf);
    IOHIDServiceClientSetProperty(service, CFSTR("ReportInterval"), number);

    if (number) CFRelease(number);
}

static void interval500000(const void *key, const void * value, void *ctx) {
    IOHIDServiceClientRef client;

    CFNumberGetValue((CFNumberRef)key, kCFNumberLongLongType, &client);
    set_interval(client, 500000);
}
static void interval0(const void *key, const void * value, void *ctx) {
    IOHIDServiceClientRef client;

    CFNumberGetValue((CFNumberRef)key, kCFNumberLongLongType, &client);
    set_interval(client, 0);
}

void event_callback(void *target, void *refcon, IOHIDEventQueueRef queue, IOHIDEventRef event) {
    CFNumberRef number;

    if (IOHIDEventGetType(event) == kIOHIDEventTypeTemperature) {
        CFIndex count = 0;
        CFArrayRef child = IOHIDEventGetChildren(event);
        if (child) count = CFArrayGetCount(child);

        number = CFNumberCreate(kCFAllocatorDefault, kCFNumberLongLongType, &queue);
        if (CFDictionaryContainsKey((CFDictionaryRef)refcon, number)) {
            CFDictionaryRef dict = CFDictionaryGetValue((CFDictionaryRef)refcon, number);
            if (dict) {
                int i = 0;
                CFNumberRef LocationID;
                CFStringRef Product;
                do {
                    if (count < 2 || CFDictionaryGetValue(dict, CFSTR("Product"))) {
                        LocationID = CFDictionaryGetValue(dict, CFSTR("LocationID"));
                        Product = CFDictionaryGetValue(dict, CFSTR("Product"));
                        format_print(LocationID, Product);
                    } else {
                        LocationID = CFDictionaryGetValue(dict, CFSTR("LocationID"));
                        Product = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%d"), i);
                        format_print(LocationID, Product);
                        if (Product) CFRelease(Product);
                    }
                    printf("%g\n", IOHIDEventGetFloatValue(event, kIOHIDEventFieldTemperatureLevel));

                    if (count <= i) break;

                    event = (IOHIDEventRef)CFArrayGetValueAtIndex(child, i++);
                } while (event);
            }
            CFDictionaryRemoveValue(refcon, number);
            if (!CFDictionaryGetCount(refcon)) {
                CFRunLoopRef current = CFRunLoopGetCurrent();
                CFRunLoopStop(current);
            }
        }
        if (number) CFRelease(number);
    }
}

int main(int argc, char *argv[]) {
    int do_callbacks = 0;
    int option;
    usage_grp *selected_grp;

    static CFMutableDictionaryRef callback_values;

    selected_grp = &temp_grp;

    // This is how Apple originally do
repeat:
    while ((option = getopt(argc, argv, "bt:")) != -1) {
        switch (option) {
            case 'b': {
                do_callbacks = 1;
                continue;
            }
            case 't': {
                switch (*optarg) {
                    case 't':
                        goto repeat;
                    case 'v':
                        selected_grp = &volt_grp;
                        goto repeat;
                    case 'i':
                        selected_grp = &curr_grp;
                        goto repeat;
                    case 'p':
                        selected_grp = &pwr_grp;
                        goto repeat;
                }
            }
        }
        usage();
    }

    IOHIDEventSystemClientRef client = IOHIDEventSystemClientCreate(kCFAllocatorDefault);
    if (!client) {
        fwrite("could not create HIDEventSystem\n", 32, 1, stderr);
        goto ERR_1;
    }

    int argc_locID = argc - optind;
    char *locID = argv[optind];
    int buf;
    CFIndex count;
    void *keys[3] = {(void *)CFSTR("PrimaryUsagePage"), (void *)CFSTR("PrimaryUsage"), (void *)CFSTR("LocationID")};
    void *values[3];
    values[0] = (void *)CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &selected_grp->page);
    values[1] = (void *)CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &selected_grp->usage);
    values[2] = NULL;

    if (argc_locID == 0) {
        count = 2;
    } else if (argc_locID != 1) {
        usage();
    } else {
        if (strlen(locID) == 4 && isalpha(locID[0])) {
            buf = locID[0] << 0x18 | locID[1] << 0x10 | locID[2] << 0x08 | locID[3] << 0x00; // Str to char conversion
        } else {
            // Fallback
            buf = (int)strtol(locID, NULL, 0);
        }

        count = 3LL;
        values[2] = (void *)CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &buf);
    }

    CFDictionaryRef matchDict = CFDictionaryCreate(kCFAllocatorDefault, (const void **)keys, (const void **)values, count, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFRelease(values[0]);
    CFRelease(values[1]);
    if (values[2]) CFRelease(values[2]);

    IOHIDEventSystemClientSetMatching(client, matchDict);
    CFRelease(matchDict);

    CFArrayRef services = IOHIDEventSystemClientCopyServices(client);
    if (!services) {
        printf("Found no %s sensors\n", selected_grp->sensor);
        goto ERR_1;
    }

    if (do_callbacks)
        callback_values = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

    printf("Found %ld %s sensors:\n\n", CFArrayGetCount(services), selected_grp->sensor);
    printf("%-3s %-40s %s\n", "ID", "   DESCRIPTION", selected_grp->value_type);
    int i;
    uint64_t ptr; // IOHIDServiceClientRef, but we need to put it at callback_values later
    CFNumberRef LocationID;
    CFStringRef Product;
    if (CFArrayGetCount(services) >= 1) {
        i = 0;
        do {
            ptr = (uint64_t)CFArrayGetValueAtIndex(services, i);
            LocationID = IOHIDServiceClientCopyProperty((IOHIDServiceClientRef)ptr, CFSTR("LocationID"));
            Product = IOHIDServiceClientCopyProperty((IOHIDServiceClientRef)ptr, CFSTR("Product"));

            format_print(LocationID, Product);

            IOHIDEventRef event = IOHIDServiceClientCopyEvent((IOHIDServiceClientRef)ptr, selected_grp->event, 0, 0);
            if (event) {
                printf("%f", IOHIDEventGetFloatValue(event, selected_grp->field));
                CFRelease(event);
            } else {
                printf("--");

                if (do_callbacks) {
                    CFNumberRef callback_addr = CFNumberCreate(kCFAllocatorDefault, kCFNumberLongLongType, &ptr);
                    if (callback_addr) {
                        CFMutableDictionaryRef dict = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
                        if (dict) {
                            if (Product) CFDictionarySetValue(dict, CFSTR("Product"), Product);
                            if (LocationID) CFDictionarySetValue(dict, CFSTR("LocationID"), LocationID);
                        }

                        CFDictionarySetValue(callback_values, callback_addr, dict);
                        CFRelease(callback_addr);
                    }
                }
            }
            putchar('\n');

            if (LocationID) CFRelease(LocationID);
            if (Product) CFRelease(Product);
            ++i;
        } while (CFArrayGetCount(services) > i);
    }
    CFRelease(services);

    if (do_callbacks && CFDictionaryGetCount(callback_values) >= 1) {
        puts("\nSensor callbacks:");
        CFDictionaryApplyFunction(callback_values, interval500000, NULL);
        CFRunLoopRef current = CFRunLoopGetCurrent();
        IOHIDEventSystemClientScheduleWithRunLoop(client, current, kCFRunLoopDefaultMode);
        IOHIDEventSystemClientRegisterEventCallback(client, (IOHIDEventSystemClientEventCallback)event_callback, NULL, callback_values);
        CFRunLoopRunInMode(kCFRunLoopDefaultMode, 2.0, 0);
        CFDictionaryApplyFunction(callback_values, interval0, 0LL);
    }

    CFRelease(client);
    exit(0);

ERR_1:
    exit(1);
}
