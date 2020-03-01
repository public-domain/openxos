typedef struct grpdata
{   char grplen;
    char grpname[17];
    char usrlen;
    char usrname[17];
    char udpdev[20];
    long addr;
    long port;
} GRPDATA;

int getgroup(char *group, GRPDATA *data);
int reqresp(long udpdev, long addr, long port, void *req, int reqsz,
  void *resp, int respsz);
