#if DITHER_IMGIO_USE_COREGRAPHICS
#include "impl_io_coregraphics.c"
#elif DITHER_IMGIO_USE_STBI
#include "impl_io_stbi.c"
#endif