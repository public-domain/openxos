
typedef void (*mkbfail)(char *str1, long code, char *str2);
typedef void (*mkbnotice)(char *str);

int mkbootf(char diskname[], char *disktype, char *fstype, char fname[10][50],
    char defprog[], char autoboot, int timeout, char quiet,
    mkbfail fail, mkbnotice notice);

int rmbootf(char diskname[], char *disktype, mkbfail fail);
