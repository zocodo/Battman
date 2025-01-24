//
//  ViewController.m
//  Battman
//
//  Created by Torrekie on 2025/1/18.
//

#import "ViewController.h"
#import "SceneDelegate.h"
#include "common.h"

@interface ViewController () <UINavigationBarDelegate>
@property (strong, nonatomic) UINavigationController *titleBar;

@property (strong, nonatomic) UITabBarController *bottomBarController;
@property (strong, nonatomic) UIView *batteryView;
@property (strong, nonatomic) UIViewController *batteryBarItemController;
@property (strong, nonatomic) UIViewController *moreBarItemController;
@end

@implementation ViewController

- (void)loadView {
    [super loadView];
    self.title = _("Battman");
}

- (void)viewDidLoad {
    [super viewDidLoad];
    self.view.backgroundColor = [UIColor systemBackgroundColor];

    [self setupBottomBar];
    
}

- (UIBarPosition)positionForBar:(id<UIBarPositioning>)bar
{
    return UIBarPositionTopAttached;
}

- (UIView *)setupBatteryView {
    self.batteryView = [[UIView alloc] initWithFrame:[UIScreen mainScreen].bounds];
    self.batteryView.backgroundColor = [UIColor lightGrayColor];
    self.batteryView.translatesAutoresizingMaskIntoConstraints = YES;
    self.batteryView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    
    return self.batteryView;
}

- (void)setupBottomBar {
    // Bottom Bar
    self.bottomBarController = [[UITabBarController alloc] init];

    // Bottom Bar - Battery
    self.batteryBarItemController = [[UIViewController alloc] init];
    self.batteryBarItemController.tabBarItem = [[UITabBarItem alloc] init];
    self.batteryBarItemController.tabBarItem.title = _("Battery");
    self.batteryBarItemController.tabBarItem.image = [UIImage systemImageNamed:@"battery.100"];
    self.batteryBarItemController.tabBarItem.tag = 0;
    [self.batteryBarItemController.view addSubview:[self setupBatteryView]];

    // Bottom Bar - More
    self.moreBarItemController = [[UIViewController alloc] init];
    self.moreBarItemController.tabBarItem = [[UITabBarItem alloc] initWithTabBarSystemItem:UITabBarSystemItemMore tag:1];
    self.moreBarItemController.tabBarItem.title = _("More");


    // Bottom Bar Summary
    self.bottomBarController.viewControllers = @[self.batteryBarItemController, self.moreBarItemController];

    [self.bottomBarController willMoveToParentViewController:self];
    [self.view addSubview:self.bottomBarController.view];
    [self addChildViewController:self.bottomBarController];
    [self.bottomBarController didMoveToParentViewController:self];
}

- (void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];
}

@end
