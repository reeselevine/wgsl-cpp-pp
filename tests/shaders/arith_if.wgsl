#define NUM_THREADS 64
#define BLOCKS 4
#define HIGH_QUALITY 1

#if (NUM_THREADS * BLOCKS) == 256
var product_256 : i32 = 1;
#else
var product_256 : i32 = 0;
#endif

#if (NUM_THREADS * BLOCKS) != 256
var product_not_256 : i32 = 1;
#else
var product_not_256 : i32 = 0;
#endif

#if (NUM_THREADS == 64) && (BLOCKS == 4) && HIGH_QUALITY
var combo_true : i32 = 1;
#else
var combo_true : i32 = 0;
#endif

#if (NUM_THREADS == 32) || (BLOCKS == 1)
var combo_false : i32 = 1;
#else
var combo_false : i32 = 0;
#endif

