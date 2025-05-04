#import "SettingsViewController.h"
#include "common.h"
#include <math.h>

@interface LanguageSelectionVC:UITableViewController
@end

enum sections_settings {
	SS_SECT_ABOUT,
#ifdef DEBUG
	SS_SECT_DEBUG,
#endif
	SS_SECT_COUNT
};

extern NSMutableAttributedString *redirectedOutput;
extern void (^redirectedOutputListener)(void);

static BOOL _coolDebugVCPresented=0;

@interface DebugViewController : UIViewController
@property (nonatomic, readwrite, strong) UITextView *textField;
@end
@implementation DebugViewController

- (NSString *)title {
    return _("Logs");
}

- (void)DebugExportPressed {
    NSString *str = [redirectedOutput string];
    UIActivityViewController *activityViewController = [[UIActivityViewController alloc] initWithActivityItems:@[str] applicationActivities:nil];
    [self.navigationController presentViewController:activityViewController animated:YES completion:^{}];
}

- (void)closeCoolDebug {
	if(!_coolDebugVCPresented)
		return;
	_coolDebugVCPresented=0;
	CGRect myFrame=self.navigationController.view.frame;
	[self.navigationController.view removeFromSuperview];
	self.navigationController.parentViewController.view.frame=CGRectMake(0,0,myFrame.size.width,myFrame.size.height*3);
	[self.navigationController removeFromParentViewController];
}

- (void)viewDidUnload {
	[super viewDidUnload];
	redirectedOutputListener=nil;
}

- (void)viewDidLoad {
    [super viewDidLoad];
    if(_coolDebugVCPresented) {
    	self.navigationItem.leftBarButtonItem=[[UIBarButtonItem alloc] initWithTitle:@"Close" style:UIBarButtonItemStyleDone target:self action:@selector(closeCoolDebug)];
    }

    self.view.backgroundColor = [UIColor whiteColor];
    self.textField = [UITextView new];
    self.textField.font = [UIFont fontWithName:@"Courier" size:10];
    
    self.textField.text = [redirectedOutput string];
    redirectedOutputListener=^{
    	self.textField.text=[redirectedOutput string];
    	if(!self.textField.scrollEnabled)
    		return;
    	// https://stackoverflow.com/questions/952412/uiscrollview-scroll-to-bottom-programmatically
    	[self.textField setContentOffset:CGPointMake(0,fmax(self.textField.contentSize.height-self.textField.bounds.size.height+self.textField.contentInset.bottom,-50)) animated:YES];
    };

    [self.view addSubview:self.textField];
    self.textField.translatesAutoresizingMaskIntoConstraints=NO;
    [self.textField.topAnchor constraintEqualToAnchor:self.view.topAnchor].active=1;
    [self.textField.bottomAnchor constraintEqualToAnchor:self.view.bottomAnchor].active=1;
    [self.textField.leadingAnchor constraintEqualToAnchor:self.view.leadingAnchor].active=1;
    [self.textField.trailingAnchor constraintEqualToAnchor:self.view.trailingAnchor].active=1;
    
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
    [export_button addTarget:self action:@selector(DebugExportPressed) forControlEvents:UIControlEventTouchUpInside];
    [export_button setShowsTouchWhenHighlighted:YES];

    UIBarButtonItem *barButton = [[UIBarButtonItem alloc] initWithCustomView:export_button];
    self.navigationItem.rightBarButtonItem = barButton;
}

@end

@implementation SettingsViewController

- (NSString *)title {
	return _("More");
}

- (instancetype)init {
	UITabBarItem *tabbarItem = [[UITabBarItem alloc] initWithTabBarSystemItem:UITabBarSystemItemMore tag:1];
	tabbarItem.title = _("More"); // UITabBarSystemItem cannot change title like this
    [tabbarItem setValue:_("More") forKey:@"internalTitle"]; // This is the correct way (But not accepted by App Store)
	self.tabBarItem = tabbarItem;
	return [super initWithStyle:UITableViewStyleGrouped]; // or plain if desired
}

- (NSInteger)tableView:(id)tv numberOfRowsInSection:(NSInteger)section {
	if (section == SS_SECT_ABOUT)
		return 2;
#ifdef DEBUG
	else if (section == SS_SECT_DEBUG)
		return 5;
#endif
	return 0;
}

- (NSInteger)numberOfSectionsInTableView:(id)tv {
	return SS_SECT_COUNT;
}

- (NSString *)tableView:(UITableView *)tv titleForHeaderInSection:(NSInteger)sect {
	if (sect == SS_SECT_ABOUT)
		return _("About Battman");
#ifdef DEBUG
	else if (sect == SS_SECT_DEBUG)
		return _("Debug");
#endif
	return nil;
}

- (void)tableView:(UITableView *)tv didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
    if (indexPath.section == SS_SECT_ABOUT) {
        if (indexPath.row == 0) {
            [self.navigationController pushViewController:[CreditViewController new] animated:YES];
        } else if (indexPath.row == 1) {
            open_url("https://github.com/Torrekie/Battman");
        }
    }
#ifdef DEBUG
	if (indexPath.section == SS_SECT_DEBUG) {
		if (indexPath.row == 0) {
			if(_coolDebugVCPresented) {
				show_alert("Cool debug VC", "Already presented", "ok");
				[tv deselectRowAtIndexPath:indexPath animated:YES];
				return;
			}
			[self.navigationController pushViewController:[DebugViewController new] animated:YES];
		} else if (indexPath.row == 1){
#ifndef USE_GETTEXT
			[self.navigationController pushViewController:[LanguageSelectionVC new] animated:YES];
#else
			show_alert("USE_GETTEXT", "UNIMPLEMENTED YET", "OK");
#endif
		} else if (indexPath.row == 2){
			app_exit();
		}else if(indexPath.row==3){
			if(_coolDebugVCPresented) {
				show_alert("Cool debug VC", "Already presented", "ok");
				[tv deselectRowAtIndexPath:indexPath animated:YES];
				return;
			}
			_coolDebugVCPresented=1;
			UINavigationController *vc=[[UINavigationController alloc] initWithRootViewController:[DebugViewController new]];
			UITabBarController *tbc=self.tabBarController;
			CGFloat halfHeight=tbc.view.frame.size.height/3;
			self.tabBarController.view.frame=CGRectMake(0,0,tbc.view.frame.size.width,halfHeight*2);
			vc.view.frame=CGRectMake(0,halfHeight*2,tbc.view.frame.size.width,halfHeight);
			[self.tabBarController.view.superview addSubview:vc.view];
			[self.tabBarController addChildViewController:vc];
			//extern void worker_test(void);
			//worker_test();
		}else{
			extern int connect_to_daemon();
			int fd=connect_to_daemon();
			if(!fd) {
				show_alert("Daemon", "Failed to connect to daemon", "ok");
				[tv deselectRowAtIndexPath:indexPath animated:YES];
				return;
			}
			dispatch_queue_t queue = dispatch_queue_create("daemonOutputRedirectQueue", NULL);
			dispatch_async(queue,^{
				char buf[512];
				*buf=6;
				write(fd,buf,1);
				while(1) {
					int len=read(fd,buf,512);
					if(len<=0) {
						close(fd);
						return;
					}
					write(1,buf,len);
				}
			});
			show_alert("Done", "Check logs", "ok");
		}
	}
#endif

    [tv deselectRowAtIndexPath:indexPath animated:YES];
}

- (UITableViewCell *)tableView:(id)tv cellForRowAtIndexPath:(NSIndexPath *)indexPath {
	// TODO: REUSE (Too few cells to reuse for now so no need at this moment)
    if (indexPath.section == SS_SECT_ABOUT) {
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
	if (indexPath.section == SS_SECT_DEBUG) {
		if (indexPath.row == 0) {
			UITableViewCell *cell = [UITableViewCell new];
			cell.textLabel.text = _("Logs (stdout)");
			cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
			return cell;
		} else if (indexPath.row == 1) {
			UITableViewCell *cell = [UITableViewCell new];
			cell.textLabel.text = _("Select language override");
			cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
			return cell;
		} else if (indexPath.row == 2) {
			UITableViewCell *cell = [UITableViewCell new];
			cell.textLabel.text = _("Exit App");
			cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
			return cell;
		} else if (indexPath.row == 3) {
			UITableViewCell *cell = [UITableViewCell new];
			cell.textLabel.text = _("Logs (stdout) (very cool)");
			cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
			return cell;
		}else if(indexPath.row==4) {
			UITableViewCell *cell = [UITableViewCell new];
			cell.textLabel.text = _("Redirect daemon logs");
			cell.accessoryType = UITableViewCellAccessoryDisclosureIndicator;
			return cell;
		}
	}
#endif
	UITableViewCell *batteryChargeCell = [UITableViewCell new];
	batteryChargeCell.textLabel.text = @"Test222";
	return batteryChargeCell;
}

@end

static NSString *_contrib[] = {
	@"Torrekie", @"https://github.com/Torrekie",
	@"Ruphane", @"https://github.com/LNSSPsd",
};

// Credit

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
    open_url([_contrib[indexPath.row * 2 + 1] UTF8String]);
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

// Language
#ifdef USE_GETTEXT
static int __unused cond_localize_cnt = 0;
static int cond_localize_language_cnt=0;
static CFStringRef __unused localization_arr[];
#else
extern int cond_localize_cnt;
extern int cond_localize_language_cnt;
extern CFStringRef localization_arr[];
extern CFStringRef **cond_localize_find(const char *str);
#endif
extern void preferred_language_code_clear(void);

@implementation LanguageSelectionVC

- (NSString *)title {
	return _("Language Override");
}

- (NSInteger)tableView:(id)tv numberOfRowsInSection:(NSInteger)section {
	return cond_localize_language_cnt + 1;
}

- (NSInteger)numberOfSectionsInTableView:(id)tv {
	return 1;
}

- (UITableViewCell *)tableView:(id)tv cellForRowAtIndexPath:(NSIndexPath *)indexPath {
	UITableViewCell *cell = [UITableViewCell new];
	if (indexPath.row == 0) {
		cell.textLabel.text = _("Clear");
		return cell;
	}
#if !defined(USE_GETTEXT)
	cell.textLabel.text = (__bridge NSString *)(*cond_localize_find("English"))[indexPath.row - 1];
	if (preferred_language_code() + 1 == indexPath.row) {
		cell.accessoryType = UITableViewCellAccessoryCheckmark;
	}
#endif
	return cell;
}

- (void)tableView:(UITableView *)tv didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
	if (indexPath.row == 0) {
		const char *homedir = getenv("HOME");
		if (!homedir)
			return;
		char *langoverride_fn = malloc(strlen(homedir) + 20);
		stpcpy(stpcpy(langoverride_fn, homedir), lang_cfg_file());
		remove(langoverride_fn);
		free(langoverride_fn);
		preferred_language_code_clear();
		[tv reloadData];
		return;
	} else {
		int fd = open_lang_override(O_RDWR | O_CREAT, 0600);
		int n = (int)indexPath.row - 1;
		write(fd, &n, 4);
		close(fd);
		preferred_language_code_clear();
		[tv reloadData];
	}
	[tv deselectRowAtIndexPath:indexPath animated:YES];
}

@end
