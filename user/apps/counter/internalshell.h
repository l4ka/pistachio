#ifndef INTERNALSHELL_H
#define INTERNALSHELL_H


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
};

#endif // INTERNALSHELL_H
