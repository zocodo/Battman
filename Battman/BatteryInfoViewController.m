#import "BatteryInfoViewController.h"
#import "SPWaterProgressIndicatorView.h"

@implementation BatteryInfoViewController

- (NSString *)title {
	return @"Battman";
}

- (instancetype)init {
	UITabBarItem *tabbarItem=[UITabBarItem new];
	tabbarItem.title=@"Battery";
	tabbarItem.image=[UIImage systemImageNamed:@"battery.100"];
	tabbarItem.tag = 0;
	self.tabBarItem=tabbarItem;
	return [super initWithStyle:UITableViewStylePlain]; // or grouped if desired
}

- (NSInteger)tableView:(id)tv numberOfRowsInSection:(NSInteger)section {
	return 1;
}

- (NSInteger)numberOfSectionsInTableView:(id)tv {
	return 1;
}

- (void)tableView:(UITableView *)tv didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
	[tv deselectRowAtIndexPath:indexPath animated:YES];
}

- (UITableViewCell *)tableView:(id)tv cellForRowAtIndexPath:(NSIndexPath *)indexPath {
	UITableViewCell *batteryChargeCell=[[UITableViewCell alloc] initWithFrame:CGRectMake(0,0,1000,100)];
	UIView *batteryCell = [[UIView alloc] initWithFrame:CGRectMake(0, 0, 80, 80)];
	batteryCell.layer.cornerRadius = 30;
	batteryCell.layer.masksToBounds = YES;
	batteryCell.backgroundColor = [UIColor secondarySystemFillColor];
#pragma Battery Animation -- Start
	/* FullChargeCapacity/DesignCapacity */
	{
		/* True Remaining */
		SPWaterProgressIndicatorView *waterViewTR = [[SPWaterProgressIndicatorView alloc] initWithFrame:batteryCell.bounds];
		waterViewTR.center=batteryCell.center;
		// Create the background layer that will hold the gradient
		CAGradientLayer *waterViewTRGradient = [CAGradientLayer layer];
		waterViewTRGradient.frame = batteryCell.frame;
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
		waterViewTR.waveColor = [UIColor colorWithPatternImage:backgroundColorImage];
		waterViewTR.frequency = 0.5;
		waterViewTR.amplitude = 0.2;
		waterViewTR.phaseShift = 0.05;
		//[batteryCell addSubview:waterViewTR];
		waterViewTR.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
		waterViewTR.transform = CGAffineTransformMakeRotation(M_PI);
#warning TODO: FullChargeCapacity/DesignCapacity (B0FC/B0DC)
		[waterViewTR updateWithPercentCompletion:50];
		[waterViewTR startAnimation];
	}
	{
        SPWaterProgressIndicatorView *waterViewSoC = [[SPWaterProgressIndicatorView alloc] initWithFrame:batteryCell.bounds];
        waterViewSoC.center = batteryCell.center;

        // Create the background layer that will hold the gradient
        CAGradientLayer *waterViewSoCGradient = [CAGradientLayer layer];
        waterViewSoCGradient.frame = batteryCell.frame;
    
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
        waterViewSoC.waveColor = [UIColor colorWithPatternImage:backgroundColorImage];
        waterViewSoC.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
        waterViewSoC.phaseShift = 0.1;
        [batteryCell addSubview:waterViewSoC];
#warning TODO: StateOfCharge (BRSC)
        [waterViewSoC updateWithPercentCompletion:50];
    
        [waterViewSoC startAnimation];
    }

	UIView *positionView=[[UIView alloc] initWithFrame:CGRectMake(20,20,80,80)];
	[positionView addSubview:batteryCell];
	[batteryChargeCell.contentView addSubview:positionView];
	UILabel *batteryRemainingLabel=[[UILabel alloc] initWithFrame:CGRectMake(120,10,600,100)];
	batteryRemainingLabel.lineBreakMode=NSLineBreakByWordWrapping;
	batteryRemainingLabel.numberOfLines=0;
	batteryRemainingLabel.text=@"Battery Capacity: 90%\nCharge: 50%\nHotness: 0%";
	[batteryChargeCell.contentView addSubview:batteryRemainingLabel];
	//batteryChargeCell.textLabel.text=@"Test";
	return batteryChargeCell;
}

- (CGFloat)tableView:(id)tv heightForRowAtIndexPath:(NSIndexPath *)indexPath {
	if(indexPath.row==0) {
		return 130;
	}
	return 30;
}

@end