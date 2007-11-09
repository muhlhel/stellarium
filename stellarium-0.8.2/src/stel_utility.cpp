/*
 * Stellarium
 * Copyright (C) 2002 Fabien Chereau
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */


#include <math.h> // fmod
#include <sstream>
#include <cstdlib>
#include <ctime>

#if defined( CYGWIN )
 #include <malloc.h>
#endif

#include "stel_utility.h"
#include "stellarium.h"
#include "translator.h"
#include "stellastro.h"


wstring StelUtility::stringToWstring(const string& s)
{
	return Translator::UTF8stringToWstring(s);
}


string StelUtility::wstringToString(const wstring& ws)
{
	// Get UTF-8 string length
	size_t len = wcstombs(NULL, ws.c_str(), 0)+1;
	// Create wide string
	char* s = new char[len];
	wcstombs(s, ws.c_str(), len);
	string ss(s);
	delete [] s;
	return ss;
}

wstring StelUtility::doubleToWstring(double d)
{
	std::wostringstream woss;
	woss << d;
	return woss.str();
}

wstring StelUtility::intToWstring(int i)
{
	std::wostringstream woss;
	woss << i;
	return woss.str();
}

double StelUtility::hms_to_rad( unsigned int h, unsigned int m, double s )
{
	return (double)M_PI/24.*h*2.+(double)M_PI/12.*m/60.+s*M_PI/43200.;
}

double StelUtility::dms_to_rad(int d, int m, double s)
{
	return (double)M_PI/180.*d+(double)M_PI/10800.*m+s*M_PI/648000.;
}

double hms_to_rad(unsigned int h, double m)
{
	return (double)M_PI/24.*h*2.+(double)M_PI/12.*m/60.;
}

double dms_to_rad(int d, double m)
{
	double t = (double)M_PI/180.*d+(double)M_PI/10800.*m;
	return t;
}


void sphe_to_rect(double lng, double lat, Vec3d& v)
{
	const double cosLat = cos(lat);
	v.set(cos(lng) * cosLat, sin(lng) * cosLat, sin(lat));
}

void sphe_to_rect(double lng, double lat, double r, Vec3d& v)
{
	const double cosLat = cos(lat);
	v.set(cos(lng) * cosLat * r, sin(lng) * cosLat * r, sin(lat) * r);
}

void sphe_to_rect(float lng, float lat, Vec3f& v)
{
	const double cosLat = cos(lat);
	v.set(cos(lng) * cosLat, sin(lng) * cosLat, sin(lat));
}

void rect_to_sphe(double *lng, double *lat, const Vec3d& v)
{
	double r = v.length();
	*lat = asin(v[2]/r);
	*lng = atan2(v[1],v[0]);
}

void rect_to_sphe(float *lng, float *lat, const Vec3f& v)
{
	double r = v.length();
	*lat = asin(v[2]/r);
	*lng = atan2(v[1],v[0]);
}


// Obtains a Vec3f from a string with the form x,y,z
Vec3f StelUtility::str_to_vec3f(const string& s)
{
	float x, y, z;
	if (s.empty() || (sscanf(s.c_str(),"%f,%f,%f",&x, &y, &z)!=3)) return Vec3f(0.f,0.f,0.f);
	return Vec3f(x,y,z);
}

// Obtains a string from a Vec3f with the form x,y,z
string StelUtility::vec3f_to_str(const Vec3f& v)
{
	ostringstream os;
	os << v[0] << "," << v[1] << "," << v[2];
	return os.str();
}

// Provide the luminance in cd/m^2 from the magnitude and the surface in arcmin^2
float mag_to_luminance(float mag, float surface)
{
	return expf(-0.4f * 2.3025851f * (mag - (-2.5f * log10f(surface)))) * 108064.73f;
}

// strips trailing whitespaces from buf.
#define iswhite(c)  ((c)== ' ' || (c)=='\t')
static char *trim(char *x)
{
	char *y;

	if(!x)
		return(x);
	y = x + strlen(x)-1;
	while (y >= x && iswhite(*y))
		*y-- = 0; /* skip white space */
	return x;
}



// salta espacios en blanco
static void skipwhite(char **s)
{
	while(iswhite(**s))
		++(*s);
}


double get_dec_angle(const string& str)
{
	const char* s = str.c_str();
	char *mptr, *ptr, *dec, *hh;
	int negative = 0;
	char delim1[] = " :.,;DdHhMm'\n\t\xBA";  // 0xBA was old degree delimiter
	char delim2[] = " NSEWnsew\"\n\t";
	int dghh = 0, minutes = 0;
	double seconds = 0.0, pos;
	short count;

	enum _type{
	    HOURS, DEGREES, LAT, LONG
	}type;

	if (s == NULL || !*s)
		return(-0.0);
	count = strlen(s) + 1;
	if ((mptr = (char *) malloc(count)) == NULL)
		return (-0.0);
	ptr = mptr;
	memcpy(ptr, s, count);
	trim(ptr);
	skipwhite(&ptr);

	/* the last letter has precedence over the sign */
	if (strpbrk(ptr,"SsWw") != NULL)
		negative = 1;

	if (*ptr == '+' || *ptr == '-')
		negative = (char) (*ptr++ == '-' ? 1 : negative);
	skipwhite(&ptr);
	if ((hh = strpbrk(ptr,"Hh")) != NULL && hh < ptr + 3)
		type = HOURS;
	else
		if (strpbrk(ptr,"SsNn") != NULL)
			type = LAT;
		else
			type = DEGREES; /* unspecified, the caller must control it */

	if ((ptr = strtok(ptr,delim1)) != NULL)
		dghh = atoi (ptr);
	else
	{
		free(mptr);
		return (-0.0);
	}

	if ((ptr = strtok(NULL,delim1)) != NULL)
	{
		minutes = atoi (ptr);
		if (minutes > 59)
		{
			free(mptr);
			return (-0.0);
		}
	}
	else
	{
		free(mptr);
		return (-0.0);
	}

	if ((ptr = strtok(NULL,delim2)) != NULL)
	{
		if ((dec = strchr(ptr,',')) != NULL)
			*dec = '.';
		seconds = strtod (ptr, NULL);
		if (seconds >= 60.0)
		{
			free(mptr);
			return (-0.0);
		}
	}

	if ((ptr = strtok(NULL," \n\t")) != NULL)
	{
		skipwhite(&ptr);
		if (*ptr == 'S' || *ptr == 'W' || *ptr == 's' || *ptr == 'w') negative = 1;
	}

	free(mptr);

	pos = ((dghh*60+minutes)*60 + seconds) / 3600.0;
	if (type == HOURS && pos > 24.0)
		return (-0.0);
	if (type == LAT && pos > 90.0)
		return (-0.0);
	else
		if (pos > 180.0)
			return (-0.0);

	if (negative)
		pos = -pos;

	return (pos);

}




//! @brief Print the passed angle with the format ddÃƒÂ‚Ã‚Â°mm'ss(.ss)"
//! @param angle Angle in radian
//! @param decimal Define if 2 decimal must also be printed
//! @param useD Define if letter "d" must be used instead of Â°
//! @return The corresponding string
wstring StelUtility::printAngleDMS(double angle, bool decimals, bool useD)
{
    wchar_t buf[32];
    buf[31]=L'\0';
    wchar_t sign = L'+';
    // wchar_t degsign = L'Â°'; ???
    wchar_t degsign = L'\u00B0';
    if (useD) degsign = L'd';

    angle *= 180./M_PI;

    if (angle<0) {
        angle *= -1;
        sign = '-';
    }

    if (decimals) {
        int d = (int)(0.5+angle*(60*60*100));
        const int centi = d % 100;
        d /= 100;
        const int s = d % 60;
        d /= 60;
        const int m = d % 60;
        d /= 60;
        swprintf(buf,
#ifndef MINGW32
                 sizeof(buf),
#endif
                 L"%lc%.2d%lc%.2d'%.2d.%02d\"",
                 sign, d, degsign, m, s, centi);
    } else {
        int d = (int)(0.5+angle*(60*60));
        const int s = d % 60;
        d /= 60;
        const int m = d % 60;
        d /= 60;
        swprintf(buf,
#ifndef MINGW32
                 sizeof(buf),
#endif
                 L"%lc%.2d%lc%.2d'%.2d\"",
                 sign, d, degsign, m, s);
    }
    return buf;
}

//! @brief Print the passed angle with the format +hhhmmmss(.ss)"
//! @param angle Angle in radian
//! @param decimals Define if 2 decimal must also be printed
//! @return The corresponding string
wstring StelUtility::printAngleHMS(double angle, bool decimals)
{
    wchar_t buf[16];
    buf[15] = L'\0';
    angle = fmod(angle,2.0*M_PI);
    if (angle < 0.0) angle += 2.0*M_PI; // range: [0..2.0*M_PI)
    angle *= 12./M_PI; // range: [0..24)
    if (decimals) {
        angle = 0.5+angle*(60*60*100); // range:[0.5,24*60*60*100+0.5)
        if (angle >= (24*60*60*100)) angle -= (24*60*60*100);
        int h = (int)angle;
        const int centi = h % 100;
        h /= 100;
        const int s = h % 60;
        h /= 60;
        const int m = h % 60;
        h /= 60;
        swprintf(buf,
#ifndef MINGW32
                 sizeof(buf),
#endif
                 L"%.2dh%.2dm%.2d.%02ds",h,m,s,centi);
    } else {
        angle = 0.5+angle*(60*60); // range:[0.5,24*60*60+0.5)
        if (angle >= (24*60*60)) angle -= (24*60*60);
        int h = (int)angle;
        const int s = h % 60;
        h /= 60;
        const int m = h % 60;
        h /= 60;
        swprintf(buf,
#ifndef MINGW32
                 sizeof(buf),
#endif
                 L"%.2dh%.2dm%.2ds",h,m,s);
    }
    return buf;
}


// convert string int ISO 8601-like format [+/-]YYYY-MM-DDThh:mm:ss (no timzone offset)
// to julian day

int string_to_jday(string date, double &jd)
{
	char tmp;
	int year, month, day, hour, minute, second;
	year = month = day = hour = minute = second = 0;

	std::istringstream dstr( date );

	// TODO better error checking
	dstr >> year >> tmp >> month >> tmp >> day >> tmp >> hour >> tmp >> minute >> tmp >> second;

	// cout << year << " " << month << " " << day << " " << hour << " " << minute << " " << second << endl;

	// bounds checking (per s_tui time object)
	if( year > 100000 || year < -100000 ||
	        month < 1 || month > 12 ||
	        day < 1 || day > 31 ||
	        hour < 0 || hour > 23 ||
	        minute < 0 || minute > 59 ||
	        second < 0 || second > 59) return 0;


	// code taken from s_tui.cpp
	if (month <= 2)
	{
		year--;
		month += 12;
	}

	// Correct for the lost days in Oct 1582 when the Gregorian calendar
	// replaced the Julian calendar.
	int B = -2;
	if (year > 1582 || (year == 1582 && (month > 10 || (month == 10 && day >= 15))))
	{
		B = year / 400 - year / 100;
	}

	jd = ((floor(365.25 * year) +
	       floor(30.6001 * (month + 1)) + B + 1720996.5 +
	       day + hour / 24.0 + minute / 1440.0 + second / 86400.0));

	return 1;

}


double str_to_double(string str)
{

	if(str=="") return 0;
	double dbl;
	std::istringstream dstr( str );

	dstr >> dbl;
	return dbl;
}

// always positive
double str_to_pos_double(string str)
{

	if(str=="") return 0;
	double dbl;
	std::istringstream dstr( str );

	dstr >> dbl;
	if(dbl < 0 ) dbl *= -1;
	return dbl;
}


int str_to_int(string str)
{

	if(str=="") return 0;
	int integer;
	std::istringstream istr( str );

	istr >> integer;
	return integer;
}


int str_to_int(string str, int default_value)
{

	if(str=="") return default_value;
	int integer;
	std::istringstream istr( str );

	istr >> integer;
	return integer;
}

string double_to_str(double dbl)
{

	std::ostringstream oss;
	oss << dbl;
	return oss.str();

}

long int str_to_long(string str)
{

	if(str=="") return 0;
	long int integer;
	std::istringstream istr( str );

	istr >> integer;
	return integer;
}

int fcompare(const string& _base, const string& _sub)
{
	unsigned int i = 0;
	while (i < _sub.length())
	{
		if (toupper(_base[i]) == toupper(_sub[i])) i++;
		else return -1;
	}
	return 0;
}

int fcompare(const wstring& _base, const wstring& _sub)
{
	unsigned int i = 0;
	while (i < _sub.length())
	{
		if (toupper(_base[i]) == toupper(_sub[i])) i++;
		else return -1;
	}
	return 0;
}


// Return the time zone name taken from system locale
wstring StelUtility::get_time_zone_name_from_system(double JD)
{
	// Windows will crash if date before 1970
	// And no changes on Linux before that year either
	// TODO: ALSO, on Win XP timezone never changes anyway??? 
	if(JD < 2440588 ) JD = 2440588;

	// The timezone name depends on the day because of the summer time
	time_t rawtime = get_time_t_from_julian(JD);

	struct tm * timeinfo;
	timeinfo = localtime(&rawtime);
	static char timez[255];
	timez[0] = 0;
	StelUtility::my_strftime(timez, 254, "%Z", timeinfo);
	return StelUtility::stringToWstring(timez);
}


// Return the number of hours to add to gmt time to get the local time in day JD
// taking the parameters from system. This takes into account the daylight saving
// time if there is. (positive for Est of GMT)
// TODO : %z in strftime only works on GNU compiler
// Fixed 31-05-2004 Now use the extern variables set by tzset()
float StelUtility::get_GMT_shift_from_system(double JD, bool _local)
{
	/* Doesn't seem like MACOSX is a special case... ??? rob
    #if defined( MACOSX ) || defined(WIN32)
	struct tm *timeinfo;
	time_t rawtime; time(&rawtime);
	timeinfo = localtime(&rawtime);
	return (float)timeinfo->tm_gmtoff/3600 + (timeinfo->tm_isdst!=0); 
	#else */

#if !defined(MINGW32)

	struct tm * timeinfo;

	if(!_local)
	{
		// JD is UTC
		struct tm rawtime;
		get_tm_from_julian(JD, &rawtime);
		
#ifdef HAVE_TIMEGM
		time_t ltime = timegm(&rawtime);
#else
		// This does not work
		time_t ltime = my_timegm(&rawtime);
#endif
		
		timeinfo = localtime(&ltime);
	} else {
	  time_t rtime;
	  rtime = get_time_t_from_julian(JD);
	  timeinfo = localtime(&rtime);
	}

	static char heure[20];
	heure[0] = '\0';

	my_strftime(heure, 19, "%z", timeinfo);
	//	cout << heure << endl;

	//cout << timezone << endl;
	
	heure[5] = '\0';
	float min = 1.f/60.f * atoi(&heure[3]);
	heure[3] = '\0';
	return min + atoi(heure);
#else
     struct tm *timeinfo;
     time_t rawtime;
     time(&rawtime);
     timeinfo = localtime(&rawtime);
     return -(float)timezone/3600 + (timeinfo->tm_isdst!=0);
#endif
}

// Return the time in ISO 8601 format that is : %Y-%m-%d %H:%M:%S
string StelUtility::get_ISO8601_time_UTC(double JD)
{
	struct tm time_utc;
	get_tm_from_julian(JD, &time_utc);

	static char isotime[255];
	my_strftime(isotime, 254, "%Y-%m-%d %H:%M:%S", &time_utc);
	return isotime;
}
