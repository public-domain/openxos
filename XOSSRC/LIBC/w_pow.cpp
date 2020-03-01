#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


extern "C" int _fltused_;

int _fltused_;

double pow( double /* x */, double /* y */ )
{
	return 1.0;

	//return pow( x, y );
}


