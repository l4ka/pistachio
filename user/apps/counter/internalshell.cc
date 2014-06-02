#include "internalshell.h"

InternalShell::InternalShell()
{
}

int InternalShell::ShellHelp() {
    printf("\n\n This shell supports the following commands: \n");

    printf("\n\t * beep          : Beep the PC speaker.\n");
    printf("\n\t * help, h, Help : Print this shell help notice.\n");
    printf("\n\t * malloc_test_1 : Test the liballoc port (should return 25).\n");
    printf("\n\t * shiritori     : Start the Shiritori game \(buggy!\).\n");
    printf("\n\t * tettheme      : Play part of the Tetris theme. \n");

    printf("\n\n");

printf("\n다른 여자 만나니까 좋더라\n");

printf("Raw system clock time: %d\n\n",L4_SystemClock().raw);

//texmain();

return FINISHED;

}
