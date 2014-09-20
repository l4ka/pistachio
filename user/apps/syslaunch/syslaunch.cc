#include "syslaunch.h"

SysLaunch::SysLaunch()
{
}

void SysLaunch::WaitForCmd() {

    while(1) {
        EDebugPrintf("SysLaunch","Still waiting...\n");
    }
}

int main(void) {
    //SysLaunch *launch = new SysLaunch();
    //launch->WaitForCmd();
    //int *test;
    //test = new int(1);

    printf("The clock says: %x\n", L4_SystemClock().raw);
    malloc(2);
    int *test = new int(1);

    return 0;
}
