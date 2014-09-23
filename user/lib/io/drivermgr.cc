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

        printf("\n\n[DriverMgr] : Called GetFriendlyType(%d)\n\n", aType);

    switch (aType) {
    EFileSys:
        return "File System Driver";
        break;
    EGenericBlock:
        return "Generic Block Device Driver";
        break;
    EHardDisk:
        return "Hard Disk Driver";
        break;

    EUnknown:
     default:
        return "Unknown";
        break;
    }

}

bool DriverMgr::RegName(char *aName,
                        int aType,
                        char *aSynopsis,
                        char *aVersion) {

    printf("\n\n[DriverMgr] : Registering device of type %s \n\n", GetFriendlyType(aType));
    printf("\n\n[DriverMgr] : This driver is %s, %s (%s)\n\n", aName, aSynopsis, aVersion);

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
