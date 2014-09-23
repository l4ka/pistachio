#include "ramfs.h"

#include <stdio.h>

RamFs::RamFs()
{
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
