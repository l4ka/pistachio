#include "syslaunch.h"

#include <drivermgr.h>
#include <ramfs.h>

#include <sys/sysinfo.h>

SysLaunch::SysLaunch():
    iScreenNbr(0)
{
    this->VerBanner();

    EDebugPrintf("SysLaunch", "Created new instance of SysLaunch...");
    printf("This system uses %d-sized pages\n", (int)l4e_min_pagesize());
    printf("Switching to screen %d\n", iScreenNbr);

    /* Set the POSIX UID to root (0) */
    setenv("UID", "0", 1);
    setenv("USER", "syslaunch",1);

    printf("Defaulting to user \"%s\" (%s)\n", 
	getenv("USER"), 
	getenv("UID"));

    DriverMgr();

    RamFs rfs;
    rfs.Start(1);

	rfs.ReadFile("/sys/kip/raw");


}

void SysLaunch::VerBanner() {

    struct utsname utsName;
    uname(&utsName);

    printf("\n Welcome to %s %s.%s! Running on %s.\n", utsName.sysname,
           utsName.release, utsName.version, utsName.nodename);
}

/* This function will eventually call system(), or an equivalent */ 
void SysLaunch::EscalateCmd(char *aCmd) {

	if (strcmp(getenv("ACTIVE_CMD"), "uname") == 0) {
		struct utsname utsName;
    		uname(&utsName);

		printf("%s\n", utsName.sysname);
	}

	if (strcmp(getenv("ACTIVE_CMD"), "pwd") == 0) {
		printf("%s\n", getenv("PWD"));
	}

	if (strcmp(getenv("ACTIVE_CMD"), "whoami") == 0 ) {
		printf("%s\n", getenv("USER"));
	}

//	if (strcmp(getenv("ACTIVE_CMD"), "cpucount") == 0 ) {
//		printf("%d CPUs are installed in this system.\n", get_nprocs());
//	}

	if (strcmp(getenv("ACTIVE_CMD"), "yes") == 0 ) {
		while(1) {
				printf("y\n");
			}
	}

	else {
		EDebugPrintf("SysLaunch", "No processor is available for this command. Sorry.");
	}

}				

void SysLaunch::WaitForCmd() {

    struct utsname utsName;
    uname(&utsName);

    while(1) {

	char *cmd;
	strcpy(cmd, GetPolledKbdLine());

	setenv("ACTIVE_CMD", cmd, 1);
	printf("%s@%s:/$ %s\n", getenv("USER"), utsName.nodename, getenv("ACTIVE_CMD"));
	this->EscalateCmd(cmd);

    }
}

int main(void) {
    SysLaunch launch;

    printf("The clock says: %d\n", L4_SystemClock().raw);
    malloc(2);
    int *test = new int(1);

    launch.WaitForCmd();
    return 0;
}
