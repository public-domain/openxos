long ftpAllFiles(char *arg);
long ftpAppend(char *name, long (*func)(char *bufr, int size), char *bufr,
		int size, int partial);
long ftpAscii(void);
long ftpBinary(void);
long ftpCd(char *newdir);
long ftpClose(void);
long ftpDebug(int arg);
long ftpDelete(char *file);
long ftpDir(char *name, void (*notify)(char *str), long (*func)(char *bufr,
		int size), char *bufr, int size, int partial);
long ftpGet(char *name, void (*notify)(char *str), long (*func)(char *bufr,
		int size), char *bufr, int size, time_sz *fdt, int partial);
long ftpHelp(char *name);
long ftpInit(long *rspval, char *rspstr, int rsplen, int debugarg);
long ftpLiteral(char *text);
long ftpLs(char *name, void (*notify)(char *str), long (*func)(char *bufr,
		int size), char *bufr, int size, int partial);
long ftpMkDir(char *name);
long ftpOpen(char *host);
long ftpPassword(char *password);
long ftpPut(char *name, time_sz *filedt, void (*notify)(char *str),
		long (*func)(char *bufr, int size), char *bufr, int size, int partial);
long ftpPutData(char *bufr, long size);
long ftpPWD(void);
long ftpQuit(void);
long ftpRename(char *oldname, char *newname);
long ftpRmDir(char *name);
long ftpStatus(void);
long ftpSystemType(void);
long ftpType(char *arg);
long ftpUser(char *user);
