#ifndef INTERNALSHELL_H
#define INTERNALSHELL_H

#include <liballoc.h>
//#include <cstring>

//ATA mini driver
#include <mindrvr.h>
#include "elmfat/src/diskio.h"
#include "elmfat/src/ff.h"

#include <l4io.h>
#include <l4/sigma0.h>
#include <l4/kdebug.h>

//ToAru PC speaker shim, move later
#include <pcspkr_shim.h>

/* Track the environment status */
#define ACTIVE_CMD 0
#define CMD_RESULT 1

#define FINISHED 0x00
#define FAILED 0x01
#define WAITING 0x02
#define RUNNING 0x03

class InternalShell
{
public:
    InternalShell();
    int ShellHelp();
};

#endif // INTERNALSHELL_H
