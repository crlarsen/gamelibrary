
#import <UIKit/UIKit.h>

@class templateViewController;

@interface templateAppDelegate : NSObject <UIAccelerometerDelegate> {
    UIWindow *window;
    templateViewController *viewController;
}

@property (nonatomic, retain) IBOutlet UIWindow *window;
@property (nonatomic, retain) IBOutlet templateViewController *viewController;

@end

