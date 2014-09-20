#ifndef SYSLAUNCH_H
#define SYSLAUNCH_H

#include <liballoc.h>
#include <stdio.h>
#include <stdlib.h>
#include <l4io.h>
#include <l4/sigma0.h>
#include <l4/kdebug.h>



class SysLaunch
{
public:
    SysLaunch();
    void WaitForCmd();
};

#endif // SYSLAUNCH_H
