//
//  main.m
//  Battman
//
//  Created by Torrekie on 2025/1/18.
//

#import <UIKit/UIKit.h>
#import "AppDelegate.h"
#include "main.h"

NSString *cond_localize(NSString *str) {
    NSString *retstr = NULL;

    // Check whether we are running as App */
    if (![NSBundle mainBundle]) {
        
    }
    return retstr;
}

int main(int argc, char * argv[]) {
    NSString * appDelegateClassName;
    @autoreleasepool {
        // Setup code that might create autoreleased objects goes here.
        appDelegateClassName = NSStringFromClass([AppDelegate class]);
    }
    return UIApplicationMain(argc, argv, nil, appDelegateClassName);
}
