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

    // wordlist 2 -- read wordlist.txt
    char *pWords = NULL;

    int    nCommands = 0;
    char **aCommands = NULL; // array of pointers
    char  *aLengths  = NULL;
    int   *aHashes   = NULL;

    struct KeyVal_t
    {
        int key; // hash
        int val; // line number
    };

    int       nLog2Commands; // log2( nCommands ) + 1
    KeyVal_t *aSorted = NULL;
    KeyVal_t  Temp;

    bool bCheckDupHashes = false;
    bool bVerbose = 0;

    // Default to ASCII [Ctrl-A ... ~]
    // [Begin,End)
    int gBegin = 0x00;
    int gEnd   = 0x80;

// Prototypes
    void dump_sort();
    void init_log2();
    void init_sort();
    void init_tags();
    int  line_count( const char *start, const char *end );
    void make_words( char *start, char *end );
    

// Check for duplicate hashes (collision)
// ========================================================================
void dup_check()
{
    if( !bCheckDupHashes )
        return;

    int duplicates = 0;

    for( int iCommand = 0; iCommand < nCommands-1; iCommand++ )
    {
        //if (aHashes[ iCommand ] == aHashes[ iCommand ])
        if (aSorted[ iCommand ].key == aSorted[ iCommand+1 ].key)
        {
            printf( "%X: %s\n", aSorted[ iCommand ].key, aCommands[ aSorted[ iCommand ].val ] );
            duplicates++;
        }
        else
        {
            if( duplicates )
            {
                printf( "%X: %s\n", aSorted[ iCommand ].key, aCommands[ aSorted[ iCommand ].val ] );
                printf( "Duplicates: %d\n", duplicates + 1 );
            }
            duplicates = 0;
        }
    }

    exit( 0 );
}

// ========================================================================
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


    for( int iCommand = 0; iCommand < nCommands; iCommand++ )
    {
        text = aCommands[ iCommand ];
        len  = aLengths [ iCommand ]; // strlen( text );
        hash = hash16( len, text );

        aHashes[ iCommand ] = hash;

if( bVerbose ) if( iCommand < 3)
        printf( "[%d] %04X: %s\n", iCommand, hash, text );
    }
}


// ========================================================================
void init_log2()
{
    nLog2Commands = log2_ceil( nCommands );

if( bVerbose ) printf( "Log2 %d = %d\n", nCommands, nLog2Commands );
}


// ========================================================================
void init_tags()
{
    // Need to sort hashes so find_hash() can use binary search
    for( int i = 0; i < nCommands; i++ )
    {
        aSorted[ i ].key = aHashes[ i ];
        aSorted[ i ].val = i;
    }
}


// ========================================================================
void init_sort()
{
    if( bVerbose )
        dump_sort();


#if 1 // QuickSort

    struct QuickSort
    {
        void sort( int min, int max )
        {
            // 1 element is always sorted
            if (min >= max) 
                return;

            // sort 2 elements
            if ((max - min) == 1)
            {
                if (aSorted[min].key > aSorted[max].key) // swap
                {
                    Temp = aSorted[min];
                           aSorted[min] = aSorted[max];
                                          aSorted[max] = Temp;
                }
                return;
            }

            // partition 3+ elements
            // Before: [# # # # # # # # # # #] [pivot]
            // After : [# < pivot] [# > pivot] [pivot]
            int lo  = min;
            int hi  = max-1;
            int piv = max;

            while (lo < hi)
            {
                if (aSorted[ lo ].key > aSorted[ hi ].key ) // swap
                {
                    Temp = aSorted[ lo ];
                           aSorted[ lo ] = aSorted[ hi ];
                                           aSorted[ hi ] = Temp;
                }

                if (aSorted[ lo ].key < aSorted[ piv ].key)
                    lo++;

                if (aSorted[ hi ].key > aSorted[ piv ].key)
                    hi--;
            }

            sort( min, lo-1  ); // < pivot
            sort( lo , max-1 ); // > pivot

            // Before: [...< pivot...] [...> pivot ..] [pivot]
            // After : [...< pivot...] [pivot] [...> pivot...]
            Temp = aSorted[ lo ];
            if (Temp.key > aSorted[ piv ].key)
            {
                for( int i = max; i > lo; i-- )
                    aSorted[ i ] = aSorted[ i-1 ];
                aSorted[ lo ] = Temp;
            }
        }
    };

#else // Insertion Sort
    const int n = nCommands;

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
#endif

    if( bVerbose )
        dump_sort();
}


// ========================================================================
void init_word()
{
    long        nFileSize = 0;
    const char *sWordListFileName = "wordlist.txt";

    FILE *pFile = fopen( sWordListFileName, "rb" );
    if( pFile )
    {
        fseek( pFile, 0L, SEEK_END );
        nFileSize = ftell( pFile );
        fseek( pFile, 0L, SEEK_SET );

        /**/  pWords = new char [ nFileSize + 1 ];
        char *pEnd   = pWords + nFileSize;

        fread( (void*) pWords, nFileSize, 1, pFile );
        pWords[ nFileSize ] = 0;

        nCommands = line_count( pWords, pEnd );

        if( !nCommands )
        {
            fclose( pFile );
            printf( "ERROR: Zero lines detected!\n" );
        }

if( bVerbose ) printf( "Lines: %d\n", nCommands );

        aCommands = new char*    [ nCommands ];
        aHashes   = new int      [ nCommands ];
        aLengths  = new char     [ nCommands ];
        aSorted   = new KeyVal_t [ nCommands ];

        make_words( pWords, pEnd );

        fclose( pFile );
    }
    else
    {
        printf( "Unable to open word list file: %s\n", sWordListFileName );
        exit( 1 );
    }
}


// ========================================================================
int line_count( const char *start, const char *end )
{
    int lines = 0;   

    for( const char *p = start; p < end; p++ )
        if (*p == 0xA)
            lines++;

    return lines;
}


// ========================================================================
void make_words( char *start, char *end )
{
    int   lines = 0;
    char *word  = start;
    int   len;

    for( char *p = start; p < end; p++ )
    {
        if (*p == 0xA)
        {

// TODO: Need to lowercase

            *p  = 0;
            len = (p - word); 
            aLengths [ lines ] = len;
            aCommands[ lines ] = word;

            word = p + 1;
            lines++;
        }
    }
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
    int mid = 0;
    int max = nCommands - 1;

    for( int i = 0; i < nLog2Commands; i++ )
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


// ========================================================================
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
int old = bVerbose;
bVerbose = true;
                collide( LEN, buffer, hash, find );
                collisions++;
bVerbose = old;
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
//printf( "%X\n", val );
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
        if (strcmp( aArg[ iArg ], "-az" ) == 0)
        {
            gBegin = 'a';
            gEnd   = 'z'+1;
        }
        else
        if (strcmp( aArg[ iArg ], "-up" ) == 0) // uppercase, alais for -AZ
        {
            gBegin = 'A';
            gEnd   = 'Z'+1;
        }
        else
        if (strcmp( aArg[ iArg ], "-lo" ) == 0) // lowercase, alias for -az
        {
            gBegin = 'a';
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
        if (strcmp( aArg[ iArg ], "-dup" ) == 0)
        {
            bCheckDupHashes = true;
        }
        else
            last = atoi( aArg[ iArg ] );
    }

    init_word(); //printf( "Hashing...\n" );
    init_hash(); //printf( "Tagging...\n" );
    init_tags(); //printf( "Sorting...\n" );
    init_sort(); //printf( "Log2 words ...\n" );
    init_log2();
    dup_check(); // Only check duplicate hashes in word list

printf( "Begin: %02X %c%c\n", gBegin, (gBegin < 0x20) ? '^' : gBegin, (gBegin < 0x20) ? ('@' + gBegin) : ' ' );
printf( "End  : %02X %c%c\n", gEnd-1, (gEnd-1 < 0x20) ? '^' : gEnd-1, (gEnd-1 < 0x20) ? ('@' + gEnd-1) : ' ' );

    Timer timer;
    timer.Start();

    if( last >= 2 ) test_2();
    if( last >= 3 ) test_3();
    if( last >= 4 ) test_4();
    if( last >= 5 ) test_5();

    timer.Stop( false ); // no need for milliseconds accuracy
    printf( "%s%s\n", timer.data.day, timer.data.hms );

    return 0;
}

