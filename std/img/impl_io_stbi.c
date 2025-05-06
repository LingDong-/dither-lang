#define STB_IMAGE_IMPLEMENTATION
#include "../../third_party/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../../third_party/stb_image_write.h"
#include <string.h>

uint8_t* img_impl_decode(uint8_t* buffer, size_t length, int* width, int* height, int* channels) {
  int req_channels = 4;
  int chan;
  uint8_t* data = stbi_load_from_memory(buffer, (int)length, width, height, &chan, req_channels);
  *channels = 4;
  return data;
}

typedef struct {
  uint8_t* data;
  int size;
} BufferCtx;

static void write_callback(void* context, void* data, int size) {
  BufferCtx* ctx = (BufferCtx*)context;
  ctx->data = realloc(ctx->data, ctx->size + size);
  memcpy(ctx->data + ctx->size, data, size);
  ctx->size += size;
}

uint8_t* img_impl_encode(char* format_ext, uint8_t* pixels, int width, int height, int channels, int* out_len) {
  BufferCtx ctx = {0};
  if (strcmp(format_ext, "png") == 0) {
    stbi_write_png_to_func(write_callback, &ctx, width, height, channels, pixels, width * channels);
  } else if (strcmp(format_ext, "jpg") == 0 || strcmp(format_ext, "jpeg") == 0) {
    stbi_write_jpg_to_func(write_callback, &ctx, width, height, channels, pixels, 90);
  } else if (strcmp(format_ext, "bmp") == 0) {
    stbi_write_bmp_to_func(write_callback, &ctx, width, height, channels, pixels);
  } else if (strcmp(format_ext, "tga") == 0) {
    stbi_write_tga_to_func(write_callback, &ctx, width, height, channels, pixels);
  } else {
    return NULL;
  }
  *out_len = ctx.size;
  return ctx.data;
}