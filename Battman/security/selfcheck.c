//
//  selfcheck.c
//  Battman
//
//  Created by Torrekie on 2025/5/20.
//

#include "selfcheck.h"

#include <mach-o/dyld.h>
#include <CoreFoundation/CoreFoundation.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <objc/runtime.h>
#include <objc/message.h>
#include <CoreGraphics/CoreGraphics.h>
#include <dispatch/dispatch.h>

typedef unsigned long NSUInteger;
typedef long NSInteger;

extern const CGFloat UIWindowLevelAlert;
extern void NSLog(CFStringRef, ...);

#define UISceneActivationStateForegroundActive 0
#define UIAlertControllerStyleAlert 1
#define UIAlertActionStyleDefault 0
static id gAlertWindow;
static __strong id _viewsQueue;

// Forward declarations
void showCompletionAlert(void);
void removeAllViews(void);
id collectAllSubviewsBottomUp(id view);

/* TODO: make all of them inline */

void showCompletionAlert(void) {
	id gwindowAlloc = ((id (*)(Class, SEL))objc_msgSend)(objc_getClass("UIWindow"), sel_registerName("alloc"));
	id gwindow = ((id (*)(id, SEL))objc_msgSend)(gwindowAlloc, sel_registerName("init"));
	
	extern void objc_storeStrong(id *, id);
	objc_storeStrong(&gAlertWindow, gwindow);

	dispatch_async(dispatch_get_main_queue(), ^{
		id window = NULL;
		Class uiAppClass = objc_getClass("UIApplication");
		SEL selSharedApp = sel_registerName("sharedApplication");
		id sharedApp = ((id (*)(Class, SEL))objc_msgSend)(uiAppClass, selSharedApp);
		
		// iOS 13+: find foreground-active UIWindowScene
		if (__builtin_available(iOS 13.0, *)) {
			SEL selConnectedScenes = sel_registerName("connectedScenes");
			id connectedScenes = ((id (*)(id, SEL))objc_msgSend)(sharedApp, selConnectedScenes);
			
			// Enumerate the scenes
			SEL selObjectEnumerator = sel_registerName("objectEnumerator");
			id enumerator = ((id (*)(id, SEL))objc_msgSend)(connectedScenes, selObjectEnumerator);
			SEL selNextObject = sel_registerName("nextObject");
			
			id scene = NULL;
			id s;
			while ((s = ((id (*)(id, SEL))objc_msgSend)(enumerator, selNextObject))) {
				// Check activationState == UISceneActivationStateForegroundActive
				NSInteger state = ((NSInteger (*)(id, SEL))objc_msgSend)(s, sel_registerName("activationState"));
				if (state != UISceneActivationStateForegroundActive)
					continue;
				
				// Check [scene isKindOfClass:UIWindowScene.class]
				Class windowSceneClass = objc_getClass("UIWindowScene");
				SEL selIsKind = sel_registerName("isKindOfClass:");
				BOOL isWindowScene = ((BOOL (*)(id, SEL, Class))objc_msgSend)(s, selIsKind, windowSceneClass);
				if (isWindowScene) {
					scene = s;
					break;
				}
			}
			
			if (scene) {
				// Allocate/init UIWindow with initWithWindowScene:
				Class windowClass = objc_getClass("UIWindow");
				SEL selAlloc = sel_registerName("alloc");
				id windowAlloc = ((id (*)(Class, SEL))objc_msgSend)(windowClass, selAlloc);
				
				SEL selInitWithScene = sel_registerName("initWithWindowScene:");
				window = ((id (*)(id, SEL, id))objc_msgSend)(windowAlloc, selInitWithScene, scene);
				
				gAlertWindow = window;
				// You can now set the window level, rootViewController, etc.
			}
		}
		
		// Fallback for <iOS13 or no scene
		if (!window) {
			Class screenClass = objc_getClass("UIScreen");
			SEL selMain = sel_registerName("mainScreen");
			id mainScreen = ((id (*)(Class, SEL))objc_msgSend)(screenClass, selMain);
			SEL selBounds = sel_registerName("bounds");
			CGRect bounds = ((CGRect (*)(id, SEL))objc_msgSend)(mainScreen, selBounds);
			
			Class windowClass = objc_getClass("UIWindow");
			SEL selAlloc = sel_registerName("alloc");
			id tmp = ((id (*)(Class, SEL))objc_msgSend)(windowClass, selAlloc);
			SEL selInitFrame = sel_registerName("initWithFrame:");
			window = ((id (*)(id, SEL, CGRect))objc_msgSend)(tmp, selInitFrame, bounds);
			gAlertWindow = window;
		}
		
		SEL selSetLevel = sel_registerName("setWindowLevel:");
		((void (*)(id, SEL, CGFloat))objc_msgSend)(gAlertWindow, selSetLevel, UIWindowLevelAlert + 1);
		
		Class vcClass = objc_getClass("UIViewController");
		SEL selNew = sel_registerName("new");
		id vc = ((id (*)(Class, SEL))objc_msgSend)(vcClass, selNew);
		
		SEL selView = sel_registerName("view");
		id viewObj = ((id (*)(id, SEL))objc_msgSend)(vc, selView);
		Class colorClass = objc_getClass("UIColor");
		SEL selClear = sel_registerName("clearColor");
		id clear = ((id (*)(Class, SEL))objc_msgSend)(colorClass, selClear);
		SEL selSetBG = sel_registerName("setBackgroundColor:");
		((void (*)(id, SEL, id))objc_msgSend)(viewObj, selSetBG, clear);
		
		SEL selSetRoot = sel_registerName("setRootViewController:");
		((void (*)(id, SEL, id))objc_msgSend)(gAlertWindow, selSetRoot, vc);
		
		SEL selMake = sel_registerName("makeKeyAndVisible");
		((void (*)(id, SEL))objc_msgSend)(gAlertWindow, selMake);
		
		Class alertClass = objc_getClass("UIAlertController");
		SEL selAlert = sel_registerName("alertControllerWithTitle:message:preferredStyle:");
		CFStringRef title = _("Sorry");
		CFStringRef msg   = _("Please download Battman from our official page.");
		id alert = ((id (*)(Class, SEL, CFStringRef, CFStringRef, NSInteger))objc_msgSend)(alertClass, selAlert, title, msg, (NSInteger)UIAlertControllerStyleAlert);
		
		Class actionClass = objc_getClass("UIAlertAction");
		SEL selAction = sel_registerName("actionWithTitle:style:handler:");
		
		typedef void (*ActionHandlerIMP)(id, SEL, id);
		struct Block_literal {
			void *isa;
			int flags;
			int reserved;
			void (*invoke)(void *, id);
			struct Block_descriptor {
				unsigned long int reserved;
				unsigned long int size;
			} *descriptor;
		};
		
		void (^handlerBlock)(id) = ^(id action) {
			SEL selHide = sel_registerName("setHidden:");
			((void (*)(id, SEL, BOOL))objc_msgSend)(gAlertWindow, selHide, YES);
			gAlertWindow = NULL;
			open_url("https://github.com/Torrekie/Battman");
		};
		
		id ok = ((id (*)(Class, SEL, CFStringRef, NSInteger, id))objc_msgSend)(actionClass, selAction, _("Open URL"), (NSInteger)UIAlertActionStyleDefault, (id)handlerBlock);

		SEL selAdd = sel_registerName("addAction:");
		((void (*)(id, SEL, id))objc_msgSend)(alert, selAdd, ok);
		
		SEL selPresent = sel_registerName("presentViewController:animated:completion:");
		((void (*)(id, SEL, id, BOOL, id))objc_msgSend)(vc, selPresent, alert, YES, NULL);
	});
}

void removeAllViews(void) {
	static bool scheduled = false;
	if (scheduled) return;
	scheduled = true;
	
	Class mutableArrayCls = objc_getClass("NSMutableArray");
	SEL selArray          = sel_registerName("array");
	id  viewsQueue        = ((id (*)(Class, SEL))objc_msgSend)(mutableArrayCls, selArray);
	
	// for (UIWindow *win in UIApplication.sharedApplication.windows) { … }
	Class uiAppCls        = objc_getClass("UIApplication");
	SEL   selSharedApp    = sel_registerName("sharedApplication");
	id    sharedApp       = ((id (*)(Class, SEL))objc_msgSend)(uiAppCls, selSharedApp);
	
	SEL selWindows        = sel_registerName("windows");
	id  windowsArray      = ((id (*)(id, SEL))objc_msgSend)(sharedApp, selWindows);

	NSUInteger arrCount   = ((NSUInteger (*)(id, SEL))objc_msgSend)(windowsArray, sel_registerName("count"));
	id  win;
	DBGLOG(CFSTR("COUNT: %u"), arrCount);
	for (NSUInteger i = 0; i < arrCount; i++) {
		// NSArray *subviews = collectAllSubviewsBottomUp_C(win);
		win = ((id (*)(id, SEL, NSUInteger))objc_msgSend)(windowsArray, sel_registerName("objectAtIndex:"), i);
		id subviews = collectAllSubviewsBottomUp(win);

		// [viewsQueue addObjectsFromArray:subviews];
		SEL selAddMany = sel_registerName("addObjectsFromArray:");
		((void (*)(id, SEL, id))objc_msgSend)(viewsQueue, selAddMany, subviews);
	}

	// Call [viewsQueue count]
	NSUInteger count = ((NSUInteger (*)(id, SEL))objc_msgSend)(viewsQueue, sel_registerName("count"));
	if (count == 0) {
		return;
	}

	//_viewsQueue = ((id (*)(Class, SEL, id))objc_msgSend)(mutableArrayCls, sel_registerName("arrayWithArray:"), viewsQueue);
	extern void objc_storeStrong(id *object, id value);
	objc_storeStrong(&_viewsQueue, viewsQueue);
	//extern id objc_retain(id);
	//_viewsQueue = objc_retain(_viewsQueue);

	NSLog(CFSTR("Will remove %zu views..."), count);
	
	// Start the GCD timer on main queue
	dispatch_source_t timer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, dispatch_get_main_queue());
	dispatch_source_set_timer(timer, dispatch_time(DISPATCH_TIME_NOW, NSEC_PER_SEC / 60), NSEC_PER_SEC / 60, 0);
	dispatch_source_set_event_handler(timer, ^{
		NSUInteger _count = ((NSUInteger (*)(id, SEL))objc_msgSend)(_viewsQueue, sel_registerName("count"));
		if (_count == 0) {
			dispatch_source_cancel(timer);
			DBGLOG(CFSTR("✅ All views removed."));
			showCompletionAlert();
			return;
		}
		id v = ((id (*)(id, SEL))objc_msgSend)(_viewsQueue, sel_registerName("firstObject"));
		((void (*)(id, SEL, NSUInteger))objc_msgSend)(_viewsQueue, sel_registerName("removeObjectAtIndex:"), 0);
		id superv = ((id (*)(id, SEL))objc_msgSend)(v, sel_registerName("superview"));
		if (superv) {
			((void (*)(id, SEL))objc_msgSend)(v, sel_registerName("removeFromSuperview"));
			DBGLOG(CFSTR("− Removed view: %@"), v);
		}
	});
	dispatch_resume(timer);
}

id collectAllSubviewsBottomUp(id view) {
	// Create a mutable array to store results
	id resultArray = ((id (*)(Class, SEL))objc_msgSend)(objc_getClass("NSMutableArray"), sel_registerName("array"));
	
	// Get the subviews array
	id subviews = ((id (*)(id, SEL))objc_msgSend)(view, sel_registerName("subviews"));
	
	// Get count of subviews
	NSUInteger count = ((NSUInteger (*)(id, SEL))objc_msgSend)(subviews, sel_registerName("count"));
	
	// Iterate through subviews
	for (NSUInteger i = 0; i < count; i++) {
		// Get each subview
		id subview = ((id (*)(id, SEL, NSUInteger))objc_msgSend)(subviews, sel_registerName("objectAtIndex:"), i);
		
		// First collect children's children
		id childResults = collectAllSubviewsBottomUp(subview);
		
		// Add all objects from child results to our result array
		((void (*)(id, SEL, id))objc_msgSend)(resultArray, sel_registerName("addObjectsFromArray:"), childResults);
		
		// Then add the subview itself
		((void (*)(id, SEL, id))objc_msgSend)(resultArray, sel_registerName("addObject:"), subview);
	}
	
	return resultArray;
}
