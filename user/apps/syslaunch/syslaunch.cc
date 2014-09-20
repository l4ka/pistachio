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
    malloc(2);

    return 0;
}
