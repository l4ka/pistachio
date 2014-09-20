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
    SysLaunch::SysLaunch launch;
    launch.WaitForCmd();

    return 0;
}
