//
//  AppDelegate.m
//  Battman
//
//  Created by Torrekie on 2025/1/18.
//

#import "AppDelegate.h"

@interface AppDelegate ()

@end
@interface AppDelegate12 ()

@end

@implementation AppDelegate


- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    // Override point for customization after application launch.
    return YES;
}


#pragma mark - UISceneSession lifecycle

// Don't warn about this, even we are targeting pre-iOS 13, See AppDelegate12
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunguarded-availability-new"

- (UISceneConfiguration *)application:(UIApplication *)application configurationForConnectingSceneSession:(UISceneSession *)connectingSceneSession options:(UISceneConnectionOptions *)options {
    // Called when a new scene session is being created.
    // Use this method to select a configuration to create the new scene with.
    return [[UISceneConfiguration alloc] initWithName:@"Default Configuration" sessionRole:connectingSceneSession.role];
}


- (void)application:(UIApplication *)application didDiscardSceneSessions:(NSSet<UISceneSession *> *)sceneSessions {
    // Called when the user discards a scene session.
    // If any sessions were discarded while the application was not running, this will be called shortly after application:didFinishLaunchingWithOptions.
    // Use this method to release any resources that were specific to the discarded scenes, as they will not return.
}

#pragma clang diagnostic pop
@end

extern UIWindow *gWindow;
extern BOOL graceful;
#import "license_check.h"
#import "LicenseViewController.h"
#import "BatteryInfoViewController.h"
#import "SettingsViewController.h"

@implementation AppDelegate12

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    self.window = [[UIWindow alloc] initWithFrame:[UIScreen mainScreen].bounds];
    UITabBarController *tabbar = [UITabBarController new];
    tabbar.viewControllers = @[
        [[UINavigationController alloc] initWithRootViewController:[BatteryInfoViewController new]],
        [[UINavigationController alloc] initWithRootViewController:[SettingsViewController new]]
    ];
    gWindow = self.window;

#if TARGET_OS_IPHONE
    if (!has_accepted_terms()) {
        LicenseViewController *vc = [[LicenseViewController alloc] init];
        UINavigationController *licenseView = [[UINavigationController alloc] initWithRootViewController:vc];
        gWindow.rootViewController = licenseView;
    } else
#endif
    gWindow.rootViewController = tabbar;

    [self.window makeKeyAndVisible];
    gWindow = self.window;

    /* TODO: Reduce redundant codes */
    return YES;
}

@end
