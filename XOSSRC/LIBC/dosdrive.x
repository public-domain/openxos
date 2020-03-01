#define uchar  unsigned char
#define ushort unsigned short
#define uint   unsigned int
#define ulong  unsigned long

#define DDML_INFO     1
#define DDML_ERROR    2
#define DDML_FININFO  3
#define DDML_FINERROR 4

int  dosdrvf(char *driveletter, char quiet);
void message(int type, char *text);
