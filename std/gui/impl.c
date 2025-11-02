#if DITHER_GUI_USE_COCOA
#include "impl_cocoa.h"
#elif DITHER_GUI_USE_WINAPI
#include "impl_win32.c"
#elif DITHER_GUI_USE_X11
#include "impl_x11.c"
#endif