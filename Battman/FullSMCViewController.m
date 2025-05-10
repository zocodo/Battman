#import "FullSMCViewController.h"
#include "battery_utils/libsmc.h"

static void hton_with_type(void *bytes,int type) {
	if(type=='1') { // 16
		uint16_t *val=(uint16_t*)bytes;
		*val=htons(*val);
	}else if(type=='3') { //32
		uint32_t *val=(uint32_t*)bytes;
		*val=htonl(*val);
	}else{ //64
		uint32_t *val=(uint32_t*)bytes;
		*(uint64_t*)bytes=((uint64_t)htonl(*val) <<32LL)|(uint64_t)htonl(val[1]);
	}
}

@implementation FullSMCViewController

- (instancetype)init {
	self=[super initWithStyle:UITableViewStyleGrouped];
	SMCParamStruct input={0};
	SMCParamStruct output;
	input.param.data8=kSMCGetKeyCount;
	smc_call(kSMCHandleYPCEvent,&input,&output);
	NSLog(@"SMC Count=%d",htonl(output.param.data32));
	numKeys=htonl(output.param.data32);
	input.param.data8=kSMCGetKeyFromIndex;
	allkeys=malloc(sizeof(struct smc_key)*numKeys);
	for(int i=0;i<numKeys;i++) {
		input.param.data32=i;
		smc_call(kSMCHandleYPCEvent,&input,&output);
		//output.key=htonl(output.key);
		//NSLog(@"SMC[%d]=%.4s",i,(char*)&output.key);
		allkeys[i].key=output.key;
	}
	input.param.data32=0;
	input.param.data8=kSMCGetKeyInfo;
	for(int i=0;i<numKeys;i++) {
		input.key=allkeys[i].key;
		smc_call(kSMCHandleYPCEvent,&input,&output);
		//int type=htonl(output.param.keyInfo.dataType);
		//NSLog(@"type=%.4s",(char*)&type);
		allkeys[i].dataSize=output.param.keyInfo.dataSize;
		allkeys[i].dataType=output.param.keyInfo.dataType;
	}
	UISegmentedControl *endiannessControl=[[UISegmentedControl alloc] initWithItems:@[@"Little Endian", @"Big Endian"]];
	self.navigationItem.rightBarButtonItem=[[UIBarButtonItem alloc] initWithCustomView:endiannessControl];
	endiannessControl.selectedSegmentIndex=0;
	[endiannessControl addTarget:self action:@selector(endiannessChanged:) forControlEvents:UIControlEventValueChanged];
	return self;
}

- (void)endiannessChanged:(UISegmentedControl *)ec {
	mode=ec.selectedSegmentIndex;
	[self.tableView reloadData];
}

- (void)dealloc {
	free(allkeys);
}

- (NSInteger)numberOfSectionsInTableView:(id)tv {
	return 1;
}

- (void)tableView:(UITableView *)tv didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
	[tv deselectRowAtIndexPath:indexPath animated:1];
}

- (NSInteger)tableView:(id)tv numberOfRowsInSection:(NSInteger)sect {
	return numKeys;
}

- (UITableViewCell *)tableView:(UITableView *)tv cellForRowAtIndexPath:(NSIndexPath *)indexPath {
	UITableViewCell *cell=[tv dequeueReusableCellWithIdentifier:@"regularSMCItemCell"];
	if(!cell)
		cell=[[UITableViewCell alloc] initWithStyle:UITableViewCellStyleValue1 reuseIdentifier:@"regularSMCItemCell"];
	SMCParamStruct smcstr={0};
	struct smc_key *curkey=allkeys+indexPath.row;
	smcstr.key=curkey->key;
	smcstr.param.keyInfo.dataSize=curkey->dataSize;
	smcstr.param.data8=kSMCReadKey;
	smc_call(kSMCHandleYPCEvent,&smcstr,&smcstr);
	unsigned int type=htonl(curkey->dataType);
	unsigned int key=htonl(curkey->key);
	cell.textLabel.text=[NSString stringWithFormat:@"%.4s(%u) %.4s",(char*)&type,curkey->dataSize,(char*)&key];
	if(smcstr.param.status) {
		cell.detailTextLabel.text=@"(Unable to read)";
		cell.detailTextLabel.enabled=0;
	}else{
		NSString *text;
		char *ptype=(char*)&type;
		if(*ptype=='h') {
			// hex_
			text=[NSString stringWithFormat:@"%0*llX",curkey->dataSize,*(uint64_t*)smcstr.param.bytes];
		}else if(ptype[3]=='*') {
			if(*ptype=='u') {
				// ui8\*
				NSMutableArray *arr=[NSMutableArray array];
				for(int i=0;i<curkey->dataSize;i++) {
					[arr addObject:[NSString stringWithFormat:@"%u",smcstr.param.bytes[i]]];
				}
				text=[arr componentsJoinedByString:@","];
			}else if(*ptype=='c'){
				// ch8\*
				NSMutableArray *arr=[NSMutableArray array];
				for(int i=0;i<curkey->dataSize;i++) {
					[arr addObject:[NSString stringWithFormat:@"%d",(int)smcstr.param.bytes[i]]];
				}
				text=[arr componentsJoinedByString:@","];
			}else{
				text=@"Unknown array";
			}
		}else if(*ptype=='u') {
			// ui.{2}
			if(mode&&ptype[2]!='8')
				hton_with_type(smcstr.param.bytes,ptype[2]);
			uint64_t val=*(uint64_t *)&smcstr.param.bytes;
			text=[NSString stringWithFormat:@"%llu",val];
		}else if(*ptype=='s') {
			// si.{2}
			if(mode&&ptype[2]!='8')
				hton_with_type(smcstr.param.bytes,ptype[2]);
			int64_t val=*(int64_t *)&smcstr.param.bytes;
			if(ptype[2]=='8')
				text=[NSString stringWithFormat:@"%d",(int)(char)val];
			else if(ptype[2]=='1')
				text=[NSString stringWithFormat:@"%hd",(short)val];
			else if(ptype[2]=='3')
				text=[NSString stringWithFormat:@"%d",(int)val];
			else
				text=[NSString stringWithFormat:@"%lld",val];
		}else if(*ptype=='f'&&ptype[2]=='t') {
			// flt
			if(mode)
				hton_with_type(smcstr.param.bytes,'3');
			text=[NSString stringWithFormat:@"%0.3f",*(float*)&smcstr.param.bytes];
		}else if(*ptype=='f'){
			// flag
			NSMutableString *ft=[NSMutableString stringWithCapacity:8*curkey->dataSize];
			for(int i=curkey->dataSize-1;i>=0;i--) {
				for(int b=7;b>=0;b--) {
					[ft appendString:(smcstr.param.bytes[i]&(1<<b))?@"1":@"0"];
				}
			}
			text=ft;
		}else if(*ptype=='i') {
			// ioft
			if(mode)
				hton_with_type(smcstr.param.bytes,'6');
			uint64_t res=0;
			for(int i=0;i<8;i++) {
				res|=(uint64_t)smcstr.param.bytes[i] << (i<<3);
			}
			text=[NSString stringWithFormat:@"%0.3f",*(double*)smcstr.param.bytes];
		}else{
			text=[NSString stringWithFormat:@"(Type unknown) %0*llX",curkey->dataSize,*(uint64_t*)smcstr.param.bytes];
		}
		cell.detailTextLabel.text=text;
		cell.detailTextLabel.enabled=1;
	}
	return cell;
}

@end