#include <sys/socket.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <CoreFoundation/CFDictionary.h>
#include <CoreFoundation/CFNumber.h>
#include <CoreFoundation/CFString.h>
#include "libsmc.h"
#include <stdlib.h>
#include <sys/mman.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <mach-o/dyld.h>
#include <spawn.h>
#include <errno.h>

extern void NSLog(CFStringRef,...);

extern int posix_spawnattr_set_persona_np(const posix_spawnattr_t* __restrict, uid_t, uint32_t);
extern int posix_spawnattr_set_persona_uid_np(const posix_spawnattr_t* __restrict, uid_t);
extern int posix_spawnattr_set_persona_gid_np(const posix_spawnattr_t* __restrict, uid_t);

extern char **environ;

extern CFTypeRef IORegistryEntryCreateCFProperty(void *,CFStringRef,int,int);
extern void subscribeToPowerEvents(void (*cb)(int,void*,int32_t));

struct battman_daemon_settings {
	unsigned char enable_charging_at_level;
	unsigned char disable_charging_at_level;
};

static struct battman_daemon_settings *daemon_settings;
static int CH0ICache=-1;
static int CH0CCache=-1;
static char daemon_settings_path[1024];

static void daemon_control_thread(int fd) {
	NSLog(CFSTR("Daemon: control fd=%d"),fd);
	int readyFlag=1;
	write(fd,&readyFlag,1);
	while(1) {
		char cmd;
		if(recv(fd,&cmd,1,0)<=0) {
			NSLog(CFSTR("Daemon: Closing bc %s"), strerror(errno));
			close(fd);
			return;
		}
		NSLog(CFSTR("Daemon: READ cmd %d"),(int)cmd);
		if(cmd==3) {
			char val=0;
			smc_write_safe('CH0I',&val,1);
			smc_write_safe('CH0C',&val,1);
			//send(fd,&cmd,1,0);
			close(fd);
			exit(0);
		}
	}
}

static void daemon_control() {
	struct sockaddr_un sockaddr;
	sockaddr.sun_family=AF_UNIX;
	strcpy(stpcpy(sockaddr.sun_path,getenv("HOME")),"/Library/dsck");
	remove(sockaddr.sun_path);
	umask(0);
	int sock=socket(AF_UNIX, SOCK_STREAM, 0);
	if(bind(sock,(struct sockaddr*)&sockaddr,sizeof(struct sockaddr_un))!=0)
		abort();
	int trueVal=1;
	setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&trueVal,4);
	if(listen(sock,2)!=0)
		abort();
	while(1) {
		int conn=accept(sock,NULL,NULL);
		if(conn==-1)
			continue;
		NSLog(CFSTR("Daemon: New connection %d"),conn);
		//char cmd;
		//read(conn,&cmd,1);
		//NSLog(CFSTR("Daemon: cmd=%d"),(int)cmd);
		pthread_t ct;
		pthread_create(&ct,NULL,(void*(*)(void*))daemon_control_thread,(void*)(uint64_t)conn);
		pthread_detach(ct);
	}
}

static void powerevent_listener(int a, void *b, int32_t c) {
	if(c!=-536723200)
		return;
	if(access(daemon_settings_path,F_OK)==-1) {
		// Quit when app removed or daemon no longer needed
		exit(0);
	}
	CFNumberRef capacity=IORegistryEntryCreateCFProperty(b,CFSTR("CurrentCapacity"),0,0);
	int val;
	CFNumberGetValue(capacity,kCFNumberIntType,&val);
	NSLog(CFSTR("Daemon: Value=%d"),val);
	CFRelease(capacity);
	if(daemon_settings->enable_charging_at_level!=255) {
		if(val<=daemon_settings->enable_charging_at_level) {
			val=0;
			if(val!=CH0ICache) {
				CH0ICache=val;
				smc_write_safe('CH0I',&val,1);
			}
			return;
		}
		NSLog(CFSTR("Going to disable at %d"),(int)daemon_settings->disable_charging_at_level);
		if(val>=daemon_settings->disable_charging_at_level) {
			val=1;
			if(val!=CH0ICache) {
				CH0ICache=val;
				NSLog(CFSTR("Disabling"));
				smc_write_safe('CH0I',&val,1);
			}
			return;
		}
		return;
	}
	if(daemon_settings->disable_charging_at_level!=255) {
		int val;
		if(val>=daemon_settings->disable_charging_at_level) {
			val=1;
		}else{
			val=0;
		}
		if(val!=CH0CCache) {
			CH0CCache=val;
			smc_write_safe('CH0C',&val,1);
		}
		return;
	}
}

void daemon_main() {
	if(daemon(0,0)!=0)
		abort();
	//char daemon_settings_path[1024];
	char *end=stpcpy(stpcpy(daemon_settings_path,getenv("HOME")),"/Library/daemon");
	strcpy(end,".run");
	int runfd=open(daemon_settings_path,O_RDWR|O_CREAT,0666);
	pid_t pid=getpid();
	write(runfd,&pid,4);
	close(runfd);
	strcpy(end,"_settings");
	int settingsfd=open(daemon_settings_path,O_RDONLY);
	if(settingsfd==-1)
		exit(0); // Not enabled
	daemon_settings=mmap(NULL,sizeof(struct battman_daemon_settings),PROT_READ,MAP_SHARED,settingsfd,0);
	if(!daemon_settings)
		exit(0);
	close(settingsfd);
	_smc_open();
	subscribeToPowerEvents(powerevent_listener);
	pthread_t tmp;
	pthread_create(&tmp,NULL,(void*(*)(void*))daemon_control,NULL);
	pthread_join(tmp,NULL);
}

int battman_run_daemon() {
	posix_spawnattr_t sattr;
	posix_spawnattr_init(&sattr);
	posix_spawnattr_set_persona_np(&sattr,99,1);
	posix_spawnattr_set_persona_uid_np(&sattr,0);
	posix_spawnattr_set_persona_gid_np(&sattr,0);
	char executable[1024];
	uint32_t size=1024;
	_NSGetExecutablePath(executable,&size);
	char *newargv[]={executable,"--daemon",NULL};
	pid_t dpid;
	int err=posix_spawn(&dpid, executable, NULL, &sattr, (char**)newargv, environ);
	if(err!=0) {
		abort();
	}
	posix_spawnattr_destroy(&sattr);
	return dpid;
}

