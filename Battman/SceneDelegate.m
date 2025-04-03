//
//  SceneDelegate.m
//  Battman
//
//  Created by Torrekie on 2025/1/18.
//

#import "SceneDelegate.h"
#import "BatteryInfoViewController.h"
#import "SettingsViewController.h"
#import "LicenseViewController.h"

@interface SceneDelegate ()

@end

@interface UIApplication ()
- (void)terminateWithSuccess;
@end

UIWindow *gWindow;
BOOL graceful;

@implementation SceneDelegate

// FIXME: UIScene is not for iOS 9 or earlier
- (void)scene:(UIScene *)scene willConnectToSession:(UISceneSession *)session options:(UISceneConnectionOptions *)connectionOptions {
	self.window = [[UIWindow alloc] initWithWindowScene:(UIWindowScene *)scene];
	UITabBarController *tabbar = [UITabBarController new];
	tabbar.viewControllers = @[
		[[UINavigationController alloc] initWithRootViewController:[BatteryInfoViewController new]],
		[[UINavigationController alloc] initWithRootViewController:[SettingsViewController new]]
	];
	gWindow = self.window;

    // Only do this on Embedded, since apps are containered with UUID
    // Directly reading device ID are considered immoral but able to be used both macOS and iOS
    // We do not accept this anyway, let macOS license be at the pkg installer (if we are going to have a macOS build)
#if TARGET_OS_IPHONE
    // Don't worry, we only get this ID to make sure device consistency
    NSString *containerID = [[UIDevice currentDevice] identifierForVendor].UUIDString;
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    NSString *cachedID = [defaults objectForKey:@"ContainerID"];
    
    NSLog(@"ID: %@, Cached: %@", containerID, cachedID);
    // If no device ID is stored, or if the current device ID doesn't match the cached one
    if (cachedID == nil || ![containerID isEqualToString:cachedID]) {
        LicenseViewController *vc = [[LicenseViewController alloc] init];
        UINavigationController *licenseView = [[UINavigationController alloc] initWithRootViewController:vc];
        gWindow.rootViewController = licenseView;
    } else
#endif
	gWindow.rootViewController = tabbar;

	[self.window makeKeyAndVisible];
	gWindow = self.window;
}


- (void)sceneDidDisconnect:(UIScene *)scene {
    // Called as the scene is being released by the system.
    // This occurs shortly after the scene enters the background, or when its session is discarded.
    // Release any resources associated with this scene that can be re-created the next time the scene connects.
    // The scene may re-connect later, as its session was not necessarily discarded (see `application:didDiscardSceneSessions` instead).
}


- (void)sceneDidBecomeActive:(UIScene *)scene {
    // Called when the scene has moved from an inactive state to an active state.
    // Use this method to restart any tasks that were paused (or not yet started) when the scene was inactive.
}


- (void)sceneWillResignActive:(UIScene *)scene {
    // Called when the scene will move from an active state to an inactive state.
    // This may occur due to temporary interruptions (ex. an incoming phone call).
}


- (void)sceneWillEnterForeground:(UIScene *)scene {
    // Called as the scene transitions from the background to the foreground.
    // Use this method to undo the changes made on entering the background.
}


- (void)sceneDidEnterBackground:(UIScene *)scene {
    // Called as the scene transitions from the foreground to the background.
    // Use this method to save data, release shared resources, and store enough scene-specific state information
    // to restore the scene back to its current state.
    if (graceful == YES) {
        extern UIApplication *UIApp;
        [UIApp terminateWithSuccess];
    }
}


@end
