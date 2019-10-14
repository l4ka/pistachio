#include "ramfs.h"

#include <stdio.h>
#include <drivermgr.h>

RamFs::RamFs():
    iDrvName("RamFs"),
    iDrvSynopsis("A RAM disk driver"),
    iType(EGenericBlock),
    iDrvVersion("0.0.0")
{
    iDrvName = iDrvName;

    EDebugPrintf("RamFs", "Initialised RamFs...");

    DriverMgr::RegName(iDrvName, iType,
                       /*RamFs::Start(0),*/ iDrvSynopsis, iDrvVersion);

}

int RamFs::Start(int aStatus) {

    RamFs *fs = new RamFs();

    return aStatus;
}

bool RamFs::ReadFile(char *aPath) {

    EDebugPrintf("RamFs", "Reading files is unimplemented");

    printf("RamFs::ReadFile(%d)\n", aPath);

    return -1;
}

bool RamFs::WriteFile(char *aPath) {
    EDebugPrintf("RamFs", "Writing files is unsupported");

    printf("RamFs::WriteFile(%d)\n", aPath);

    return -1;
}
