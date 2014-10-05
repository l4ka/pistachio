#include "syslaunch.h"

#include <drivermgr.h>
#include <ramfs.h>


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

    DriverMgr();

    RamFs rfs;
    rfs.Start(1);


}

void SysLaunch::VerBanner() {

    struct utsname utsName;
    uname(&utsName);

    printf("\n Welcome to %s %s.%s! Running on %s.\n", utsName.sysname,
           utsName.release, utsName.version, utsName.nodename);
}

/* This function will eventually call system(), or an equivalent */ 
void SysLaunch::EscalateCmd(char *aCmd) {

printf("Entered %s\n", aCmd);

	if (strcmp(aCmd, "uname") == 0) {
		struct utsname utsName;
    		uname(&utsName);

		printf("%s\n", utsName.sysname);
	}

	else {
		EDebugPrintf("SysLaunch", "No processor is available for this command. Sorry.");
	}

}

void SysLaunch::WaitForCmd() {

    while(1) {
        //EDebugPrintf("SysLaunch","Still waiting...\n");

	char* cmd = NULL;

	cmd = getenv("ACTIVE_CMD");

	setenv("ACTIVE_CMD", GetPolledKbdLine(), 1);
	this->EscalateCmd(cmd);
	printf(getenv("ACTIVE_CMD"));
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
