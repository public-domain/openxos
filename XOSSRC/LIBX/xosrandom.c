#include <STDIO.H>
#include <STDDEF.H>
#include <STDLIB.H>
#include <STRING.H>

void randomInit(void)

{

}


void randomGetBytes(
	char *bufr,
	int   size)

{
	memset(bufr, 0x39, size);
}
