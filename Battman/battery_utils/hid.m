#import <Foundation/Foundation.h>

extern CFTypeRef IOHIDEventSystemClientCreate(int);
extern void IOHIDEventSystemClientSetMatching(CFTypeRef,NSDictionary*);
extern NSArray *IOHIDEventSystemClientCopyServices(CFTypeRef);
extern id IOHIDServiceClientCopyProperty(id,NSString*);
extern id IOHIDServiceClientCopyEvent(id,int);
extern double IOHIDEventGetFloatValue(id,int);

NSDictionary *getTemperatureHIDData() {
	CFTypeRef client=IOHIDEventSystemClientCreate(0);
	if(!client)
		return nil;
	IOHIDEventSystemClientSetMatching(client,@{
		@"PrimaryUsagePage":@0xff00,
		@"PrimaryUsage":@5
	});
	NSArray *ret=IOHIDEventSystemClientCopyServices(client);
	NSMutableDictionary *dict=[NSMutableDictionary dictionary];
	for(id client in ret) {
		NSString *prod=IOHIDServiceClientCopyProperty(client,@"Product");
		if(!prod)
			continue;
		id event=IOHIDServiceClientCopyEvent(client,15);
		if(!event)
			continue;
		dict[prod]=[NSNumber numberWithDouble:IOHIDEventGetFloatValue(event,15<<16)];
	}
	CFRelease(client);
	return dict;
}