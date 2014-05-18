// Copyright 1988-1996 NeXT Software, Inc.

/* NXLogError.c
** Matt Watson (mwatson@next.com)
** Created 6-Jun-95
** Remove the objc dependency from NXLogError()
*/

#if !defined(KERNEL)

#include <stdio.h>

#if defined(__svr4__) || defined(hpux)
    #include <fcntl.h>		// for O_RDWR, open()
#endif

#if defined(__MACH__)
    #include <sys/fcntl.h>
#endif

#if defined(WIN32)
    #include <io.h>		// _open(), _close()
    #include <fcntl.h>		// for O_RDWR
    #include <windows.h>
    #include <winnt-pdo.h>
    #include <winbase.h>
#else
    #include <stdarg.h>
    #include <unistd.h>
    #include <syslog.h>
#endif

#if !defined(WIN32)
static int hasTerminal()
{
    static char hasTerm = -1;

    if (hasTerm == -1) {
	int fd = open("/dev/tty", O_RDWR, 0);
	if (fd >= 0) {
	    (void)close(fd);
	    hasTerm = 1;
	} else
	    hasTerm = 0;
    }
    return hasTerm;
}
#endif // not WIN32

void _NXLogError(const char *format, ...)
{
    va_list ap;
    char bigBuffer[4*1024];
#if defined(WIN32)
    HANDLE hEventLog;
    HANDLE hStdError;
    LPSTR pbuf;
    char **argv;
    char ***_NSGetArgv();
#endif //defined(WIN32)

    va_start(ap, format);
    vsprintf(bigBuffer, format, ap);
    va_end(ap);

#if defined(WIN32)
    pbuf = bigBuffer;
    argv = *_NSGetArgv();
    hEventLog = RegisterEventSourceA(NULL, argv[0]);
    if (NULL != hEventLog) {
        ReportEventA(hEventLog, EVENTLOG_ERROR_TYPE, 0, 0, NULL, 1, 0, &pbuf, NULL);
        DeregisterEventSource(hEventLog);
    }
    hStdError = GetStdHandle(STD_ERROR_HANDLE);
    /* If WriteFile fails, we don't care; what else can we do? */
    if (INVALID_HANDLE_VALUE != hStdError)
    {
	DWORD numWritten;
        WriteFile(hStdError, pbuf, strlen(pbuf), &numWritten, NULL);
    }
#else // not WIN32
    if (hasTerminal()) {
	fwrite(bigBuffer, sizeof(char), strlen(bigBuffer), stderr);
	if (bigBuffer[strlen(bigBuffer)-1] != '\n')
	    fputc('\n', stderr);
    } else {
	syslog(LOG_ERR, "%s", bigBuffer);
    }
#endif //defined(WIN32)
}

#endif
