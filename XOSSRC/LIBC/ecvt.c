//
// Name:
//	ecvt.c
//
// Desc:
//	Implementation of K&R/POSIX functions:
//	  ecvt(...) and fcvt(...)
//
// History:
//--------------------------------------------------------------------
// 00Jul01 CEY	o) Creation
// 00Aug02 CEY	o) Moved from \CENOP\POSIX to \XOSSRC\LIBC
//
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


#define MAX_DIGITS		30
#define PRECISION	17

static char value_str[(MAX_DIGITS+3) & ~3] ;

static double minus[] =
{
	1e-256
	, 1e-128
	, 1e-64
	, 1e-32
	, 1e-16
	, 1e-8
	, 1e-4
	, 1e-2
	, 1e-1
	, 1.0
};

static double plus[] =
{
	1e+256
	, 1e+128
	, 1e+64
	, 1e+32
	, 1e+16
	, 1e+8
	, 1e+4
	, 1e+2
	, 1e+1
};

//
// Desc:
//	Convert a floating point value(double) to an ASCII string.
//
// Inputs:
//	value		FP value to convert
//	numdigits	# digits of precision
//	dec_ptr		Pointer to int value to recieve # of decimal places
//				implied in resulting string.
//	sign_ptr	+/- indicator. 1 -> negative(-).
//
// Returns:
//	char *		Pointer to "STATIC" buffer containing ASCIZ result.
//
char *ecvt( double value, int numdigits, int *dec_ptr, int *sign_ptr )
{
	int		dptr;
	int		count, j, k ;    
	char	*v_ptr ;

	if ( numdigits < 0 )			numdigits = 0 ;
	else if( numdigits > MAX_DIGITS )	numdigits = MAX_DIGITS ;

	if( value < 0.0 )
	{
		value = -value ;
		*sign_ptr = 1 ;
	}
	else
	{
		*sign_ptr = 0 ;
	}

	if( value == 0.0 )
	{
		memset( value_str, '0', numdigits ) ;
		dptr = 0 ;
	}
	else
	{
		dptr	= 1 ;  
		k		= 256 ;  
		count	= 0 ;
		while( value < 1.0 )
		{
        	while( value < minus[count+1] )
			{
				value *= plus[count] ;
				dptr -= k ;
			}
			k /= 2 ;  
			count++ ;
		}

		k = 256 ;  
		count = 0 ;
		while( value >= 10.0 )
		{
			while( value >= plus[count] )
			{
				value *= minus[count] ;
				dptr += k ;
			}
			k /= 2 ;   
			count++ ;
		}

      for ( v_ptr = &value_str[0]; v_ptr <= &value_str[numdigits]; v_ptr++ )
      {
         if ( v_ptr >= &value_str[PRECISION] )  *v_ptr = '0' ;
         else
         {
            j = value ;
            *v_ptr = (char)(j + '0') ;		//@@@ (char)
            value = ( value - j + 1.0e-15 ) * 10.0 ;
         }
      }
      --v_ptr ;
      if ( *v_ptr >= '5' )
      {
        while (1)
        {
           if ( v_ptr == &value_str[0] )
           {
              dptr++ ;
              value_str[0] = '1' ;  
              break ;
           }
           *v_ptr = 0 ;      
           --v_ptr ;
           if ( *v_ptr != '9' )
           {
              (*v_ptr)++ ;  
              break ;
           }
        }
      }
   }
   *dec_ptr = dptr ;
   value_str[numdigits] = 0 ;
   return value_str ;
}

//
//
//
char *fcvt( double value, int numdigits, int *dec_ptr, int *sign_ptr )
{
	int			dptr;
	int			count;
	int			j, k ;    
	char		* v_ptr ;

#if 0	//###
	printf( "\n#ent fcvt: numdigits=%d val=%08X\n"
			, numdigits
			, *(long *)&value
			);
#endif

	if( numdigits < 0 )
	{
		numdigits = 0 ;
	}
	else if( numdigits > MAX_DIGITS )
	{
		numdigits = MAX_DIGITS ;
	}

	//
	// Check if negative,
	//  if so set flag 'sign_ptr' and get abs( value )
	//
	if( value < 0.0 )
	{
		value = -value ;
		* sign_ptr = 1 ;
	}
	else
	{
		* sign_ptr = 0 ;
	}


	if( value == 0.0 )
	{
		memset( value_str, '0', numdigits ) ;
		dptr = 0 ;
	}
	else
	{
		dptr	= 1 ;		// decimal ptr = 1 (default?)
		k		= 256 ;  
		count	= 0 ;

		while( value < 1.0 )
		{
			while( value < minus[count+1] )
			{
				value *= plus[count] ;
				dptr -= k ;
			}

			k /= 2 ;  
			count++ ;
		}

		k = 256 ;  
		count = 0 ;

		while( value >= 10.0 )
		{
			while( value >= plus[count] )
			{
				value *= minus[count] ;
				dptr += k ;
			}

			k /= 2 ;   
			count++ ;
		}

		numdigits += dptr;
		if( numdigits < 0 )
		{
			numdigits = 0 ;
		}
		else
		{
			if( numdigits > MAX_DIGITS )
				numdigits = MAX_DIGITS ;
		}

		for( v_ptr = &value_str[0]; v_ptr <= &value_str[numdigits]; v_ptr++ )
		{
			if( v_ptr >= &value_str[PRECISION] )
			{
				* v_ptr = '0' ;
			}
			else
			{
				j = value ;
				* v_ptr = (char)(j + '0') ;		//@@@ (char)

				value = ( value - j + 1.0e-15 ) * 10.0 ;
			}
		}

		--v_ptr ;

		if( *v_ptr >= '5' )
		{
			for( ;; )
			{
				if( v_ptr == &value_str[0] )
				{
					numdigits++ ;  
					dptr++ ;
					value_str[0] = '1' ;
					break ;
				}

				*v_ptr = 0 ;
				--v_ptr ;

				if( *v_ptr != '9' )
				{
					(*v_ptr)++ ;  
					break ;
				}
			}
		}
	}

	* dec_ptr = dptr ;
	value_str[numdigits] = 0 ;

#if 0	//###
	printf( "\n#ret fcvt: str=%-20.20s dptr=%d numdigits=%d\n"
			, value_str
			, dptr
			, numdigits
			);
#endif


	return value_str ;
}
