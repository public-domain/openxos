typedef struct udf
{   int    handle;
    int    offset;
    int    recpos;
    int    reccnt;
    int    hdrlen;
    int    blklen;
    int    next;
    uchar *blkpnt;
    uchar *dskpnt;
    int    dskcnt;
    uchar  dskbufr[1024];
} UDF;

void udfClose(UDF *udf);
int  udfGetBlock(UDF *udf);
int  udfGetRecord(UDF *udf, uchar *buffer, int size);
UDF *udfOpen(char *name);
