#ifndef RAMFS_H
#define RAMFS_H

class RamFs
{
public:
    RamFs();
    bool ReadFile(char *aPath);
    bool WriteFile(char *aPath);
};

#endif // RAMFS_H
