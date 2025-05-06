#include <CoreGraphics/CoreGraphics.h>
#include <ImageIO/ImageIO.h>
#include <CoreFoundation/CoreFoundation.h>
#include <stdio.h>
#include <stdlib.h>

uint8_t* img_impl_decode(uint8_t* buffer, size_t length, int* width, int* height, int* channels) {
  CFDataRef data = CFDataCreate(NULL, buffer, length);
  CGImageSourceRef src = CGImageSourceCreateWithData(data, NULL);
  CFRelease(data);

  CGImageRef image = CGImageSourceCreateImageAtIndex(src, 0, NULL);
  CFRelease(src);

  CGImageAlphaInfo alpha = CGImageGetAlphaInfo(image);

  int chan = 4;
  size_t w = CGImageGetWidth(image);
  size_t h = CGImageGetHeight(image);
  size_t stride = w * chan;

  uint8_t* pixels = malloc(h * stride);

  CGColorSpaceRef cs = CGColorSpaceCreateDeviceRGB();

  CGBitmapInfo info = kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big;

  CGContextRef ctx = CGBitmapContextCreate(pixels, w, h, 8, stride, cs, info);
  CGColorSpaceRelease(cs);

  CGContextDrawImage(ctx, CGRectMake(0, 0, w, h), image);
  CGContextRelease(ctx);
  CGImageRelease(image);

  *channels = chan;
  *width = w;
  *height = h;
  return pixels;
}

static CFStringRef uti_for_extension(char* ext) {
  if (!ext) return NULL;
  if (strcasecmp(ext, "png") == 0)   return CFSTR("public.png");
  if (strcasecmp(ext, "jpg") == 0)   return CFSTR("public.jpeg");
  if (strcasecmp(ext, "jpeg") == 0)  return CFSTR("public.jpeg");
  if (strcasecmp(ext, "bmp") == 0)   return CFSTR("com.microsoft.bmp");
  if (strcasecmp(ext, "gif") == 0)   return CFSTR("com.compuserve.gif");
  if (strcasecmp(ext, "tiff") == 0)  return CFSTR("public.tiff");
  if (strcasecmp(ext, "tif") == 0)   return CFSTR("public.tiff");
  if (strcasecmp(ext, "heic") == 0)  return CFSTR("public.heic");
  if (strcasecmp(ext, "webp") == 0)  return CFSTR("org.webmproject.webp");
  return NULL;
}

uint8_t* img_impl_encode(char* format_ext, uint8_t* pixels, int width, int height, int channels, int* out_len) {
  CFStringRef uti = uti_for_extension(format_ext);

  size_t bitsPerComponent = 8;
  size_t bitsPerPixel = channels * 8;
  CGBitmapInfo bitmapInfo;
  CGColorSpaceRef colorSpace = NULL;
  switch (channels) {
    case 1:
      colorSpace = CGColorSpaceCreateDeviceGray();
      bitmapInfo = kCGImageAlphaNone;
      break;
    case 3:
      colorSpace = CGColorSpaceCreateDeviceRGB();
      bitmapInfo = kCGImageAlphaNone | kCGBitmapByteOrderDefault;
      break;
    case 4:
      colorSpace = CGColorSpaceCreateDeviceRGB();
      bitmapInfo = kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big;
      break;
    default:
      CFRelease(uti);
      return 0;
  }
  CGContextRef ctx = CGBitmapContextCreate(pixels,width,height,bitsPerComponent,width*channels,colorSpace,bitmapInfo);

  CGColorSpaceRelease(colorSpace);

  CGImageRef cgimage = CGBitmapContextCreateImage(ctx);
  CGContextRelease(ctx);

  CFMutableDataRef outData = CFDataCreateMutable(NULL, 0);
  CGImageDestinationRef dest = CGImageDestinationCreateWithData(outData, uti, 1, NULL);
  CFRelease(uti);

  CGImageDestinationAddImage(dest, cgimage, NULL);
  CGImageDestinationFinalize(dest);
  *out_len = (int)CFDataGetLength(outData);
  uint8_t* out_bytes = (uint8_t*)malloc(*out_len);
  memcpy(out_bytes, CFDataGetBytePtr(outData), *out_len);

  CFRelease(dest);
  CFRelease(outData);
  CGImageRelease(cgimage);
  return out_bytes;
}
