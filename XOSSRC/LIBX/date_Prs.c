//-------------------------------------------------------------------------
// DATE_PRS
// A date and time parser utility
// Acceptable date formats are:
//	US		mm/dd/[[yy]yy]
//	INTL	dd.mm.[[yy]yy]
//	TEXT	dd[-]mmm[-][[yy]yy]  or
//			mmm[-]dd[-][[yy]yy]
//	PACK	yymmdd (all six values must be present)
//	TIME	hh:mm[:ss][A|P]
//
// note: 	If the year is not included, current year is used
//			if century is not included, current century is used
//
// Written by: SA Ortmann
//
// Edit History:
// 02/08/95(sao) - Initial development
// 03/17/95(sao) - Modified to accept date and/or time and XOS time_s
// 05/26/95(sao) - Added '@' as date/time seperator for fpj
//-------------------------------------------------------------------------

// ++++
// This software is in the public domain.  It may be freely copied and used
// for whatever purpose you see fit, including commerical uses.  Anyone
// modifying this software may claim ownership of the modifications, but not
// the complete derived code.  It would be appreciated if the authors were
// told what this software is being used for, but this is not a requirement.

//   THIS SOFTWARE IS PROVIDED BY THE AUTHORS "AS IS" AND ANY EXPRESS OR
//   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
//   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
//   IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//   INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
//   BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
//   OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
//   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
//   TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
//   USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//----

#include <STDIO.H>
#include <STDLIB.H>
#include <CTYPE.H>
#include <STRING.H>
#include <XOSSVC.H>
#include <XOSTIME.H>
#include <XOSSTR.H>

// Error text

char *dtPrsErr[] =
{   "Invalid Date Format",
    "Invalid Month",
    "Invalid Day",
    "Date out of range",
    "Minutes out of range",
    "Hours out of range",
    "Seconds out of range",
    "Invalid time format",
    ""
};

// Local Function prototypes

static int dt_vTime(time_d *ds);
static int Dt_vDate(time_d *ds);
static int dt_USParse(char *s, time_d *ds);
static int dt_INTLParse(char *s, time_d *ds);
static int dt_TXTParse(char *s, time_d *ds);
static int dt_TIMEParse(char *s, time_d *ds);
static int dt_PACKParse(char *s, time_d *ds);


static int dt_vDate(
    time_d *ds)

{
    time_x tswap;

    if (ds->tmx_mday < 1 || ds->tmx_mday > 31)
	return (INVDAY);
    if (ds->tmx_mon < 1 || ds->tmx_mon > 12)
	return (INVMONTH);
    if (ds->tmx_year < 100)
	ds->tmx_year += (ds->tmx_year >= 80) ? 1900 : 2000;
    tswap.dos = *ds;
    if (svcSysDateTime(T_CVD2X, &tswap) != 0)
	return (INVDTFORMAT);
    svcSysDateTime(T_CVX2D, &tswap);
    return ((tswap.dos.tmx_year != ds->tmx_year ||
	    tswap.dos.tmx_mon != ds->tmx_mon ||
	    tswap.dos.tmx_mday != ds->tmx_mday) ? INVDTRANGE : 0);
}

static int dt_vTime(
    time_d *ds)

{
    if (ds->tmx_min > 59)
	return (INVTMMIN);
    if (ds->tmx_sec > 59)
	return (INVTMSECOND);
    if (ds->tmx_hour > 23)
	return (INVTMHOUR);
    return (0);
}

static int dt_USParse(
    char   *s,
    time_d *ds)

{
    char *temp;
    char *pT;

    temp = s;
    ds->tmx_mon = ds->tmx_mday = ds->tmx_year = 0;
    while (*temp != '\0')
	if (!strchr("0123456789/", *temp))
	    return (INVDTFORMAT);
	else
	    temp++;
    pT = strtok(s, "/");
    if (pT == NULL)
	return (INVDTFORMAT);
    ds->tmx_mon = atoi(pT);
    pT = strtok(NULL, "/");
    if (pT == NULL)
	return (INVDTFORMAT);
    ds->tmx_mday = atoi(pT);
    pT = strtok(NULL, "/");
    if (pT != NULL)
	ds->tmx_year = atoi(pT);
    return (dt_vDate(ds));
}

static int dt_INTLParse(
    char   *s,
    time_d *ds)

{
    char *temp=s;
    char *pT;

    ds->tmx_mon = ds->tmx_mday = ds->tmx_year = 0;
    while (*temp != '\0')
	if (!strchr("0123456789.", *temp))
	    return (INVDTFORMAT);
	else
	    temp++;
    pT = strtok(s, ".");
    if (pT == NULL)
	return (INVDTFORMAT);
    ds->tmx_mday = atoi(pT);
    pT = strtok(NULL, ".");
    if (pT == NULL)
	return (INVDTFORMAT);
    ds->tmx_mon = atoi(pT);
    pT = strtok(NULL, ".");
    if (pT != NULL)
	ds->tmx_year = atoi(pT);
    return (dt_vDate(ds));
}

static int dt_TXTParse(
    char   *s,
    time_d *ds)

{
    char *pC;
    char *pDay;
    char *pMo;
    char *pMP;
    char *pYr;
    int   day;
    char  chr;
    char  cbDay[20];
    char  cbMo[20];
    char  cbYr[20];

    static char Months[40]={"JANFEBMARAPRMAYJUNJULAUGSEPOCTNOVDEC"};

    pC = s;
    pDay = cbDay;
    pMo = cbMo;
    pYr = cbYr;
    day = TRUE;
    ds->tmx_mon = ds->tmx_mday = ds->tmx_year = 0;
    strupper(pC);
    while ((chr = *pC++) != '\0')
    {
	if (chr == '-')
	    day = FALSE;
	else
	{
	    if (isdigit(chr))
	    {
		if (day)
		    *pDay++ = chr;
		else
		    *pYr++ = chr;
	    }
	    else
		*pMo++ = chr;
	}
    }
    *pMo = '\0';
    *pYr = '\0';
    *pDay = '\0';
    pMP = strstr(Months, cbMo);
    if (pMP == NULL || (pMP - Months) % 3 != 0)
	return (INVMONTH);
    ds->tmx_mon = ((pMP - Months)/3) + 1;
    ds->tmx_mday = atoi(cbDay);
    ds->tmx_year = atoi(cbYr);
    return (dt_vDate(ds));
}

static int dt_TIMEParse(
    char   *s,
    time_d *ds)

{
    char *pC;
    char  cbHour[20];
    char  cbMin[20];
    char  cbSec[20];
    char *pHour;
    char *pMin;
    char *pSec;
    int   hour;
    int   min;
    int   sec;
    int   pm;
    int   am;
    int   atEnd;

    pC = s;
    pHour = cbHour;
    pMin = cbMin;
    pSec = cbSec;
    hour = min = sec = 1;
    pm = am = FALSE;
    atEnd = FALSE;

    ds->tmx_min = ds->tmx_sec = ds->tmx_hour = 0;
    while (*pC != '\0')
    {
	if (atEnd)
	    return (INVTMFORMAT);

	if (*pC == ':')
	{
	    if (hour)
		hour = FALSE;
	    else if (min)
		min = FALSE;
	    else
		return (INVTMFORMAT);
	    pC++;
	    continue;
	}
	if (isdigit(*pC))
	{
	    if (hour)
	    {
		*pHour = *pC;
		pHour++;
	    }
	    else
	    {
		if (min)
		{
		    *pMin = *pC;
		    pMin++;
		}
		else
		{
		    *pSec=*pC;
		    pSec++;
		}
	    }
	}
	else
	{
	    if (min || sec && (*pC == 'a' || *pC == 'A' || *pC == 'p' ||
		    *pC == 'P'))
	    {
		if (*pC == 'p' || *pC == 'P')
		    pm = TRUE;
		else
		    am = TRUE;
		atEnd = TRUE;
	    }
	    else
		return (INVTMFORMAT);
	}
	pC++;
    }
    *pSec = '\0';
    *pMin = '\0';
    *pHour = '\0';
    ds->tmx_min = atoi(cbMin);
    ds->tmx_sec = atoi(cbSec);
    ds->tmx_hour = atoi(cbHour);
    if (pm && ds->tmx_hour != 12)
	ds->tmx_hour += 12;
    if (am && ds->tmx_hour == 12)
	ds->tmx_hour = 0;
    return dt_vTime(ds);
}

static int dt_PACKParse(
    char   *s,
    time_d *ds)

{
    char *temp;
    char  m[3];
    char  d[3];
    char  y[3];

    temp = s;
    ds->tmx_mon = ds->tmx_mday = ds->tmx_year = 0;
    if (strlen(s) != 6)
	return (INVDTFORMAT);

    while (*temp != '\0')
	if (!strchr("0123456789", *temp))
	    return (INVDTFORMAT);
	else
	    temp++;
    strncpy(y, s, 2);
    strncpy(m, s + 2, 2);
    strncpy(d, s + 4, 2);
    m[2] = '\0';
    d[2] = '\0';
    y[2] = '\0';
    ds->tmx_mday = atoi(d);
    ds->tmx_mon = atoi(m);
    ds->tmx_year = atoi(y);
    return (dt_vDate(ds));
}

int dt_parse(
    char *inp_str,
    time_s *dt)

{
    time_d ds;
    time_x dtx;
    char   tbuffer[80];
    char   v_buf[40];
    char  *temp;
    int    rv;
    char   f_time;
    char   f_date;
    char   OK;

    temp = NULL;
    f_time = f_date = FALSE;
    OK = TRUE;
    svcSysDateTime(T_GTXDTTM, dt);
    dt->time = 0;
    strcpy(tbuffer, inp_str);
    rv = INVDTFORMAT;
    while (OK)
    {
        temp = strtok(tbuffer, " ,@");
        if (temp != NULL)
        {
            strcpy(v_buf, temp);
            temp = strtok(NULL, " ,@");
            strcpy(tbuffer, (temp != NULL) ? temp : "");
        }
        else
        {
            OK = FALSE;
            continue;
        }

        if (OK)
        {
            temp = v_buf;
            while (isdigit(*temp) && *temp != '\0')
                temp++;
            switch (*temp)
            {
	     case '/' :
		rv = dt_USParse(v_buf, &ds);
		f_date = TRUE;
		break;

	     case '.' :
		rv = dt_INTLParse(v_buf, &ds);
		f_date = TRUE;
		break;

	     case '-' :
		rv = dt_TXTParse(v_buf, &ds);
		f_date = TRUE;
		break;

	     case ':' :
		rv = dt_TIMEParse(v_buf, &ds);
		f_time = TRUE;
		break;

	     case '\0' :
		rv = dt_PACKParse(v_buf, &ds);
		f_date = TRUE;
		break;

	     default:
		if (strchr("JFMASONDjfmasond", *temp) != NULL)
		{
		    rv = dt_TXTParse(v_buf, &ds);
		    f_date = TRUE;
		}
		else
		    return (rv);
		break;
	    }

	    if (rv != 0)
		return (rv);

	    if (f_time)
	    {
		f_time = FALSE;
		dtx.dos = ds;
		if (svcSysDateTime(T_CVD2X, &dtx) != 0)
                    return (INVDTFORMAT);
                dt->time = dtx.sys.time;
            }
	    else
	    {
		f_date = FALSE;
		dtx.dos = ds;
		if (svcSysDateTime(T_CVD2X, &dtx) != 0)
                    return (INVDTFORMAT);
		dt->date = dtx.sys.date;
	    }
	}
    }
    return (rv);
}
