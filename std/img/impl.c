#if DITHER_IMGIO_USE_COREGRAPHICS
#include "impl_io_coregraphics.c"
#elif DITHER_IMGIO_USE_STBI
#include "impl_io_stbi.c"
#elif DITHER_IMGIO_USE_WIC
#include "impl_io_wic.c"
#endif
#include "impl_proc.c"