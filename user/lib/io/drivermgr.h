#ifndef DRIVERMGR_H
#define DRIVERMGR_H

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

    /* Unregister a driver by name */
    /* Unregister a driver by ordinal */

    /* Return a list of drivers, types, synopses, and versions */

    /* Return the number of registered drivers, globally */

    /* Return the total number of allocated pages, globally */

private:
    int iDriverCount; //Number of registered drivers
    //The list of drivers (linked list/hash map/table?)
    int iPageCount; //Number of allocated pages?

};

#endif // DRIVERMGR_H
