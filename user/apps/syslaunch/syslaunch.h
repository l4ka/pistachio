#ifndef SYSLAUNCH_H
#define SYSLAUNCH_H

#include <liballoc.h>
#include <stdio.h>
#include <stdlib.h>
#include <l4io.h>
#include <l4/sigma0.h>
#include <l4/kdebug.h>

#include <l4e/misc.h>

#include <sys/utsname.h>



class SysLaunch
{
public:
    SysLaunch();
    void WaitForCmd();
    void VerBanner();
};

#endif // SYSLAUNCH_H
