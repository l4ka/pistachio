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

    char *value;

        printf("\n\n[DriverMgr] : Called GetFriendlyType(%d)\n\n", aType);

    switch (aType) {
    case EFileSys:
        value = "File System Driver";
        break;
    case EGenericBlock:
        value = "Generic Block Device Driver";
        break;
    case EHardDisk:
        value = "Hard Disk Driver";
        break;
    case ESmBiosZone:
        value = "System Management BIOS Zone Driver";
	break;
    case EGenericPci:
	value = "Generic PCI Driver";
	break;
    case EGenericUsb:
	value = "Generic USB Driver";
	break; 
    case EGeneric1394:
	value = "Generic IEEE 1394 Driver";
	break;
    case ESerialUart:
	value = "Serial Port/UART Device Driver";
	break;
    case EAirGuitar:
	value = "Loopback Audio Source (Line In) Driver";
	break;
    case EAnalogueMic:
	value = "Analogue Microphone Driver";
	break;
    case EKeyboard:
	value = "Keyboard Driver";
	break;
   case EMouse: 
	value = "Mouse Driver";
	break;
   case ESpeakers: 
	value = "Speaker Driver";
 	break;
    case EMarshmallowAmp:
        value = "Loopback Audio Sink (Line Out) Driver";
        break;

   case EUnknown:
    default:
        value = "Unknown";
        break;
    }

    return value;

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
