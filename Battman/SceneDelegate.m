//
//  SceneDelegate.m
//  Battman
//
//  Created by Torrekie on 2025/1/18.
//

#import "SceneDelegate.h"
#import "BatteryInfoViewController.h"
#import "SettingsViewController.h"

@interface SceneDelegate ()

@end

UIWindow *gWindow;

@implementation SceneDelegate


- (void)scene:(UIScene *)scene willConnectToSession:(UISceneSession *)session options:(UISceneConnectionOptions *)connectionOptions {
	self.window = [[UIWindow alloc] initWithWindowScene:(UIWindowScene *)scene];
	UITabBarController *tabbar = [UITabBarController new];
	tabbar.viewControllers = @[
		[[UINavigationController alloc] initWithRootViewController:[BatteryInfoViewController new]],
		[[UINavigationController alloc] initWithRootViewController:[SettingsViewController new]]
	];
	gWindow = self.window;
	gWindow.rootViewController = tabbar;

	[self.window makeKeyAndVisible];
	gWindow = self.window;

    extern UIViewController* find_top_controller(UIViewController *root);
    dispatch_async(dispatch_get_main_queue(), ^{
        UIWindowScene *scene = (UIWindowScene *)[[[UIApplication sharedApplication] connectedScenes] anyObject];
        UIWindow *keyWindow = scene.windows.firstObject;
        
        UIViewController *topController = find_top_controller(keyWindow.rootViewController);
        NSLog(@"%@", topController);
    });
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
}


@end
