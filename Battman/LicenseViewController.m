#import "LicenseViewController.h"
#include "intlextern.h"
#include "common.h"

/* Ignore UIWebView deprecation warnings for now */
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

@interface LicenseViewController ()
@property (nonatomic, strong) UIWebView *webView;
@property (nonatomic, strong) UIToolbar *bottomToolbar;
@end

@implementation LicenseViewController

- (NSString *)composeLocalizedLicense {
    // TODO: Non-free encryptions for License
#if LICENSE == LICENSE_NONFREE
    NSString *locale = _("locale_name");
    char *loc_license = license_en;
    if ([locale isEqualToString:@"中文"]) {
        loc_license = license_zh;
    }
    return [NSString stringWithFormat:@"%s", loc_license];
#endif

    NSString *bodyStruct = [NSString stringWithFormat:@"<html><body><h1>%@</h1><p>%@</p></body></html>", _("Battman License"), _("Battman does not use non-free license at current stage, press Agree to proceed.")];
    return bodyStruct;
}

- (void)viewDidLoad {
    [super viewDidLoad];
    self.title = _("Term of Use & Privacy Policies");
    UIColor *bgColor = [UIColor whiteColor];

    if (@available(iOS 13.0, *)) {
        bgColor = [UIColor systemBackgroundColor];
    }
    self.view.backgroundColor = bgColor;

    self.webView = [[UIWebView alloc] initWithFrame:CGRectZero];
    self.webView.translatesAutoresizingMaskIntoConstraints = NO;
    [self.view addSubview:self.webView];

    NSString *htmlString = [self composeLocalizedLicense];
    [self.webView loadHTMLString:htmlString baseURL:nil];
    
    // 2. Create and add a bottom UIToolbar
    self.bottomToolbar = [[UIToolbar alloc] initWithFrame:CGRectZero];
    self.bottomToolbar.translatesAutoresizingMaskIntoConstraints = NO;
    [self.view addSubview:self.bottomToolbar];
    
    // Create toolbar items
    UIBarButtonItem *acceptItem = [[UIBarButtonItem alloc]
                                   initWithTitle:_("Agree")
                                   style:UIBarButtonItemStylePlain
                                   target:self
                                   action:@selector(handleAccept)];
    
    UIBarButtonItem *flexibleSpace = [[UIBarButtonItem alloc]
                                      initWithBarButtonSystemItem:UIBarButtonSystemItemFlexibleSpace
                                      target:nil
                                      action:nil];
    
    UIBarButtonItem *denyItem = [[UIBarButtonItem alloc]
                                 initWithTitle:_("Deny")
                                 style:UIBarButtonItemStylePlain
                                 target:self
                                 action:@selector(handleDeny)];
    
    [self.bottomToolbar setItems:@[acceptItem, flexibleSpace, denyItem]];

    [NSLayoutConstraint activateConstraints:@[
        // WebView constraints: top to safe area, left/right to superview, bottom to toolbar top
        [self.webView.topAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.topAnchor],
        [self.webView.leadingAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.leadingAnchor],
        [self.webView.trailingAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.trailingAnchor],

        [self.webView.bottomAnchor constraintEqualToAnchor:self.bottomToolbar.topAnchor],
        
        [self.bottomToolbar.leadingAnchor constraintEqualToAnchor:self.view.leadingAnchor],
        [self.bottomToolbar.trailingAnchor constraintEqualToAnchor:self.view.trailingAnchor],
        [self.bottomToolbar.bottomAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.bottomAnchor],
    ]];
}

// Handle Accept
- (void)handleAccept {
    // if unsandboxed, the config will be located at ~/Library/Preferences/com.torrekie.Battman.plist
    // which editable by `defaults` command
    NSString *containerID = [[UIDevice currentDevice] identifierForVendor].UUIDString;
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    [defaults setObject:containerID forKey:@"ContainerID"];
    [defaults synchronize];
    show_alert_async(_C("Agreed"), _C("Reopen Battman to take effect"), _C("OK"), ^(bool ok) {
        app_exit();
    });
}

// Handle Deny
- (void)handleDeny {
    show_alert_async(_C("Denied"), _C("You will not able to use Battman before you agree the Term of Use."), _C("OK"), NULL);
}

@end
#pragma clang diagnostic pop
