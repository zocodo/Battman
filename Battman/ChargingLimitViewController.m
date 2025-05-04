#import "ChargingLimitViewController.h"
#import "SliderTableViewCell.h"
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include "common.h"
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/mman.h>

enum {
	CL_SECTION_MAIN,
	CL_SECTION_COUNT
};

@implementation ChargingLimitViewController

- (instancetype)init {
	self=[super initWithStyle:UITableViewStyleGrouped];
	[self.tableView registerClass:[SliderTableViewCell class] forCellReuseIdentifier:@"clhighthr"];
	[self.tableView registerClass:[SliderTableViewCell class] forCellReuseIdentifier:@"cllowthr"];
	char buf[1024];
	char *end=stpcpy(stpcpy(buf,getenv("HOME")),"/Library/daemon");
	strcpy(end,".run");
	int drfd=open(buf, O_RDONLY);
	if(drfd!=-1) {
		int pid;
		if(read(drfd,&pid,4)==4) {
			if(kill(pid,0)==0||errno!=ESRCH) {
				daemon_pid=pid;
			}
		}
		close(drfd);
	}
	strcpy(end,"_settings");
	int fd=open(buf,O_RDWR|O_CREAT);
	char _vals[2];
	if(read(fd,_vals,2)!=2) {
		_vals[0]=-1;
		_vals[1]=-1;
		lseek(fd,0,SEEK_SET);
		write(fd,_vals,2);
	}
	lseek(fd,0,SEEK_SET);
	vals=mmap(NULL,2,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
	close(fd);
	return self;
}

- (void)dealloc {
	munmap(vals,2);
	return;
}

- (NSInteger)tableView:(id)tv numberOfRowsInSection:(NSInteger)sect {
	return 7;
}

- (NSInteger)numberOfSectionsInTableView:(id)tv {
	return 1;
}

- (void)tableView:(UITableView *)tv didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
	if(indexPath.row==6) {
		if(daemon_pid) {
			struct sockaddr_un sockaddr;
			sockaddr.sun_family=AF_UNIX;
			strcpy(stpcpy(sockaddr.sun_path,getenv("HOME")),"/Library/dsck");
			int sock=socket(AF_UNIX,SOCK_STREAM,0);
			int sfd;
			if((sfd=connect(sock,(struct sockaddr*)&sockaddr,sizeof(struct sockaddr_un))==-1)) {
				show_alert("Error", "Failed to connect to daemon","ok");
				close(sock);
				[tv deselectRowAtIndexPath:indexPath animated:YES];
				return;
			}
			int dmp;
			read(sfd,&dmp,1);
			int stop_cmd=3;
			send(sfd,&stop_cmd,1,0);
			close(sfd);
			close(sock);
			daemon_pid=0;
			[tv reloadData];
		}else{
			extern int battman_run_daemon();
			daemon_pid=battman_run_daemon();
			[tv reloadData];
		}
	}
	[tv deselectRowAtIndexPath:indexPath animated:YES];
}

- (void)cltypechanged:(UISegmentedControl *)segCon {
	vals[0]=segCon.selectedSegmentIndex?0:-1;
	[self.tableView reloadData];
}

- (UITableViewCell *)tableView:(UITableView *)tv cellForRowAtIndexPath:(NSIndexPath *)indexPath {
	UITableViewCell *cell=[UITableViewCell new];
	cell.selectionStyle=UITableViewCellSelectionStyleNone;
	if(indexPath.row==0) {
		cell.textLabel.text=@"When Threshold Reached";
		UISegmentedControl *segCon=[[UISegmentedControl alloc] initWithItems:@[@"Halt",@"Discharge"]];
		if(vals[0]==-1) {
			segCon.selectedSegmentIndex=0;
		}else{
			segCon.selectedSegmentIndex=1;
		}
		[segCon addTarget:self action:@selector(cltypechanged:) forControlEvents:UIControlEventValueChanged];
		cell.accessoryView=segCon;
		return cell;
	}else if(indexPath.row==1) {
		cell.textLabel.text=@"Charging Threshold (High End)";
		return cell;
	}else if(indexPath.row==2) {
		SliderTableViewCell *scell=[tv dequeueReusableCellWithIdentifier:@"clhighthr" forIndexPath:indexPath];
		if(!scell) {
			scell=[[SliderTableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:@"clhighthr"];
		}
		scell.slider.minimumValue=0;
		scell.slider.maximumValue=100;
		scell.slider.enabled=1;
		scell.textField.enabled=1;
		scell.delegate=(id)self;
		if(vals[1]==-1) {
			scell.slider.value=100;
			scell.textField.text=@"100";
		}else{
			scell.slider.value=(float)vals[1];
			scell.textField.text=[NSString stringWithFormat:@"%d",(int)vals[1]];
		}
		return scell;
	}else if(indexPath.row==3) {
		if(vals[0]==-1)
			cell.textLabel.enabled=NO;
		cell.textLabel.text=@"Discharging Threshold (Low End)";
		return cell;
	}else if(indexPath.row==4) {
		SliderTableViewCell *scell=[tv dequeueReusableCellWithIdentifier:@"cllowthr" forIndexPath:indexPath];
		if(!scell) {
			scell=[[SliderTableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:@"cllowthr"];
		}
		scell.slider.minimumValue=0;
		scell.slider.maximumValue=100;
		scell.delegate=(id)self;
		if(vals[0]==-1) {
			scell.slider.enabled=NO;
			scell.slider.userInteractionEnabled=NO;
			scell.slider.value=0;
			scell.textField.enabled=0;
			scell.textField.userInteractionEnabled=0;
			scell.textField.text=@"0";
		}else{
			scell.slider.enabled=1;
			scell.slider.userInteractionEnabled=1;
			scell.slider.value=(float)vals[0];
			scell.textField.enabled=1;
			scell.textField.userInteractionEnabled=1;
			scell.textField.text=[NSString stringWithFormat:@"%d",(int)vals[0]];
		}
		return scell;
	}else if(indexPath.row==5) {
		if(daemon_pid) {
			cell.textLabel.text=[NSString stringWithFormat:@"Daemon Running (pid: %d)",daemon_pid];
		}else{
			cell.textLabel.text=@"Daemon Not Running";
		}
		return cell;
	}else if(indexPath.row==6) {
		cell.selectionStyle=UITableViewCellSelectionStyleDefault;
		if(daemon_pid) {
			cell.textLabel.text=@"Kill Daemon (Disable Charging Limit)";
		}else{
			cell.textLabel.text=@"Start Daemon (Enforce Charging Limit)";
		}
		cell.textLabel.textColor=[UIColor linkColor];
	}
	return cell;
}

- (void)sliderTableViewCell:(SliderTableViewCell *)cell didChangeValue:(float)value {
	BOOL isHighThr=[cell.reuseIdentifier isEqualToString:@"clhighthr"];
	if(isHighThr&&value<vals[0]) {
		show_alert("Invalid value", "High end shall not have a value smaller than low end","ok");
		[self.tableView reloadData];
		return;
	}else if(!isHighThr&&value>vals[1]) {
		show_alert("Invalid value", "Low end shall not have a value bigger than high end","ok");
		[self.tableView reloadData];
		return;
	}
	vals[isHighThr]=(char)value;
}

@end