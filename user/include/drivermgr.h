#ifndef DRIVERMGR_H
#define DRIVERMGR_H

enum {

    EFileSys = 1,
    EGenericBlock = 2,
    EHardDisk = 3,
    EUnknown = 0

} TDriver;

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
