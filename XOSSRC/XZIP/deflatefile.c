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


int deflatefile(void)

{
	z_stream zstream;
	long     rtn;
	long     zerr;
	long     amount;
	long     size;
	long     last;

	printf("Deflating: %.*s ... ", fipnt->lfh.namelen, fipnt->spec);
	size = fipnt->lfh.size;
	last = 0;
	fipnt->lfh.method = METHOD_DEFLATE;
	if ((zerr = deflateInit2(&zstream, level, Z_DEFLATED, -15, MAX_MEM_LEVEL,
			Z_DEFAULT_STRATEGY)) != Z_OK) // windowBits is passed < 0 to
	{									  //   suppress the zlib header
		printf("? XZIP: zlib (deflateInit2) error, %d\n", zerr);
		exit(1);
	}
	zstream.next_out = outbufr;
    zstream.avail_out = OUTBUFR;
	zstream.total_in = zstream.total_out = 0;
	fipnt->lfh.crc32 = 0;
	while ((amount = svcIoInBlock(ifhndl, filebufr, FILEBUFR)) > 0)
	{
///		printf("### have %d input\n", amount);

		if (size > (2 * FILEBUFR))
		{
			rtn = ratio(size - zstream.total_in + amount, size);
			if (rtn != last)
			{
				last = rtn;
				printf("\b\b\b\b%%%02.2d ", rtn);
			}
		}
	    zstream.next_in = filebufr;
	    zstream.avail_in = amount;
		while (TRUE)
		{
			if (zstream.avail_out == 0)
			{
				if ((rtn = svcIoOutBlockP(zfhndl, outbufr, OUTBUFR,
						&zfoutparms)) < 0)
					femsg2(prgname, "Error writing data to ZIPfile", rtn, NULL);
				zfoutparms.pos.value += OUTBUFR;
	            zstream.next_out = outbufr;
    	        zstream.avail_out = OUTBUFR;
        	}
			if (zstream.avail_in == 0)
				break;
	        if ((zerr = deflate(&zstream, Z_NO_FLUSH)) != Z_OK)
			{
				printf("? XZIP: zlib (deflate) error, %d\n", zerr);
				exit(1);
			}

///			printf("### deflate (N) returned %d, ain: %d, aout: %d, tin: %d, "
///					"tout: %d\n", zerr, zstream.avail_in, zstream.avail_out,
///					zstream.total_in, zstream.total_out);
	    }
		fipnt->lfh.crc32 = crc32(fipnt->lfh.crc32, filebufr, amount);
	}
	if (amount < 0 && amount != ER_EOF)
	{
		fileerror("reading", amount);
		return (FALSE);
	}
	do
	{
///		printf("### finish: in: %d, out: %d\n", zstream.avail_in,
///				zstream.avail_out);

		if ((zerr = deflate(&zstream, Z_FINISH)) != Z_OK &&
				zerr != Z_STREAM_END)
		{
			printf("? XZIP: zlib (deflate - finish) error, %d\n", zerr);
			exit(1);
		}

///		printf("### deflate returned %d, out: %d\n", zerr, zstream.avail_out);

		if (zstream.avail_out < OUTBUFR)
		{
			amount = OUTBUFR - zstream.avail_out;
			if ((rtn = svcIoOutBlockP(zfhndl, outbufr, amount,
					&zfoutparms)) < 0)
				femsg2(prgname, "Error writing data to ZIPfile", rtn, NULL);
			zfoutparms.pos.value += amount;
            zstream.next_out = outbufr;
   	        zstream.avail_out = OUTBUFR;
       	}
	} while (zerr != Z_STREAM_END);
	fipnt->lfh.comsize = zstream.total_out;
	fipnt->lfh.size = zstream.total_in;
	if ((zerr = deflateEnd(&zstream)) != Z_OK)
	{
		printf("? XZIP: zlib (deflateEnd) error, %d\n", zerr);
		exit(1);
	}
	if (last != 0)
		fputs("\b\b\b\b... ", stdout);
	return (TRUE);
}
