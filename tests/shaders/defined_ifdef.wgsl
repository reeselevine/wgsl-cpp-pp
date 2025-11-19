#define ENABLE_FOO 1

#ifdef ENABLE_FOO
var foo_enabled : i32 = 1;
#else
var foo_enabled : i32 = 0;
#endif

#ifndef DISABLE_BAR
var bar_disabled : i32 = 0;
#else
var bar_disabled : i32 = 1;
#endif

