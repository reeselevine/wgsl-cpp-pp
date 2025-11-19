#define MODE 2
#define HIGH_QUALITY 1

#if MODE == 1
var selected_mode : i32 = 1;
#elif MODE == 2
var selected_mode : i32 = 2;

    #if HIGH_QUALITY
    var quality_level : i32 = 2;
    #else
    var quality_level : i32 = 1;
    #endif

#else
var selected_mode : i32 = 3;
#endif

