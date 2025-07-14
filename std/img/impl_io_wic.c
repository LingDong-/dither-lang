#include <windows.h>
#include <wincodec.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "ole32.lib")

uint8_t* img_impl_decode(uint8_t* buffer, size_t length, int* width, int* height, int* channels) {
  HRESULT hr;
  IWICImagingFactory* factory = NULL;
  IWICStream* stream = NULL;
  IWICBitmapDecoder* decoder = NULL;
  IWICBitmapFrameDecode* frame = NULL;
  IWICFormatConverter* converter = NULL;
  uint8_t* pixels = NULL;
  *width = *height = *channels = 0;
  hr = CoInitialize(NULL);
  hr = CoCreateInstance(&CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER,
                        &IID_IWICImagingFactory, (LPVOID*)&factory);
  hr = factory->lpVtbl->CreateStream(factory, &stream);
  hr = stream->lpVtbl->InitializeFromMemory(stream, buffer, (DWORD)length);
  hr = factory->lpVtbl->CreateDecoderFromStream(factory, (IStream*)stream, NULL,
                                                WICDecodeMetadataCacheOnLoad, &decoder);
  hr = decoder->lpVtbl->GetFrame(decoder, 0, &frame);
  hr = factory->lpVtbl->CreateFormatConverter(factory, &converter);
  hr = converter->lpVtbl->Initialize(converter, (IWICBitmapSource*)frame,
                                      &GUID_WICPixelFormat32bppRGBA,
                                      WICBitmapDitherTypeNone, NULL, 0.0f,
                                      WICBitmapPaletteTypeCustom);
  UINT w = 0, h = 0;
  converter->lpVtbl->GetSize(converter, &w, &h);
  pixels = (uint8_t*)malloc(w * h * 4);
  hr = converter->lpVtbl->CopyPixels(converter, NULL, w * 4, w * h * 4, pixels);
  *width = (int)w;
  *height = (int)h;
  *channels = 4;
  if (converter) converter->lpVtbl->Release(converter);
  if (frame) frame->lpVtbl->Release(frame);
  if (decoder) decoder->lpVtbl->Release(decoder);
  if (stream) stream->lpVtbl->Release(stream);
  if (factory) factory->lpVtbl->Release(factory);
  CoUninitialize();
  return pixels;
}

static const GUID* format_guid_for_ext(const char* ext) {
  if (_stricmp(ext, "png") == 0) return &GUID_ContainerFormatPng;
  if (_stricmp(ext, "jpg") == 0 || _stricmp(ext, "jpeg") == 0) return &GUID_ContainerFormatJpeg;
  if (_stricmp(ext, "bmp") == 0) return &GUID_ContainerFormatBmp;
  if (_stricmp(ext, "tiff") == 0 || _stricmp(ext, "tif") == 0) return &GUID_ContainerFormatTiff;
  return NULL;
}

uint8_t* img_impl_encode(const char* format_ext, uint8_t* pixels, int width, int height, int channels, int* out_len) {
  HRESULT hr;
  IWICImagingFactory* factory = NULL;
  IWICBitmap* bitmap = NULL;
  IWICStream* stream = NULL;
  IWICBitmapEncoder* encoder = NULL;
  IWICBitmapFrameEncode* frame = NULL;
  IPropertyBag2* props = NULL;
  uint8_t* out_bytes = NULL;
  *out_len = 0;
  const GUID* containerFormat = format_guid_for_ext(format_ext);
  if (!containerFormat || channels != 4) return NULL; // only RGBA supported
  hr = CoInitialize(NULL);
  hr = CoCreateInstance(&CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER,
                        &IID_IWICImagingFactory, (void**)&factory);
  hr = factory->lpVtbl->CreateBitmapFromMemory(factory,
      width, height, &GUID_WICPixelFormat32bppRGBA,
      width * 4, width * height * 4, pixels, &bitmap);
  hr = factory->lpVtbl->CreateStream(factory, &stream);
  IStream* memStream = NULL;
  HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, 0);
  hr = CreateStreamOnHGlobal(hMem, TRUE, &memStream);
  hr = factory->lpVtbl->CreateEncoder(factory, containerFormat, NULL, &encoder);
  hr = encoder->lpVtbl->Initialize(encoder, memStream, WICBitmapEncoderNoCache);
  hr = encoder->lpVtbl->CreateNewFrame(encoder, &frame, &props);
  hr = frame->lpVtbl->Initialize(frame, props);
  hr = frame->lpVtbl->SetSize(frame, width, height);
  WICPixelFormatGUID format = GUID_WICPixelFormat32bppRGBA;
  hr = frame->lpVtbl->SetPixelFormat(frame, &format);
  hr = frame->lpVtbl->WriteSource(frame, (IWICBitmapSource*)bitmap, NULL);
  hr = frame->lpVtbl->Commit(frame);
  hr = encoder->lpVtbl->Commit(encoder);
  STATSTG stat;
  hr = memStream->lpVtbl->Stat(memStream, &stat, STATFLAG_NONAME);
  SIZE_T size = (SIZE_T)stat.cbSize.QuadPart;
  HGLOBAL hCopy = NULL;
  GetHGlobalFromStream(memStream, &hCopy);
  void* pData = GlobalLock(hCopy);
  out_bytes = (uint8_t*)malloc(size);
  memcpy(out_bytes, pData, size);
  *out_len = (int)size;
  GlobalUnlock(hCopy);
  if (props) props->lpVtbl->Release(props);
  if (frame) frame->lpVtbl->Release(frame);
  if (encoder) encoder->lpVtbl->Release(encoder);
  if (memStream) memStream->lpVtbl->Release(memStream);
  if (stream) stream->lpVtbl->Release(stream);
  if (bitmap) bitmap->lpVtbl->Release(bitmap);
  if (factory) factory->lpVtbl->Release(factory);
  CoUninitialize();
  return out_bytes;
}
