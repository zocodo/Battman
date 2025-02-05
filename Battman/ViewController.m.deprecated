//
//  ViewController.m
//  Battman
//
//  Created by Torrekie on 2025/1/18.
//

#import "ViewController.h"
#import "SceneDelegate.h"
#import "SPWaterProgressIndicatorView.h"
#include "common.h"

@interface ViewController () <UINavigationBarDelegate, UICollectionViewDataSource, UICollectionViewDelegate>
@property (strong, nonatomic) UINavigationController *titleBar;

@property (strong, nonatomic) UITabBarController *bottomBarController;
@property (strong, nonatomic) UIView *batteryView;
@property (strong, nonatomic) UIViewController *batteryBarItemController;
@property (strong, nonatomic) UIViewController *moreBarItemController;
@property (strong, nonatomic) UICollectionView *batteryCollection;

// Scroller
@property (strong, nonatomic) UIScrollView *batteryScrollView;
@property (strong, nonatomic) UIView *containerView;
@property (strong, nonatomic) UICollectionViewFlowLayout *batteryCollectionLayout;

// Battery Cell
@property (strong, nonatomic) UIView *batteryCell;
@property (strong, nonatomic) SPWaterProgressIndicatorView *waterViewSoC;
@property (strong, nonatomic) CAGradientLayer *waterViewSoCGradient;
@property (strong, nonatomic) SPWaterProgressIndicatorView *waterViewTR;
@property (strong, nonatomic) CAGradientLayer *waterViewTRGradient;
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
    // Scroller
    UIScrollView *batteryScrollView = [[UIScrollView alloc] initWithFrame:[UIScreen mainScreen].bounds];
    self.batteryScrollView = batteryScrollView;
    batteryScrollView.translatesAutoresizingMaskIntoConstraints = YES;
    batteryScrollView.showsVerticalScrollIndicator = YES;
    batteryScrollView.scrollEnabled = YES;
    batteryScrollView.bounces = YES;
    batteryScrollView.alwaysBounceVertical = YES;
    batteryScrollView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;

    // Scroller Container
    UIView *containerView = [[UIView alloc] init];
    self.containerView = containerView;
    containerView.translatesAutoresizingMaskIntoConstraints = NO;
    [batteryScrollView addSubview:containerView];
    [containerView.topAnchor constraintEqualToAnchor:batteryScrollView.topAnchor].active = YES;
    [containerView.leadingAnchor constraintEqualToAnchor:batteryScrollView.leadingAnchor].active = YES;
    [containerView.trailingAnchor constraintEqualToAnchor:batteryScrollView.trailingAnchor].active = YES;

    self.batteryCollectionLayout = [[UICollectionViewFlowLayout alloc] init];
    UICollectionViewFlowLayout *batteryCollectionLayout = self.batteryCollectionLayout;
    batteryCollectionLayout.itemSize = CGSizeMake(self.view.frame.size.width / 2 - 30, self.view.frame.size.width / 2 - 30);
    batteryCollectionLayout.minimumInteritemSpacing = 15;
    batteryCollectionLayout.minimumLineSpacing = 15;
    batteryCollectionLayout.sectionInset = UIEdgeInsetsZero;
    
    self.batteryCollection = [[UICollectionView alloc] initWithFrame:CGRectMake(20, 20, self.view.frame.size.width - 40, self.view.frame.size.width - 40) collectionViewLayout:batteryCollectionLayout];
    //batteryCollection.layer.cornerRadius = 30;
    self.batteryCollection.layer.masksToBounds = YES;
    self.batteryCollection.backgroundColor = [UIColor clearColor];
    [self.batteryCollection registerClass:[UICollectionViewCell class] forCellWithReuseIdentifier:@"Cell"];
    self.batteryCollection.dataSource = self;
    self.batteryCollection.delegate = self;
    [containerView addSubview:self.batteryCollection];
    
    // Add a rounded rectangle (UIView with rounded corners)
    CGFloat batteryCellWidth = (self.view.frame.size.width) / 2 - 30;
    self.batteryCell = [[UIView alloc] initWithFrame:CGRectMake(0, 0, batteryCellWidth, batteryCellWidth)];
    self.batteryCell.layer.cornerRadius = 30;
    self.batteryCell.layer.masksToBounds = YES;
    self.batteryCell.backgroundColor = [UIColor secondarySystemFillColor];
#pragma Battery Animation -- Start
    /* FullChargeCapacity/DesignCapacity */
    {
        /* True Remaining */
        self.waterViewTR = [[SPWaterProgressIndicatorView alloc] initWithFrame:self.batteryCell.bounds];
        self.waterViewTR.center = self.batteryCell.center;

        // Create the background layer that will hold the gradient
        self.waterViewTRGradient = [CAGradientLayer layer];
        CAGradientLayer *waterViewTRGradient = self.waterViewTRGradient;
        waterViewTRGradient.frame = self.batteryCell.frame;
    
        // Create an array of CG colors from our UIColor array
        NSMutableArray *cgColors = [NSMutableArray array];
        for (UIColor *color in @[[UIColor whiteColor], [UIColor whiteColor], [UIColor lightGrayColor]]) {
            [cgColors addObject:(__bridge id)color.CGColor];
        }
        waterViewTRGradient.colors = cgColors;

        // Create an image context to render the gradient
        UIGraphicsBeginImageContext(waterViewTRGradient.bounds.size);
        [waterViewTRGradient renderInContext:UIGraphicsGetCurrentContext()];
        UIImage *backgroundColorImage = UIGraphicsGetImageFromCurrentImageContext();
        UIGraphicsEndImageContext();
        self.waterViewTR.waveColor = [UIColor colorWithPatternImage:backgroundColorImage];
        self.waterViewTR.frequency = 0.5;
        self.waterViewTR.amplitude = 0.2;
        self.waterViewTR.phaseShift = 0.05;
        [self.batteryCell addSubview:self.waterViewTR];
        self.waterViewTR.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
        self.waterViewTR.transform = CGAffineTransformMakeRotation(M_PI);
#warning TODO: FullChargeCapacity/DesignCapacity (B0FC/B0DC)
        [self.waterViewTR updateWithPercentCompletion:10];
    
        [self.waterViewTR startAnimation];
    }
    /* StateOfCharge */
    {
        self.waterViewSoC = [[SPWaterProgressIndicatorView alloc] initWithFrame:self.batteryCell.bounds];
        self.waterViewSoC.center = self.batteryCell.center;

        // Create the background layer that will hold the gradient
        self.waterViewSoCGradient = [CAGradientLayer layer];
        CAGradientLayer *waterViewSoCGradient = self.waterViewSoCGradient;
        waterViewSoCGradient.frame = self.batteryCell.frame;
    
        // Create an array of CG colors from our UIColor array
        NSMutableArray *cgColors = [NSMutableArray array];
        for (UIColor *color in @[[UIColor cyanColor], [UIColor greenColor]]) {
            [cgColors addObject:(__bridge id)color.CGColor];
        }
        waterViewSoCGradient.colors = cgColors;

        // Create an image context to render the gradient
        UIGraphicsBeginImageContext(waterViewSoCGradient.bounds.size);
        [waterViewSoCGradient renderInContext:UIGraphicsGetCurrentContext()];
        UIImage *backgroundColorImage = UIGraphicsGetImageFromCurrentImageContext();
        UIGraphicsEndImageContext();
        self.waterViewSoC.waveColor = [UIColor colorWithPatternImage:backgroundColorImage];
        self.waterViewSoC.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
        self.waterViewSoC.phaseShift = 0.1;
        [self.batteryCell addSubview:self.waterViewSoC];
#warning TODO: StateOfCharge (BRSC)
        [self.waterViewSoC updateWithPercentCompletion:50];
    
        [self.waterViewSoC startAnimation];
    }
#pragma Battery Animation -- End
    
    UILabel *copyrightLabel = [[UILabel alloc] initWithFrame:CGRectMake(20, CGRectGetMaxY(self.batteryCollection.frame) + 20, self.view.frame.size.width - 40, 40)];
    copyrightLabel.text = _("2025 Ⓒ Torrekie <me@torrekie.dev>");
    copyrightLabel.font = [UIFont preferredFontForTextStyle:UIFontTextStyleFootnote];
    copyrightLabel.textColor = [UIColor systemGray2Color];
    copyrightLabel.textAlignment = NSTextAlignmentCenter;
    [containerView addSubview:copyrightLabel];
    
    // Add bottom padding to the container view
    UIView *bottomPaddingView = [[UIView alloc] initWithFrame:CGRectMake(0, CGRectGetMaxY(copyrightLabel.frame) + 20, self.view.frame.size.width, 20)];
    [containerView addSubview:bottomPaddingView];
    
    // Add constraints to adjust containerView's bottom based on bottomPaddingView
    [containerView.bottomAnchor constraintEqualToAnchor:bottomPaddingView.bottomAnchor].active = YES;
    
    // Set content size of the scroll view after adding all subviews
//    batteryScrollView.contentSize = containerView.bounds.size;
    CGRect contentRect = CGRectZero;
    for (UIView *view in containerView.subviews) {
        contentRect = CGRectUnion(contentRect, view.frame);
    }
    batteryScrollView.contentSize = contentRect.size;

    return batteryScrollView;
}

// Override to handle rotation
- (void)viewWillTransitionToSize:(CGSize)size withTransitionCoordinator:(id<UIViewControllerTransitionCoordinator>)coordinator {
    [super viewWillTransitionToSize:size withTransitionCoordinator:coordinator];
    
    // Adjust layout when the screen rotates
    self.batteryCollection.frame = CGRectMake(20, 20, size.width - 40, size.width - 40);

    CGFloat itemWidth = (size.width / 2) - 30;
    self.batteryCollectionLayout.itemSize = CGSizeMake(itemWidth, itemWidth);
    self.batteryCell.frame = CGRectMake(0, 0, itemWidth, itemWidth);

    self.waterViewSoCGradient.frame = self.batteryCell.frame;
    self.waterViewSoC.frame = self.batteryCell.frame;
    self.waterViewSoC.center = self.batteryCell.center;
    UIGraphicsBeginImageContext(self.waterViewSoCGradient.bounds.size);
    [self.waterViewSoCGradient renderInContext:UIGraphicsGetCurrentContext()];
    UIImage *backgroundColorImage = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    self.waterViewSoC.waveColor = [UIColor colorWithPatternImage:backgroundColorImage];

    self.waterViewTRGradient.frame = self.batteryCell.frame;
    self.waterViewTR.frame = self.batteryCell.frame;
    self.waterViewTR.center = self.batteryCell.center;
    UIGraphicsBeginImageContext(self.waterViewTRGradient.bounds.size);
    [self.waterViewTRGradient renderInContext:UIGraphicsGetCurrentContext()];
    backgroundColorImage = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    self.waterViewTR.waveColor = [UIColor colorWithPatternImage:backgroundColorImage];
    
    // Animate the transition if needed
    [coordinator animateAlongsideTransition:^(id<UIViewControllerTransitionCoordinatorContext>  _Nonnull context) {
        [self.batteryCollection.collectionViewLayout invalidateLayout]; // Invalidate the layout to trigger a refresh
    } completion:nil];
    
    CGRect contentRect = CGRectZero;
    for (UIView *view in self.containerView.subviews) {
        contentRect = CGRectUnion(contentRect, view.frame);
    }
    self.batteryScrollView.contentSize = contentRect.size;
}

#pragma mark - UICollectionView DataSource
// Number of sections
- (NSInteger)numberOfSectionsInCollectionView:(UICollectionView *)collectionView {
    return 1;
}

// Number of items in the section (4 items for 2x2 grid)
- (NSInteger)collectionView:(UICollectionView *)collectionView numberOfItemsInSection:(NSInteger)section {
    return 4; // TODO: Globalize
}

// Create and configure the cell
- (UICollectionViewCell *)collectionView:(UICollectionView *)collectionView cellForItemAtIndexPath:(NSIndexPath *)indexPath {
    UICollectionViewCell *cell = [collectionView dequeueReusableCellWithReuseIdentifier:@"Cell" forIndexPath:indexPath];
    
    // Customize the cell appearance (e.g., background color)
    cell.backgroundColor = [UIColor clearColor];
    
    // Add a label to the cell (optional)
    UILabel *label = [[UILabel alloc] initWithFrame:cell.contentView.bounds];
#warning TODO
    NSArray *stubdata = @[_("SoC"), _("Health"), _("Temperature"), _("Remain Time")];
    label.text = stubdata[indexPath.item];
    label.textAlignment = NSTextAlignmentCenter;
    label.font = [UIFont boldSystemFontOfSize:24];

    // self.batteryCell
    if (indexPath.item == 0) {
        [self.batteryCell addSubview:label];
        [cell.contentView addSubview:self.batteryCell];
        return cell;
    }
    
    [cell.contentView addSubview:label];
    
    return cell;
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
    //[self.moreBarItemController.tabBarItem.setValue(_("More"), forKey: "internalTitle")];


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
