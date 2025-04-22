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
    UIButton *export_button;
    if (@available(iOS 13.0, *)) {
        UIImage *export_img = [UIImage systemImageNamed:@"square.and.arrow.up"];
        export_button = [[UIButton alloc] initWithFrame:CGRectMake(0, 0, export_img.size.width, export_img.size.height)];
        [export_button setBackgroundImage:export_img forState:UIControlStateNormal];
    } else {
        export_button = [UIButton buttonWithType:UIButtonTypeSystem];
        [export_button.titleLabel setFont:[UIFont fontWithName:@"SFProDisplay-Regular" size:22]];
        // U+100202
        [export_button setTitle:@"􀈂" forState:UIControlStateNormal];
        [export_button setFrame:CGRectZero];
    }
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
            open_url("https://github.com/Torrekie/Battman");
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
            if (@available(iOS 13.0, *)) {
                sourceCodeCell.textLabel.textColor = [UIColor linkColor];
            } else {
                sourceCodeCell.textLabel.textColor = [UIColor colorWithRed:0 green:(122.0f / 255) blue:1 alpha:1];
            }
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

NSString *_contrib[] = {
	@"Torrekie", @"https://github.com/Torrekie",
	@"Ruphane", @"https://github.com/LNSSPsd",
};

@implementation CreditViewController

- (NSString *)title {
	return _("Credit");
}

- (instancetype)init {
	return [super initWithStyle:UITableViewStyleGrouped];
}

- (NSInteger)tableView:(id)tv numberOfRowsInSection:(NSInteger)section {
	return sizeof(_contrib) / (2 * sizeof(NSString *));
}

- (NSInteger)numberOfSectionsInTableView:(id)tv {
	return 1;
}

- (NSString *)tableView:(UITableView *)tv titleForHeaderInSection:(NSInteger)sect {
	return _("Battman Credit");
}

- (void)tableView:(UITableView *)tv didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
	[[UIApplication sharedApplication] openURL:[NSURL URLWithString:_contrib[indexPath.row*2+1]] options:@{} completionHandler:nil];
	[tv deselectRowAtIndexPath:indexPath animated:YES];
}

- (UITableViewCell *)tableView:(id)tv cellForRowAtIndexPath:(NSIndexPath *)indexPath {
    UITableViewCell *cell = [UITableViewCell new];
    cell.textLabel.text = _contrib[indexPath.row * 2];
    if (@available(iOS 13.0, *)) {
        cell.textLabel.textColor = [UIColor linkColor];
    } else {
        cell.textLabel.textColor = [UIColor colorWithRed:0 green:(122.0f / 255) blue:1 alpha:1];
    }

    return cell;
}

+ (NSString *)getTHEPATH {
	extern char *THEPATH;
	return [NSString stringWithUTF8String:THEPATH];
}

+ (NSNumber *)getTHENUM {
	extern NSInteger THENUM;
	return [NSNumber numberWithInteger:THENUM];
}

+ (NSArray *)debugGetBatteryCausesLeakDoNotUseInProduction {
	void *IOPSCopyPowerSourcesByType(int);
	return (__bridge NSArray *)IOPSCopyPowerSourcesByType(1);
}

@end
