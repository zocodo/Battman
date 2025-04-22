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

- (void)applicationWillResignActive:(UIApplication *)application {
    // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
    // Use this method to pause ongoing tasks, disable timers, and invalidate graphics rendering callbacks. Games should use this method to pause the game.
}


- (void)applicationDidEnterBackground:(UIApplication *)application {
    // Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later.
    // If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
}


- (void)applicationWillEnterForeground:(UIApplication *)application {
    // Called as part of the transition from the background to the active state; here you can undo many of the changes made on entering the background.
}


- (void)applicationDidBecomeActive:(UIApplication *)application {
    // Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
}


- (void)applicationWillTerminate:(UIApplication *)application {
    // Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
}

@end
