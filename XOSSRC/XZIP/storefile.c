#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <xoserr.h>
#include <xostrm.h>
#include <xosrtn.h>
#include <xos.h>
#include <xossvc.h>
#include <xostime.h>
#include <xoserrmsg.h>
#include <progarg.h>
#include <proghelp.h>
#include <global.h>
#include <xosstr.h>
#include <xoszipfile.h>
#include <xoswildcard.h>
#include <dirscan.h>
#include <zlib.h>
#include "xzip.h"


int storefile(void)

{
	long amount;
	long rtn;
	long size;
	long last;

	printf("  Storing: %.*s ... ", fipnt->lfh.namelen, fipnt->spec);
	size = fipnt->lfh.size;
	last = 0;
	fipnt->lfh.method = METHOD_STORE;

	fipnt->lfh.size = 0;
	fipnt->lfh.crc32 = 0;
	while ((amount = svcIoInBlock(ifhndl, filebufr, FILEBUFR)) > 0)
	{
		fipnt->lfh.size += amount;
		if (size > (2 * FILEBUFR))
		{
			rtn = ratio(size - fipnt->lfh.size, size);
			if (rtn != last)
			{
				last = rtn;
				printf("\b\b\b\b%%%02.2d ", rtn);
			}
		}
		if ((rtn = svcIoOutBlockP(zfhndl, filebufr, amount, &zfoutparms)) < 0)
			femsg2(prgname, "Error writing data to ZIPfile", rtn, NULL);
		fipnt->lfh.crc32 = crc32(fipnt->lfh.crc32, filebufr, amount);
		zfoutparms.pos.value += amount;
	}
	if (amount != 0 && amount != ER_EOF)
	{
		fileerror("reading", amount);
		return (FALSE);
	}
	fipnt->lfh.comsize = fipnt->lfh.size;
	if (last != 0)
		fputs("\b\b\b\b... ", stdout);
	return (TRUE);
}
