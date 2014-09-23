#ifndef RAMFS_H
#define RAMFS_H



class RamFs
{
public:
    RamFs();
    bool ReadFile(char *aPath);
    bool WriteFile(char *aPath);
    int Start(int aStatus);

private:
    char *iDrvName;
    char *iDrvVersion;
    char *iDrvSynopsis;
    int iType;
};

#endif // RAMFS_H
