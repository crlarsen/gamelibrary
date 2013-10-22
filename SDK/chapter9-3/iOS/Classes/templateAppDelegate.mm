#import "templateAppDelegate.h"
#import "templateViewController.h"

#include "templateApp.h"

@implementation templateAppDelegate

@synthesize window;
@synthesize viewController;

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
	[[UIApplication sharedApplication] setIdleTimerDisabled:YES];

	// Set up the accelerometer to be updated 24 times per second.
    //[[UIAccelerometer sharedAccelerometer ] setUpdateInterval:( 1.0f / 24.0f )];
	
	// Startup the accelerometer
    //[[UIAccelerometer sharedAccelerometer] setDelegate:self];	


    self.window.rootViewController = self.viewController;
    return YES;
}

- (void)applicationWillResignActive:(UIApplication *)application
{
    [self.viewController stopAnimation];
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
    [self.viewController startAnimation];
}

- (void)applicationWillTerminate:(UIApplication *)application
{
    [self.viewController stopAnimation];
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
    // Handle any background procedures not related to animation here.
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
    // Handle any foreground procedures not related to animation here.
}

- (void)dealloc
{
    [viewController release];
    [window release];
    
    [super dealloc];
}


- (void)accelerometer:(UIAccelerometer *)accelerometer didAccelerate:(UIAcceleration *)acceleration
{
	if( templateApp.Accelerometer )
	{
		templateApp.Accelerometer( acceleration.x,
								   acceleration.y,
								   acceleration.z );
	}
}


@end
