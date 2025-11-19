#define HAS_ALPHA 1

#if defined(HAS_ALPHA)
var has_alpha : i32 = 1;
#else
var has_alpha : i32 = 0;
#endif

#if defined HAS_BETA
var has_beta  : i32 = 1;
#else
var has_beta  : i32 = 0;
#endif

