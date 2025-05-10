#import "BatterySubscriberViewControllerBase.h"

struct smc_key {
	unsigned int key;
	unsigned int dataSize;
	unsigned int dataType;
};

@interface FullSMCViewController:BatterySubscriberViewControllerBase
{
	int numKeys;
	struct smc_key *allkeys;
}
@end