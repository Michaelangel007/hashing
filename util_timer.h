// Util Timer
// Copyleft 2016 by Michael Pohoreski
// version 6 - cleanup snprintf()
// version 5 - fix ms rounding bug
// version 4 - display milliseconds is optional
// version 3 - if nanoseconds zero, bump up to 1 to prevent division by zero
// version 2 - T and P prefix

// uint64_t
//    #include <stdint.h>
// typedef unsigned size_t uint64_t;

#ifdef _WIN32 // MSC_VER
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
    #include <Windows.h> // Windows.h -> WinDef.h defines min() max()

    /*
        typedef uint16_t WORD ;
        typedef uint32_t DWORD;

        typedef struct _FILETIME {
            DWORD dwLowDateTime;
            DWORD dwHighDateTime;
        } FILETIME;

        typedef struct _SYSTEMTIME {
              WORD wYear;
              WORD wMonth;
              WORD wDayOfWeek;
              WORD wDay;
              WORD wHour;
              WORD wMinute;
              WORD wSecond;
              WORD wMilliseconds;
        } SYSTEMTIME, *PSYSTEMTIME;
    */

    // WTF!?!? Exists in winsock2.h
    typedef struct timeval {
        long tv_sec;
        long tv_usec;
    } timeval;

    // *sigh* no gettimeofday on Win32/Win64
    int gettimeofday(struct timeval * tp, struct timezone * tzp)
    {
        // FILETIME Jan 1 1970 00:00:00
        // Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
        static const uint64_t EPOCH = ((uint64_t) 116444736000000000ULL); 

        // https://msdn.microsoft.com/en-us/library/windows/desktop/ms724950(v=vs.85).aspx
        SYSTEMTIME  system_time;
        FILETIME    file_time;
        uint64_t    time;

        GetSystemTime( &system_time );
        SystemTimeToFileTime( &system_time, &file_time );
        time =  ((uint64_t)file_time.dwLowDateTime )      ;
        time += ((uint64_t)file_time.dwHighDateTime) << 32;

        tp->tv_sec  = (long) ((time - EPOCH) / 10000000L);
        tp->tv_usec = (long) (system_time.wMilliseconds * 1000); // 1,000 milliseconds / microsecond
        return 0;
    }
#else
    #include <sys/time.h>
//  #include <time.h>
#endif // WIN32

struct DataRate
{
    char     prefix ;
    uint64_t samples;
    uint64_t per_sec;
};

struct TimeText
{
    char     day[ 16 ]; // output
    char     hms[ 12 ]; // output

    uint16_t _ms   ; // 0..999
    uint8_t  _secs ; // 0..59
    uint8_t  _mins ; // 0..59
    uint8_t  _hours; // 0..23
    uint32_t _days ; // 0..#

    void Format( double elapsed, bool bShowMilliSeconds = true )
    {
        uint64_t ms = (uint64_t)(elapsed * 1000.0);
        _ms    = ms % 1000; ms /= 1000;
        _secs  = ms %   60; ms /= 60;
        _mins  = ms %   60; ms /= 60;
        _hours = ms %   24; ms /= 24;
        _days  = (uint32_t) ms;

        day[0] = 0;
        if( _days > 0 )
            snprintf( day, sizeof(day)-1, "%d day%s, ", _days, (_days == 1) ? "" : "s" );

        int offset = sprintf( hms, "%02d:%02d:%02d", _hours, _mins, _secs );
        if( bShowMilliSeconds )
            sprintf( hms + offset, ".%03d", _ms );
    }
};

class Timer
{
    timeval start, end; // Windows: winsock2.h  Unix: sys/time.h 
public:
    double   elapsed; // total seconds

    TimeText data;
    DataRate throughput;

    void Start() {
        gettimeofday( &start, NULL );
    }

    void Stop( bool bShowMilliSeconds = true ) {
        gettimeofday( &end, NULL );
        elapsed = (end.tv_sec - start.tv_sec);

        long usec = (end.tv_usec - start.tv_usec);
        if( !usec )
             usec = 1;
        elapsed += usec / (1000. * 1000.);
        data.Format( elapsed, bShowMilliSeconds );
    }

    // size is number of bytes in a file, or number of iterations that you want to benchmark
    DataRate Throughput( uint64_t size )
    {
        DataRate datarate[] = {
            {' '}, {'K'}, {'M'}, {'G'}, {'T'}, {'P'} // 1; K=1,000; M=1,000,000; G=1,000,000,000, T=1,000,000,000,000, P=1,000,000,000,000,000
        };
        const int MAX_PREFIX = sizeof( datarate ) / sizeof( datarate[0] );

        int best = 0;
        for( int units = 0; units < MAX_PREFIX; units++ ) {
            datarate[ units ].samples  = size >> (10*units);
            datarate[ units ].per_sec = (uint64_t) (datarate[units].samples / elapsed);
            if (datarate[units].per_sec > 0)
                best = units;
        }

        throughput = datarate[ best ];
        return throughput;
    }
};
