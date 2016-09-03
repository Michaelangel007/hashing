// Includes
    #include <stdio.h>  // printf()
    #include <string.h> // strlen()
    #include <stdlib.h> // atoi()
    #include <stdint.h> // uint16_t

    #include "hash16.h"
    #include "util_itoa.h"
    #include "util_log2.h"
    #include "util_timer.h"

// Typedefs

    //#define INT unsigned short // 7.863s
    //#defeme INT int            // 7.686s
    //#define INT int_fast16_t   // 7.826s
      #define INT int_fast32_t   // 7.678s, 7.694s, 7.788s

// Globals

    // wordlist 1
    const char *aCommands[] =
    {
          "INIT"
        , "LOAD"
        , "SAVE"
        , "RUN"
        , "CHAIN"
        , "DELETE"
        , "LOCK"
        , "UNLOCK"
        , "CLOSE"
        , "READ"
        , "EXEC"
        , "WRITE"
        , "POSITION"
        , "OPEN"
        , "APPEND"
        , "RENAME"
        , "CATALOG"
        , "MON"
        , "NOMON"
        , "PR#"
        , "IN#"
        , "MAXFILES"
        , "FP"
        , "INT"
        , "BSAVE"
        , "BLOAD"
        , "BRUN"
        , "VERIFY"
    };
    const int nCommands = sizeof( aCommands ) / sizeof( const char * );
    /* */ int aHashes[ nCommands ];

    struct KeyVal_t
    {
        int key;
        int val;
    };

    int      nSorted; // log2( nCommands ) + 1
    KeyVal_t aSorted[ nCommands+1 ];

    bool bVerbose = 0;

    // Default to ASCII [Ctrl-A ... ~]
    // [Begin,End)
    int gBegin = 0x00;
    int gEnd   = 0x80;

void dump_sort()
{
    const int n = nCommands;

    for( int i= 0; i <  n; i++ )
        printf( "[%2d] %04X: %s\n", i, aSorted[i].key, aCommands[ aSorted[i].val ] );
}


// ========================================================================
void init_hash()
{
    size_t len;
    int    hash;
    const char *text;

    const int n = nCommands;

    for( int iCommand = 0; iCommand < nCommands; iCommand++ )
    {
        text = aCommands[ iCommand ];
        len  = strlen( text );
        hash = hash16( len, text );

        aHashes[ iCommand ] = hash;
//        printf( "[%2d] %04X: %s\n", iCommand, hash, text );
    }

    // Need to sort hashes so find_hash() can use binary search
    for( int i = 0; i < n; i++ )
    {
        aSorted[ i ].key = aHashes[ i ];
        aSorted[ i ].val = i;
    }

    if( bVerbose )
        dump_sort();

    for( int i = 0; i < n-1; i++ )
    {
        if( aSorted[i].key > aSorted[i+1].key )
        {
            aSorted[ n ] = aSorted[ i ];
                           aSorted[ i ] = aSorted[i+1];
                                          aSorted[i+1] = aSorted[ n ];
            i = -1;
        }
    }

    if( bVerbose )
        dump_sort();

    nSorted  = log2_ceil( nCommands );
}

// ========================================================================
int find_hash( const int hash )
{
#if 0 // Linear Search
    for( int iCommand = 0; iCommand < nCommands; iCommand++ )
        if( aHashes[ iCommand ] == hash )
            return iCommand + 1;
#else // Binary Search
    int min = 0;
    int mid;
    int max = nCommands - 1;

    for( int i = 0; i < nSorted; i++ )
    {
        mid = (min + max) / 2;
        /**/ if( aSorted[ mid ].key == hash ) return aSorted[ mid ].val + 1;
        else if( aSorted[ mid ].key >= hash)  max = mid;
        else /*  aSorted[ mid ].key <  hash*/ min = mid;
    }
#endif

    return 0;
}

// ========================================================================
void control( char c, char *buffer )
{
    int len = 0;
    if (c < 32)
    {
        buffer[ len++ ] = '^';
        buffer[ len++ ] = '@' + (c & 0x3F);
    }
    else
    {
        buffer[ len++ ] =  c;
        buffer[ len++ ] = ' ';
    }

    buffer[ len ] = 0;
}


// ========================================================================
const char* ctrl1( char c )
{
    static char text[ 4 ];

    control( c, text );
    return text;
}

// ========================================================================
const char* ctrl2( char c )
{
    static char text[ 4 ];

    control( c, text );
    return text;
}


void collide( const size_t len, const char *buffer, int hash, int find )
{
    if( !bVerbose )
        return;

    printf( "%04X: %s%s (%02X,%02X) == %04X: %s\n"
        , hash
        , ctrl2( buffer[ 1 ] )
        , ctrl1( buffer[ 0 ] )
        , buffer[0]
        , buffer[1]
        , aHashes[ find ]
        , aCommands[ find ]
    );

}


// ========================================================================
void print_collisions( size_t len, int collisions )
{
    char *num = itoa_comma( (size_t) collisions );
    // '2x: Collisions: '
    //  0123456789ABCDEF
    //      ^   ^   ^   ^
    printf( "%dx: Collisions: %s\n", (int) len, num );

}


// ========================================================================
void test_2()
{
    size_t LEN = 2;
    int    hash;
    int    find;

    char buffer[ 16 ];
    const char *text = buffer;

    int collisions = 0;

    for( int i = 0;i < 16; i++ )
        buffer[i] = 0;

    INT a, b;

    for( a = gBegin; a < gEnd; a++ )
    {
        buffer[0] = a;

        for( b = gBegin; b < gEnd; b++ )
        {
            buffer[1] = b;

// ----------
            hash = hash16( LEN, text );
            find = find_hash( hash );
            if( find )
            {
                find--;
                collide( LEN, buffer, hash, find );
                collisions++;
            }
// ----------
        } // b
    } // a

    print_collisions( LEN, collisions );
}


// ========================================================================
void test_3()
{
    size_t LEN = 3;
    int    hash;
    int    find;

    char buffer[ 16 ];
    const char *text = buffer;

    int collisions = 0;

    for( int i = 0;i < 16; i++ )
        buffer[i] = 0;

    INT a, b, c;

    for( a = gBegin; a < gEnd; a++ )
    {
        buffer[0] = a;

        for( b = gBegin; b < gEnd; b++ )
        {
            buffer[1] = b;


            for( c = gBegin; c < gEnd; c++ )
            {
                buffer[2] = c;

// ----------
                hash = hash16( LEN, text );
                find = find_hash( hash );
                if( find )
                {
                    find--;
                    collide( LEN, buffer, hash, find );
                    collisions++;
                }
// ----------
            } // c
        } // b
    } // a

    print_collisions( LEN, collisions );
}


// ========================================================================
void test_4()
{
    size_t LEN = 4;
    int    hash;
    int    find;

    char buffer[ 16 ];
    const char *text = buffer;

    int collisions = 0;

    for( int i = 0;i < 16; i++ )
        buffer[i] = 0;

    INT a, b, c, d;

    for( a = gBegin; a < gEnd; a++ )
    {
        buffer[0] = a;

        for( b = gBegin; b < gEnd; b++ )
        {
            buffer[1] = b;


            for( c = gBegin; c < gEnd; c++ )
            {
                buffer[2] = c;

                for( d = gBegin; d < gEnd; d++ )
                {
                    buffer[3] = d;
// ----------
                    hash = hash16( LEN, text );
                    find = find_hash( hash );
                    if( find )
                    {
                        find--;
                        collide( LEN, buffer, hash, find );
                        collisions++;
                    }
// ----------
                } // d
            } // c
        } // b
    } // a

    print_collisions( LEN, collisions );
}

// ========================================================================
void test_5()
{
    size_t LEN = 5;
    int    hash;
    int    find;

    char buffer[ 16 ];
    const char *text = buffer;

    int collisions = 0;

    for( int i = 0;i < 16; i++ )
        buffer[i] = 0;

    INT a, b, c, d, e;

    for( a = gBegin; a < gEnd; a++ )
    {
        buffer[0] = a;

        for( b = gBegin; b < gEnd; b++ )
        {
            buffer[1] = b;


            for( c = gBegin; c < gEnd; c++ )
            {
                buffer[2] = c;

                for( d = gBegin; d < gEnd; d++ )
                {
                    buffer[3] = d;

                    for( e = gBegin; e < gEnd; e++ )
                    {
                        buffer[4] = e;
// ----------
                        hash = hash16( LEN, text );
                        find = find_hash( hash );
                        if( find )
                        {
                            find--;
                            collide( LEN, buffer, hash, find );
                            collisions++;
                        }
// ----------
                    } // e
                } // d
            } // c
        } // b
    } // a

    print_collisions( LEN, collisions );
}

// if Arg
//    is char in range ' ' .. '~'
// ========================================================================
bool getRange( const char *aArg, int * n_ )
{
    const char  *p = aArg;
    const size_t n = strlen( aArg );

    int val = 0;

    if ((n > 1) && (n <= 4)) // check for hex char: 0x? .. 0x??
    {
        if ((p[0] == '0') && ((p[1] == 'x') || (p[1] == 'X')))
        {
            for( p += 2; *p; p++ )
            {
                val <<= 4;
                if ((*p >= '0') && (*p <= '9'))
                    val |= (*p & 0xF);
                else
                if ((*p >= 'A') && (*p <= 'F'))
                    val |= (*p - 'A' + 10);
                else
                if ((*p >= 'a') && (*p <= 'f'))
                    val |= (*p - 'a' + 10);
            }
printf( "%X\n", val );
            *n_ = val;
            return true;
        }
    }
    else
    if ((*p > ' ') && (*p <= '~'))
    {
        *n_ = *p;
        return true;
    }

    return false;
}

// ========================================================================
int main( const int nArg, const char *aArg[] )
{
    int iArg;
    int last = 4;

    if (nArg > 1)
    for( iArg = 1; iArg < nArg; iArg++ )
    {
        if (strcmp( aArg[ iArg ], "-v" ) == 0)
            bVerbose = true;
        else
        if (strcmp( aArg[ iArg ], "-b" ) == 0)
        {
            iArg++;
            if( iArg < nArg)
                getRange( aArg[ iArg ], &gBegin );
        }
        else
        if (strcmp( aArg[ iArg ], "-e" ) == 0)
        {
            iArg++;
            if( iArg < nArg)
                if( getRange( aArg[ iArg ], &gEnd ) )
                    gEnd++;
        }
        else
        if (strcmp( aArg[ iArg ], "-up" ) == 0)
        {
            gBegin = 'A';
            gEnd   = 'Z'+1;
        }
        else
        if (strcmp( aArg[ iArg ], "-AZ" ) == 0)
        {
            gBegin = 'A';
            gEnd   = 'Z'+1;
        }
        else
        if (strcmp( aArg[ iArg ], "-Az" ) == 0)
        {
            gBegin = 'A';
            gEnd   = 'z'+1;
        }
        else
        if (strcmp( aArg[ iArg ], "-print" ) == 0) // printables
        {
            gBegin = ' ';
            gEnd   = '~'+1;
        }
        else
        if (strcmp( aArg[ iArg ], "-ascii" ) == 0)
        {
            gBegin = 0x00; // Ctrl-@
            gEnd   = 0x80; // 7E ~, 7F <del>
        }
        else
        if (strcmp( aArg[ iArg ], "-byte" ) == 0)
        {
            gBegin = 0x00; // Ctrl-@
            gEnd   =0x100; // 0xFF
        }
        else
            last = atoi( aArg[ iArg ] );
    }

    init_hash();

printf( "Begin: %02X %c%c\n", gBegin, (gBegin < 0x20) ? '^' : gBegin, (gBegin < 0x20) ? ('@' + gBegin) : ' ' );
printf( "End  : %02X %c%c\n", gEnd-1, (gEnd-1 < 0x20) ? '^' : gEnd-1, (gEnd-1 < 0x20) ? ('@' + gEnd-1) : ' ' );

    Timer timer;
    timer.Start();

    if( last >= 2 ) test_2(); bVerbose = 0;
    if( last >= 3 ) test_3();
    if( last >= 4 ) test_4();
    if( last >= 5 ) test_5();

    timer.Stop( false ); // no need for milliseconds accuracy
    printf( "%s%s\n", timer.data.day, timer.data.hms );

    return 0;
}

