#include "ramfs.h"

#include <stdio.h>
#include <drivermgr.h>

RamFs::RamFs()
{
    EDebugPrintf("RamFs", "Initialised RamFs...");
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
