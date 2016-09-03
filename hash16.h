/*
    Hash Type Legend

        &x##   And (hex) (constant) value       hash &= 0xFFFF      // &xFFFF
        *#     Multiply (dec) constant value    hash *= 3           // *3
        *x#    Multiply (hex) constant value    hash *= 0x193       // *x193
        +(...) Add expression that follows;     hash += (hash << 3) // +(L3)
        ^#     Xor dec value                    hash ^= 1           // ^1
        ^n     Xor string length                hash ^= len         // ^n
        ^x#    Xor hex value                    hash ^= 0xD5        // ^xD5

        A      Add string byte, alias for +b;   hash += byte        // A
        E      Xor string byte, alias for ^b;   hash ^= byte        // E
        L#     Shift hash left #-bits;          hash <<= 3          // L3
        M#     Multiply constant value          hash *= 3           // M3
        N#     And (dec) constant value         hash &= 32767       // N32767
        Nx#    And (hex) constant value         hash &= 0xFFFF      // NxFFFF
        R#     Shift hash right #-bits;         hash >>= 3          // R3
        X#     Xor (dec) constant value         hash ^= 119         // X193
        Xx#    Xor (hex) constant value         hash ^= 0xD5        // XxD5
        Xn     Xor string length                hash ^= len         // Xn
*/

#define AS3XD5 100
#define FNV    101
#define FNV16  102
#define S1X    103
#define SO     104


//Bits  
// 16 = 4-bit len + 12-bit data
// 20 = 4-bit len + 16-bit data
// 24 = 4-bit len + 20-bit data
#ifndef HASH_BITS
//#define HASH_BITS 24
//#define HASH_ALGO  1

#define HASH_BITS 16
#define HASH_ALGO FNV
#endif//HASH_BITS



int hash16( const size_t len, const char *text )
{
// v1
//    int hash = 0xD5AA; // v1

    unsigned int  hash = 0; // v2=0xFFFF
    unsigned int  n    = (int) len;
    unsigned char b;

#if HASH_BITS == 32
    #if HASH_ALGO == FNV
        hash = 2166136261;
    #endif
#endif

    #if HASH_ALGO == 3
    int c = 0;
    #endif

    if( !n )
    {
        n = (int) strlen( text );
        //printf( "Auto-detect: (%d) %s\n", n, text );
    }

    for( int i = 0; i < n; i++ )
    {
        b = (unsigned) text[i];

#if HASH_BITS == 16
    #if HASH_ALGO == FNV // XbMx1000193
/*                              -AZ         -Az         -print      -ascii
DOS3.3
    2x: Collisions: 3           1           1           2           3
    3x: Collisions: 846         8           76          345         846
    4x: Collisions: 110,294     201         4,697       33,800      110,294
    5x: Collisions: 14,157,240  4,975       270,525     3,188,249   14,157,240
    Time:           7m2.700s    00:00:00    00:00:11    00:02:20    7m36s
                                                        ^ 1m38s
                                                        2m38s (before)

wordlist
    2x: Collisions:             0           0           2           3
    3x: Collisions:             12          65          264         619
    4x: Collisions:             122         3,341       23,605      77,850
    5x: Collisions:             3,431       190,242     2,245,714
    Time:                       00:00:01    00:01:05    12m
*/
        hash ^= b;
        hash *= 16777619; // 0x1000193 // 224 + 28 + 0x93 = 16777619
    #endif

    #if HASH_ALGO == FNV16 // XbMx193NxFFFF
/*                  -AZ     -Az     -print      -ascii
    2x: Collisions: 1       3       5
    3x: Collisions: 11      87      354
    4x: Collisions: 194     4,682   33,547
    5x: Collisions: 4,876   270,394 3,189,173
    Time:           0m0s    0m8s    1m37s
*/
        hash ^= b;
        hash *= 0x193;
        hash &= 0xFFFF;
    #endif

    #if HASH_ALGO == S1X // v1 // @2 = 64 collisions, 'S1X'
        hash <<= 1;
        hash &= 0xFFFF;
        hash ^= b;
    #endif

    #if HASH_ALGO == AL3XD5 // v2 -- @2 = 32 collision 'AS3XD5
            hash += b;
        hash &= 0xFFFF;
            hash <<= 3;
        hash &= 0xFFFF;
            hash ^= 0xD5;
        hash &= 0xFFFF;
    #endif

    #if HASH_ALGO == SO // v5 -- @2 = 16-bit hash
        /*
        // http://stackoverflow.com/questions/7130032/uniform-16bit-hash-function-for-strings
        2: Collisions: 20
        3: Collisions: 903
        4: Collisions: 114,947
        5: Collisions: 14,222,650
        Time: 7m1.638s
        */
        hash = hash + ((hash) << 5) + b + (b << 7);
    #endif

    #if HASH_ALGO == 4 // v4 -- @ = 4 collisions
        hash = hash + ((hash) << 5) + b + (b << 7);
        hash &= 0xFFFF;
    #endif

    #if HASH_ALGO == 5 // v3 -- @2 = 4 collisions, +(L5)AXn
        /*
        2: Collisions: 4
        3: Collisions: 2,572
        4: Collisions: 524,185
        5: Collisions: 50,332,301
        Time: 6m58.556s
        */
        hash += (hash << 5);
        hash &= 0xFFFF;
        hash += b;
        hash &= 0xFFFF;
        hash ^= n;
    #endif

    #if HASH_ALGO == 6 // X(L3)A(R5)Xb
    /*
        2: Collisions: 16
        3: Collisions: 2505
        4: Collisions: 516878
    */
        hash ^= hash << 3;
        hash += hash >> 5;
        hash ^= b;
    #endif

    #if HASH_ALGO == 7 // XbM3 or ^b*3
/*                  -ascii
    2x: Collisions: 43
    3x: Collisions: 9,245
    4x: Collisions: 626,811
    5x: Collisions: 46,607,657
    Time:           6m29.187s
*/
        hash ^= b;
        hash *= 3;
    #endif

    #if HASH_ALGO == 8 // XbM5
        hash ^= b;
        hash *= 5;
    #endif

    #if HASH_ALGO == 9 // v7
        hash ^= b;
        hash *= 7;
    #endif

    #if HASH_ALGO == 11 // v11
        hash ^= b;
        hash *= 11;
    #endif

    #if HASH_ALGO == 13 // v13
        hash ^= b;
        hash *= 13;
    #endif

    #if HASH_ALGO == 17 // v17
/*              -ascii
2x: Collisions: 8
3x: Collisions: 2,545
4x: Collisions: 515,806
*/
        hash ^= b;
        hash *= 17;
    #endif

    #if HASH_ALGO == 19 // v19 E
        hash ^= b;
        hash *= 19;
    #endif

    #if HASH_ALGO == 20 // v10
        hash ^= b;
        for( int s = 0; s < 3; s++ )
        {
            hash <<= 1;
            hash +=  n;
            hash &= 0xFFFF;
        }
    #endif
#endif // 16-bit (2 bytes)

// ========== 20 ==========
#if HASH_BITS == 20
    #if HASH_ALGO == 1 // 20-bit hash --
            hash ^= n;
            hash += b;
            hash &= 0xFFFF;
            hash <<= 3;
            hash &= 0xFFFF;
    #endif
#endif // 20-bit (2.5 bytes)

// ========== 24 ==========
#if HASH_BITS == 24
    #if HASH_ALGO == 1 // XS3
/*

TODO: -AZ -Az -print -ascii -byte

S = 3               -AZ     -Az         -print      -ascii      -byte
    2x: Collisions: 3       5           12          16          32
    3x: Collisions: 48      254         713         1,280       5,120
    4x: Collisions: 242     2,874       13,560      49,408      425,984
    5x: Collisions: 676     41,268      585,711     2,621,440   83,886,080
    Time:           0m0s    0m7s        1m27s       6m30s       207m54

S = 4               -AZ     -Az         -print      -ascii      -byte
    2x: Collisions: 1       3           6           8
    3x: Collisions: 9       66          248         592
    4x: Collisions: 89      2,092       16,132      53,248
    5x: Collisions: 1,508   87,928      1,164,795   5,242,880
    Time:           0m0s    0m7s        1m26        6m34s

S = 5               -AZ     -Az         -print      -ascii      -byte
    2x: Collisions: 1       2           3           4           8
    3x: Collisions: 9       84          321         768         6,144
    4x: Collisions: 286     4,988       30,495      98,304      1,572,864
    5x: Collisions: 4,732   218,660     2,599,200   11,534,336  ???
    Time:           0m0s    0m7s        1m28        6m16s       25+m

S = 6               -AZ     -Az         -print      -ascii      -byte


S = 6                   -ascii      -AZ 5   -Az 5
        2x: Collisions: 2           1       1
        3x: Collisions: 1,664       60      161
        4x: Collisions: 212,992     1,664   10,846
        5x: Collisions: 20,971,520  31,096  390,224
        Time: 6m28.753s             0m0s    0m7s

S = 7
        2: Collisions: 2
        3: Collisions: 3,072
        4: Collisions: 393,216
        Time:

S = 5 ??????????        -print
        2: Collisions: 4        ?????
        3: Collisions: 832      ????
        4: Collisions: 106,496  ????


*/
        hash ^= b;
        for( int s = 0; s < 3; s++ )
        {
            hash <<= 1;
            hash +=  n;
        }
    #endif

    #if HASH_ALGO_2
        hash <<= 3;
        hash &= 0xFFFFFF;
        hash ^= b;
        hash += n;
    #endif

    #if HASH_ALGO == 3
    /*
            A = LEN
            X = ptr lo
            Y = ptr hi

            Hash24
                    STA LEN
                    STX p+0
                    STY p+1

                    TAY
                    LDA #0
                    STA HASH+0
                    STA HASH+1
                    STA HASH+2
                    CLC
            HashChar           
                    LDA (p),y
                    EOR HASH+0
                    LDX #5
            Shift
                    ROL HASH+0
                    ROL HASH+1
                    ROL HASH+2
                    LDA HASH+0
                    ADC LEN
                    STA HASH+0
                    DEX
                    BNE Shift

                    DEY
                    BNE HashChar

                    LDA LEN
                    ASL     ; 0-000x_xxx0
                    ASL     ; 0-00xx_xx00
                    ASL     ; 0_0xxx_x000
                    ASL     ; 0_xxxx_0000
                    STA LEN
                    LDA HASH+2
                    AND #$F
                    ORA LEN
                    STA HASH+2
                    RTS
        */
//         76543210
// ROL  C<-abcdefgh<-C
// ROR  C->abcdefgh->C
// LSR  0->abcdefgh->C
// ASL  C<-abcdefgh<-0
/*
-up 5
2: Collisions: 1
3: Collisions: 4
4: Collisions: 117
5: Collisions: 1837
*/
        b = (unsigned) text[ n-i-1 ];
        hash ^= b;
        for( int s = 0; s < 5; s++ )
        {
            c = (hash >> 15) & 1;
            hash <<= 1;
            hash += (n + c);
        }
    #endif
#endif // 24-bit (3 bytes)
    }

    //6502:
    //   hash[1] &= 0x0F;
    //   hash[1] |= (len << 4);

#if HASH_BITS == 16
    #if HASH_ALGO   == FNV
        hash = (hash >> 16) ^ (hash & 0xFFFF);
    #elif HASH_ALGO == FNV16
        hash &= 0xFFFF;
    #elif HASH_ALGO == 5 // v5
        return ((hash) ^ (hash >> 16)) & 0xffff; // @2 = 20 collisions
    #else // v1, v2, v3
        hash &= 0x0FFF;
        hash |= (n << 12);
    #endif
#endif

#if HASH_BITS == 20
    hash &= 0xFFFF;
    hash |= (n << 16);
#endif

#if HASH_BITS == 24
    hash &= 0x0FFFFF;
    hash |= (n < (4*5));
#endif

    return hash;
}

