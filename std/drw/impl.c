#if DITHER_2D_USE_COREGRAPHICS
#include "impl_coregraphics.c"
#elif DITHER_2D_USE_OPENGL
#include "impl_opengl.c"
#elif DITHER_2D_USE_DIRECT2D
#include "impl_direct2d.c"
#endif