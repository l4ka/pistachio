#ifndef DRIVERMGR_H
#define DRIVERMGR_H

enum {

    EFileSys = 1,
    EGenericBlock = 2,
    EHardDisk = 3,
    ESmBiosZone = 4,
    EGenericPci = 5,
    EGenericUsb = 6,
    EGeneric1394 = 7,
    ESerialUart = 8,
    EAirGuitar = 9, /* Hear no evil (loopback audio accessory) */
    EAnalogueMic = 10,
    EKeyboard = 11,
    EMouse = 12,
    ESpeakers = 13,
    EMarshmallowAmp = 14, /* Loopback audio accessory - sink */
    EUnknown = 0

} TDriver;

/* Potential errors (File System) */
static const int KErrNone = 0; //Operation successful
static const int KErrAccessDenied = 1; //Access denied, or impossible action
static const int KErrNotSupported = 2; //Action not supported
static const int KErrVolumeFull = 3; //The disk is full
static const int KErrCompletion = 4; //Task completed
static const int KErrEof = 5; //End of file
static const int KErrVolumeDirty = 6; //Disk wasn't unmounted cleanly
static const int KErrExtAttrTooLarge = 7; //The extended attribute's payload exceeds the supported size
static const int KErrDisconnected = 8; //The network server went away
static const int KErrGeneral = 9; //General error...
static const int KErrArgument = 10; //Argument error
static const int KErrTooBig = 11; //Specified file/sector/cluster size exceeds the volume size
static const int KErrCorrupt = 12; //File/boot sector is damaged; not necessarily same as 6
static const int KErrNotFound = 13; //The requested file/cluster/sector could not be located for some reason

class DriverMgr
{
public:
    DriverMgr();

    //Constructor taking L4 thread/page to sit in?

    /* Register a driver by name

      aName
      aType
      aHook
      aSynopsis
      aVersion

     */

    bool RegName(char *aName, int aType,
        bool (*aHook)(int), char *aSynopsis, char *aVersion);

  static bool RegName(char *aName, int aType,
        char *aSynopsis, char *aVersion);

    /* Unregister a driver by name */
    
    /* Unregister a driver by ordinal */

    /* Return a list of drivers, types, synopses, versions, and PID/thread IDs */

    /* Return the number of registered drivers, globally */
    int GetDriverCount();

    /* Return the total number of allocated pages, globally */
    int GetPageCount();

    static char *GetFriendlyType(int aType);

private:
    int iDriverCount; //Number of registered drivers
    //The list of drivers (linked list/hash map/table?)
    int iPageCount; //Number of allocated pages?

};

#endif // DRIVERMGR_H
