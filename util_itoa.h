// printf() sucks for (decimal) integer output
// Why?
// No commas in (decimal) numbers > 1,000
//
// v5 rename itoaComma() -> itoa_comma()
// v4 Rename end -> tail, p -> head
// v3 Cleanup, add note about 2^32-1 and 2^64-1
// v2 Null terminate
// v1 NOTE: Does NOT null terminate ouput
// Mem usage: 32 bytes
// ========================================================================
char* itoa_comma( uint64_t n, char *output_ = NULL )
{
    const  size_t SIZE = 32;              // 2^32-1 =              4,294,967,295 (13 digits w/ commas)
    static char   buffer[ SIZE ];         // 2^64-1 = 18,446,744,073,709,551,615 (26 digits w/ commas)
    /* */  char  *tail = buffer + SIZE-1; // start at last char and fill backwards
    /*  */ char  *head = tail;            // head
    *head-- = 0;

    while( n >= 1000 )
    {
        *head-- = '0' + (n % 10); n /= 10;
        *head-- = '0' + (n % 10); n /= 10;
        *head-- = '0' + (n % 10); n /= 10;
        *head-- = ','                    ;
    }

    /*      */ { *head-- = '0' + (n % 10); n /= 10; }
    if( n > 0) { *head-- = '0' + (n % 10); n /= 10; }
    if( n > 0) { *head-- = '0' + (n % 10); n /= 10; }

    if( output_ )
    {
        size_t  len = tail - head; 
        memcpy( output_, head+1, len );
    }

    return ++head;
}
