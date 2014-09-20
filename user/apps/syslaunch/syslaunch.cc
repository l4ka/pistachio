#include "syslaunch.h"

SysLaunch::SysLaunch()
{
    EDebugPrintf("SysLaunch", "Created new instance of SysLaunch...");
    printf("This system uses %d-sized pages", (int)l4e_min_pagesize());
}

void SysLaunch::WaitForCmd() {

    while(1) {
        EDebugPrintf("SysLaunch","Still waiting...\n");
    }
}

int main(void) {
    SysLaunch launch;

    //int *test;
    //test = new int(1);

    printf("The clock says: %x\n", L4_SystemClock().raw);
    malloc(2);
    int *test = new int(1);

    launch.WaitForCmd();
    return 0;
}
