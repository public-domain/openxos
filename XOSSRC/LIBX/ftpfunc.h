long ftpInit(void);

long ftpDebug(int arg);

long ftpOpen(char *host, char *user, char *password);

long ftpUser(char *user);

long ftpPswd(char *password);

long ftpClose(void);

long ftpAscii(void);

long ftpBinary(void);

long ftpCd(char *newdir);

long ftpAllFiles(void);

long ftpLiteral(char *text);

long ftpDelete(char *file);

long ftpRename(char *oldname, char *newname);

long ftpRmDir(char *name);

long ftpMkDir(char *name);

long ftpLs(void);

long ftpDir(void);

long ftpGet(char *rmtname, char *lclname);

long ftpPut(char *rmtname, char *lclname);

long ftpAppend(char *lclname, char *rmtname);

long ftpHelp(char *arg);

long ftpType(char *arg);
