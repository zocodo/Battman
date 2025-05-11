#import "SimpleTemperatureViewController.h"

extern NSDictionary *getTemperatureHIDData();

@implementation SimpleTemperatureViewController

- (instancetype)init {
	self=[super initWithStyle:UITableViewStyleGrouped];
	self.tableView.allowsSelection=0;
	temperatureHIDData=getTemperatureHIDData();
	return self;
}

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tv {
	return 1;
}

- (NSInteger)tableView:(id)tv numberOfRowsInSection:(NSInteger)section {
	return temperatureHIDData.count;
}

- (UITableViewCell *)tableView:(UITableView *)tv cellForRowAtIndexPath:(NSIndexPath *)ip {
	UITableViewCell *cell=[tv dequeueReusableCellWithIdentifier:@"stvc:main"];
	if(!cell)
		cell=[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleValue1 reuseIdentifier:@"stvc:main"];
	cell.textLabel.text=temperatureHIDData.allKeys[ip.row];
	cell.detailTextLabel.text=[temperatureHIDData[temperatureHIDData.allKeys[ip.row]] stringValue];
	return cell;
}

@end