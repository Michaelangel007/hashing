/*
    There are 2 varieties of log2:

        log2_floor()
        log2_ceil()

    n       log2()      log2_floor()    log2_ceil
    127     6.98...     6               7
    255     7.99...     7               8
*/

size_t log2_ceil( size_t n )
{
    size_t pow2 = 1;
    size_t y    = 0;

    while (pow2 <= n)
    {
        y++;
        pow2 <<= 1;
    }

    return y;
}
