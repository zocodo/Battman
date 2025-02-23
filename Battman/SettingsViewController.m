#import "SettingsViewController.h"
#include "common.h"

static NSMutableArray *sections_settings;

extern NSMutableAttributedString *redirectedOutput;

#ifdef DEBUG
@interface DebugViewController : UIViewController
@property (nonatomic, readwrite, strong) UITextView *textField;
@end
@implementation DebugViewController

- (NSString *)title {
    return @"Logs";
}

- (void)DebugExportPressed {
    NSString *str = [redirectedOutput string];
    UIActivityViewController *activityViewController = [[UIActivityViewController alloc] initWithActivityItems:@[str] applicationActivities:nil];
    [self.navigationController presentViewController:activityViewController
                                      animated:YES
                                    completion:^{}];
}

- (void)viewDidLoad {
    [super viewDidLoad];

    self.view.backgroundColor = [UIColor whiteColor];
    self.textField = [[UITextView alloc] initWithFrame:self.view.bounds];
    self.textField.font = [UIFont fontWithName:@"Courier" size:10];
    
    self.textField.text = [redirectedOutput string];

    [self.view addSubview:self.textField];
    
    self.navigationItem.rightBarButtonItem = [[UIBarButtonItem alloc] init];
    UIImage *export_img = [UIImage systemImageNamed:@"square.and.arrow.up"];
    UIButton *export_button = [[UIButton alloc] initWithFrame:CGRectMake(0, 0, export_img.size.width, export_img.size.height)];
    [export_button setBackgroundImage:export_img forState:UIControlStateNormal];
    [export_button addTarget:self action:@selector(DebugExportPressed)
      forControlEvents:UIControlEventTouchUpInside];
    [export_button setShowsTouchWhenHighlighted:YES];

    UIBarButtonItem *barButton = [[UIBarButtonItem alloc] initWithCustomView:export_button];
    self.navigationItem.rightBarButtonItem = barButton;
}

@end
#endif

@implementation SettingsViewController

- (NSString *)title {
	return _("More");
}

- (instancetype)init {
	UITabBarItem *tabbarItem = [[UITabBarItem alloc] initWithTabBarSystemItem:UITabBarSystemItemMore tag:1];
	tabbarItem.title = _("More"); // UITabBarSystemItem cannot change title like this
    [tabbarItem setValue:_("More") forKey:@"internalTitle"]; // This is the correct way (But not accepted by App Store)
	self.tabBarItem = tabbarItem;

    sections_settings = [[NSMutableArray alloc] initWithArray:@[_("About Battman")]];
#if DEBUG
    [sections_settings addObject:_("Debug")];
#endif
	return [super initWithStyle:UITableViewStyleGrouped]; // or plain if desired
}

- (NSInteger)tableView:(id)tv numberOfRowsInSection:(NSInteger)section {
    if (section == [sections_settings indexOfObject:_("About Battman")]) {
        return 2;
    }
#ifdef DEBUG
    if (section == [sections_settings indexOfObject:_("Debug")]) {
        return 1;
    }
#endif
    return 0;
}

- (NSInteger)numberOfSectionsInTableView:(id)tv {
    return sections_settings.count;
}

- (NSString *)tableView:(UITableView *)tv titleForHeaderInSection:(NSInteger)sect {
    return sections_settings[sect];
}

- (void)tableView:(UITableView *)tv didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
    if (indexPath.section == [sections_settings indexOfObject:_("About Battman")]) {
        if (indexPath.row == 0) {
            [self.navigationController pushViewController:[CreditViewController new] animated:YES];
        } else if (indexPath.row == 1) {
            [[UIApplication sharedApplication] openURL:[NSURL URLWithString:@"https://github.com/Torrekie/Battman"] options:[NSDictionary new] completionHandler:nil];
        }
    }
#ifdef DEBUG
    if (indexPath.section == [sections_settings indexOfObject:_("Debug")])
        [self.navigationController pushViewController:[[DebugViewController alloc] init] animated:YES];
#endif

    [tv deselectRowAtIndexPath:indexPath animated:YES];
}

- (UITableViewCell *)tableView:(id)tv cellForRowAtIndexPath:(NSIndexPath *)indexPath {
	// TODO: REUSE (Too few cells to reuse for now so no need at this moment)
    if (indexPath.section == [sections_settings indexOfObject:_("About Battman")]) {
        if (indexPath.row == 0) {
            UITableViewCell *creditCell = [UITableViewCell new];
            creditCell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
            creditCell.textLabel.text = _("Credit");
            return creditCell;
        } else if (indexPath.row == 1) {
            UITableViewCell *sourceCodeCell = [UITableViewCell new];
            sourceCodeCell.textLabel.text = _("Source Code");
            sourceCodeCell.textLabel.textColor = [UIColor colorWithRed:0 green:0.478 blue:1 alpha:1];
            return sourceCodeCell;
        }
    }
#ifdef DEBUG
    if (indexPath.section == [sections_settings indexOfObject:_("Debug")]) {
        UITableViewCell *cell = [UITableViewCell new];
        cell.textLabel.text = _("Logs (stdio)");
        cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
        return cell;
    }
#endif
	UITableViewCell *batteryChargeCell = [UITableViewCell new];
	batteryChargeCell.textLabel.text = @"Test222";
	return batteryChargeCell;
}

@end

@implementation CreditViewController

- (NSString *)title {
	return _("Credit");
}

- (instancetype)init {
	return [super initWithStyle:UITableViewStyleGrouped];
}

- (NSInteger)tableView:(id)tv numberOfRowsInSection:(NSInteger)section {
	return 2;
}

- (NSInteger)numberOfSectionsInTableView:(id)tv {
	return 1;
}

- (NSString *)tableView:(UITableView *)tv titleForHeaderInSection:(NSInteger)sect {
	return _("Battman Credit");
}

- (void)tableView:(UITableView *)tv didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
	//if(indexPath.row==1) {
	//	[[UIApplication sharedApplication] openURL:@"https://github.com/Torrekie/Battman" options:nil completionHandler:nil];
	//}
	[tv deselectRowAtIndexPath:indexPath animated:YES];
}

- (UITableViewCell *)tableView:(id)tv cellForRowAtIndexPath:(NSIndexPath *)indexPath {
	// TODO: REUSE (Too few cells to reuse for now so no need at this moment)
    // Consider also localize those names?
	if (indexPath.row == 0) {
		UITableViewCell *aCell = [UITableViewCell new];
		aCell.textLabel.text = @"Torrekie";
		return aCell;
	} else if (indexPath.row == 1) {
		UITableViewCell *bCell = [UITableViewCell new];
		bCell.textLabel.text = @"Ruphane";
		return bCell;
	}
	return nil;
}

+ (NSArray *)debugGetBatteryCausesLeakDoNotUseInProduction {
	void *IOPSCopyPowerSourcesByType(int);
	return (__bridge NSArray *)IOPSCopyPowerSourcesByType(1);
}

@end
