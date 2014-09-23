#include "drivermgr.h"

#include <stdio.h>

DriverMgr::DriverMgr():
    iDriverCount(0),
    iPageCount(0)
{
    EDebugPrintf("DriverMgr","Initialising Driver Manager...");
}

int DriverMgr::GetDriverCount() {
    return iDriverCount;
}

int DriverMgr::GetPageCount() {
    return iPageCount;
}



char* DriverMgr::GetFriendlyType(int aType) {
    switch (aType) {
    EFileSys:
        return "File System Driver";
    EGenericBlock:
        return "Generic Block Device Driver";
    EHardDisk:
        return "Hard Disk Driver";

    EUnknown:
     default:
        return "Unknown";
    }
}

bool DriverMgr::RegName(char *aName,
                        int aType,
                        char *aSynopsis,
                        char *aVersion) {

    printf("\n\n[DrvrMgr] : Registering device of type %s \n\n", GetFriendlyType(aType));

    return 0;

}


bool DriverMgr::RegName(char *aName,
                        int aType,
                        bool (*aHook)(int),
                        char *aSynopsis,
                        char *aVersion) {

    printf("\n\n[DrvrMgr] : Registering device of type %s \n\n", GetFriendlyType(aType));

    return 0;

}
