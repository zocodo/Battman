#import <Foundation/Foundation.h>

#if __has_include(<IOKit/hid/IOHIDDeviceKeys.h>)
#include <IOKit/hid/IOHIDDeviceKeys.h>
#else
#define kIOHIDProductKey "Product"
#define kIOHIDPrimaryUsagePageKey "PrimaryUsagePage"
#define kIOHIDPrimaryUsageKey "PrimaryUsage"
#endif

#if __has_include(<IOKit/hid/AppleHIDUsageTables.h>)
#include <IOKit/hid/AppleHIDUsageTables.h>
#else
#define kHIDPage_AppleVendor 0xFF00
#define kHIDUsage_AppleVendor_TemperatureSensor 0x0005
#endif

#if __has_include(<IOKit/hid/IOHIDEventTypes.h>)
#include <IOKit/hid/IOHIDEventTypes.h>
#else
#define kIOHIDEventTypeTemperature 15
#define kIOHIDEventFieldTemperatureLevel (kIOHIDEventTypeTemperature << 16)
#endif

#if __has_include(<IOKit/hid/IOHIDEvent.h>)
#include <IOKit/hid/IOHIDEvent.h>
#else
CF_IMPLICIT_BRIDGING_ENABLED
#ifdef __LP64__
typedef double IOHIDFloat;
#else
typedef float IOHIDFloat;
#endif
typedef struct __IOHIDEvent * IOHIDEventRef;
typedef uint32_t IOHIDEventField;

extern IOHIDFloat IOHIDEventGetFloatValue(IOHIDEventRef, IOHIDEventField);
CF_IMPLICIT_BRIDGING_DISABLED
#endif

#if __has_include(<IOKit/hidsystem/IOHIDEventSystemClient.h>)
#include <IOKit/hidsystem/IOHIDEventSystemClient.h>
#else
CF_IMPLICIT_BRIDGING_ENABLED
typedef struct CF_BRIDGED_TYPE(id) __IOHIDEventSystemClient * IOHIDEventSystemClientRef;

extern CFArrayRef IOHIDEventSystemClientCopyServices(IOHIDEventSystemClientRef);
CF_IMPLICIT_BRIDGING_DISABLED
#endif

// Sadly this is SPI
#if __has_include(<IOKit/hid/IOHIDEventSystemClient.h>) && 0
#include <IOKit/hid/IOHIDEventSystemClient.h>
#else
CF_IMPLICIT_BRIDGING_ENABLED
extern IOHIDEventSystemClientRef IOHIDEventSystemClientCreate(CFAllocatorRef);
extern void IOHIDEventSystemClientSetMatching(IOHIDEventSystemClientRef, CFDictionaryRef);
CF_IMPLICIT_BRIDGING_DISABLED
#endif

#if __has_include(<IOKit/hidsystem/IOHIDServiceClient.h>)
#include <IOKit/hidsystem/IOHIDServiceClient.h>
#else
CF_IMPLICIT_BRIDGING_ENABLED
typedef struct CF_BRIDGED_TYPE(id) __IOHIDServiceClient * IOHIDServiceClientRef;

extern CFTypeRef _Nullable IOHIDServiceClientCopyProperty(IOHIDServiceClientRef service, CFStringRef key);
CF_IMPLICIT_BRIDGING_DISABLED
#endif
// Should be in IOHIDServiceClient.h but nope
CF_IMPLICIT_BRIDGING_ENABLED
extern IOHIDEventRef IOHIDServiceClientCopyEvent(IOHIDServiceClientRef service, int64_t type, int32_t options, int64_t timestamp);
CF_IMPLICIT_BRIDGING_DISABLED

NSDictionary *getTemperatureHIDData(void) {
    IOHIDEventSystemClientRef client = IOHIDEventSystemClientCreate(0);
	if (!client)
		return nil;

    NSDictionary *matching = @{
        @kIOHIDPrimaryUsagePageKey: @(kHIDPage_AppleVendor),
        @kIOHIDPrimaryUsageKey: @(kHIDUsage_AppleVendor_TemperatureSensor),
    };
    IOHIDEventSystemClientSetMatching(client, (__bridge CFDictionaryRef)matching);

	NSArray *ret = (__bridge NSArray *)IOHIDEventSystemClientCopyServices(client);
	NSMutableDictionary *dict = [NSMutableDictionary dictionary];
	for (id client in ret) {
		NSString *prod = (__bridge NSString *)IOHIDServiceClientCopyProperty((IOHIDServiceClientRef)client, CFSTR(kIOHIDProductKey));
		if (!prod)
			continue;
        IOHIDEventRef event = IOHIDServiceClientCopyEvent((IOHIDServiceClientRef)client, kIOHIDEventTypeTemperature, 0, 0);
		if (!event)
			continue;
		dict[prod] = [NSNumber numberWithDouble:IOHIDEventGetFloatValue(event, kIOHIDEventFieldTemperatureLevel)];
	}
	CFRelease(client);
	return dict;
}
