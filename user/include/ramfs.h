#ifndef RAMFS_H
#define RAMFS_H


//https://code.google.com/p/ostutorial/wiki/8VFSAndInitrd
class RamFs
{
public:
    RamFs();
    bool ReadFile(char *aPath);
    bool WriteFile(char *aPath);
    int Start(int aStatus);

/* Mount callback? */
/* Format callback */
/* Directories? */

private:
    char *iDrvName;
    char *iDrvVersion;
    char *iDrvSynopsis;
    int iType;
};

#endif // RAMFS_H
