#include <stdio.h>
#include <string.h>
#include "hash16.h"

int main( const int nArg, const char *aArg[] )
{
    if (nArg > 1)
    {
        const char   *txt = aArg[1];
        const size_t  len = strlen( txt );
        int hash = hash16( len, txt );
        printf( "%X: %s\n", hash, txt );
    }

    return 0;
}
