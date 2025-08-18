#include <windows.h>
#include <dshow.h>
#include <stdio.h>
#include <stdint.h>

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "strmiids.lib")
#pragma comment(lib, "OleAut32.lib")

static const GUID CLSID_SampleGrabber = 
{ 0xC1F400A0, 0x3F08, 0x11d3,{ 0x9F, 0x0B, 0x00, 0x60, 0x08, 0x03, 0x9E, 0x37 } };
static const GUID IID_ISampleGrabber = 
{ 0x6B652FFF, 0x11FE, 0x4fce,{ 0x92, 0xAD, 0x02, 0x66, 0xB5, 0xD7, 0xC7, 0x8F } };
static const GUID IID_ISampleGrabberCB = 
{ 0x0579154A, 0x2B53, 0x4994,{ 0xB0, 0xD0, 0xE7, 0x73, 0x14, 0x8E, 0xFF, 0x85 } };
static const CLSID CLSID_NullRenderer = 
{ 0xC1F400A4, 0x3F08, 0x11d3,{ 0x9F, 0x0B, 0x00, 0x60, 0x08, 0x03, 0x9E, 0x37 } };
typedef struct ISampleGrabberCBVtbl ISampleGrabberCBVtbl;
typedef struct ISampleGrabberCB {
    ISampleGrabberCBVtbl *lpVtbl;
} ISampleGrabberCB;
struct ISampleGrabberCBVtbl {
    HRESULT(STDMETHODCALLTYPE *QueryInterface)(ISampleGrabberCB*, REFIID, void**);
    ULONG(STDMETHODCALLTYPE *AddRef)(ISampleGrabberCB*);
    ULONG(STDMETHODCALLTYPE *Release)(ISampleGrabberCB*);
    HRESULT(STDMETHODCALLTYPE *SampleCB)(ISampleGrabberCB*, double, IMediaSample*);
    HRESULT(STDMETHODCALLTYPE *BufferCB)(ISampleGrabberCB*, double, BYTE*, long);
};
typedef struct ISampleGrabberVtbl ISampleGrabberVtbl;
typedef struct ISampleGrabber {
    ISampleGrabberVtbl *lpVtbl;
} ISampleGrabber;
struct ISampleGrabberVtbl {
  HRESULT(STDMETHODCALLTYPE *QueryInterface)(ISampleGrabber*, REFIID, void**);
  ULONG(STDMETHODCALLTYPE *AddRef)(ISampleGrabber*);
  ULONG(STDMETHODCALLTYPE *Release)(ISampleGrabber*);
  HRESULT(STDMETHODCALLTYPE *SetOneShot)(ISampleGrabber*, BOOL);
  HRESULT(STDMETHODCALLTYPE *SetMediaType)(ISampleGrabber*, AM_MEDIA_TYPE*);
  HRESULT(STDMETHODCALLTYPE *GetConnectedMediaType)(ISampleGrabber*, AM_MEDIA_TYPE*);
  HRESULT(STDMETHODCALLTYPE *SetBufferSamples)(ISampleGrabber*, BOOL);
  HRESULT(STDMETHODCALLTYPE *GetCurrentBuffer)(ISampleGrabber*, long*, long*);
  HRESULT(STDMETHODCALLTYPE *GetCurrentSample)(ISampleGrabber*, IMediaSample**);
  HRESULT(STDMETHODCALLTYPE *SetCallback)(ISampleGrabber*, ISampleGrabberCB*, long);
};
#define DeleteMediaType(pmt) CoTaskMemFree((pmt)->pbFormat)
#ifndef VIDEOINFOHEADER2_DEFINED
#define VIDEOINFOHEADER2_DEFINED
typedef struct _VIDEOINFOHEADER2 {
    RECT            rcSource;
    RECT            rcTarget;
    DWORD           dwBitRate;
    DWORD           dwBitErrorRate;
    REFERENCE_TIME  AvgTimePerFrame;
    BITMAPINFOHEADER bmiHeader;
    DWORD           dwInterlaceFlags;
    DWORD           dwCopyProtectFlags;
    DWORD           dwPictAspectRatioX;
    DWORD           dwPictAspectRatioY;
    DWORD           dwControlFlags;
    DWORD           dwReserved2;
} VIDEOINFOHEADER2;
#endif

typedef struct MyGrabberCB {
  ISampleGrabberCBVtbl *lpVtbl;
  LONG ref;
  int idx;
} MyGrabberCB;

#define SOURCE_WEBCAM 1
#define SOURCE_FREAD  2
#define SOURCE_FWRITE 4
#define RESO_VGA  8
#define RESO_HD   16
#define RESO_FHD  32
#define EFFECT_MIRROR  64

#undef ARR_DEF
#define ARR_DEF(dtype) \
  typedef struct { int len; int cap; dtype* data; } dtype ## _arr_t;

#undef ARR_INIT
#define ARR_INIT(dtype,name) \
  name.len = 0;  \
  name.cap = 8; \
  name.data = (dtype*) malloc((name.cap)*sizeof(dtype));

#undef ARR_PUSH
#undef ARR_ITEM_FORCE_CAST
#ifdef _WIN32
#define ARR_ITEM_FORCE_CAST(dtype,item) item
#else
#define ARR_ITEM_FORCE_CAST(dtype,item) (dtype)item
#endif
#define ARR_PUSH(dtype,name,item) \
  if (name.cap < name.len+1){ \
    int hs = name.cap/2; \
    name.cap = name.len+MAX(1,hs); \
    name.data = (dtype*)realloc(name.data, (name.cap)*sizeof(dtype) ); \
  }\
  name.data[name.len] = ARR_ITEM_FORCE_CAST(dtype,item);\
  name.len += 1;

#undef ARR_POP
#define ARR_POP(dtype,name) (name.data[--name.len])

#undef ARR_CLEAR
#define ARR_CLEAR(dtype,name) {name.len = 0;}


int vid_cnt = 0;

typedef struct vid_st {
  int id;
  int w;
  int h;
  int flag;
  char* data;
  int rw;
  int rh;
  IMediaControl* pMC;
  IMediaSeeking* pMS;
  IMediaEventEx* pME;
} vid_t;

ARR_DEF(vid_t);

vid_t_arr_t vids = {0};

void processBuffer(int idx, BYTE* base){
  vid_t* vid = ((vid_t*)vids.data) + idx;
  if (vid->data == NULL){
    vid->data = malloc(vid->w*vid->h*4);
  }
  int bytesPerRow = vid->rw*4;
  int wh = (float)vid->rw/(float)vid->w > (float)vid->rh/(float)vid->h;
  uint8_t *p = (uint8_t *)base;
  for (int y = 0; y < vid->h; y++) {
    uint8_t* row;
    if (wh){
      row = p + ((y*vid->rh)/vid->h) * bytesPerRow;
    }else{
      row = p + ((y*vid->rw)/vid->w) * bytesPerRow;
    }
    for (int x = 0; x < vid->w; x++) {
      int col;
      if (wh){
        col = ((x * vid->rh)/vid->h)*4;
      }else{
        col = ((x * vid->rw)/vid->w)*4;
      }
      int xd = x;
      if (vid->flag & EFFECT_MIRROR){
        xd = vid->w-x-1;
      }
      int yd = vid->h-y-1;
      ((uint8_t*)(vid->data))[(yd*vid->w+xd)*4+0] = row[col+2];
      ((uint8_t*)(vid->data))[(yd*vid->w+xd)*4+1] = row[col+1];
      ((uint8_t*)(vid->data))[(yd*vid->w+xd)*4+2] = row[col+0];
      ((uint8_t*)(vid->data))[(yd*vid->w+xd)*4+3] = row[col+3];
    }
  }
}


HRESULT GetPin(IBaseFilter *pFilter, PIN_DIRECTION dir, IPin **ppPin) {
  *ppPin = NULL;
  IEnumPins *pEnum = NULL;
  if (FAILED(pFilter->lpVtbl->EnumPins(pFilter, &pEnum))) return E_FAIL;
  IPin *pPin = NULL;
  while (pEnum->lpVtbl->Next(pEnum, 1, &pPin, NULL) == S_OK) {
    PIN_DIRECTION pd;
    pPin->lpVtbl->QueryDirection(pPin, &pd);
    if (pd == dir) {
      *ppPin = pPin;
      pEnum->lpVtbl->Release(pEnum);
      return S_OK;
    }
    pPin->lpVtbl->Release(pPin);
  }
  pEnum->lpVtbl->Release(pEnum);
  return E_FAIL;
}

HRESULT STDMETHODCALLTYPE my_QueryInterface(ISampleGrabberCB* This, REFIID riid, void** ppvObject) {
  if (!ppvObject) return E_POINTER;
  *ppvObject = NULL;
  if (IsEqualIID(riid, &IID_ISampleGrabberCB) || IsEqualIID(riid, &IID_IUnknown)) {
    *ppvObject = This;
    This->lpVtbl->AddRef(This);
    return S_OK;
  }
  return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE my_AddRef(ISampleGrabberCB* This) {
    return 1;
}

ULONG STDMETHODCALLTYPE my_Release(ISampleGrabberCB* This) {
    return 1;
}

HRESULT STDMETHODCALLTYPE my_SampleCB(ISampleGrabberCB* This, double SampleTime, IMediaSample* pSample) {
  MyGrabberCB *cb = (MyGrabberCB*)This;
  BYTE* pData = NULL;
  if (pSample && SUCCEEDED(pSample->lpVtbl->GetPointer(pSample, &pData)) && pData) {
    processBuffer(((MyGrabberCB*)This)->idx,pData);
  }
  return S_OK;
}

HRESULT STDMETHODCALLTYPE my_BufferCB(ISampleGrabberCB *p, double SampleTime, BYTE *pBuffer, long BufferLen) {
  processBuffer(((MyGrabberCB*)p)->idx,pBuffer);
  return S_OK;
}

int vin_impl_create(int flag, char* path, int* w, int* h){
  vid_t v;
  v.data = NULL;
  v.flag = flag;

  int n_path = strlen(path);
  WCHAR* wpath = (WCHAR*)_alloca((n_path+1)*sizeof(WCHAR));
  MultiByteToWideChar(CP_UTF8, 0, path, -1, wpath, n_path+1);
  
  static ISampleGrabberCBVtbl MyGrabberCB_Vtbl = {
    my_QueryInterface,
    my_AddRef,
    my_Release,
    my_SampleCB,
    my_BufferCB
  };
  AM_MEDIA_TYPE mt;
  ZeroMemory(&mt, sizeof(mt));
  mt.majortype = MEDIATYPE_Video;
  mt.subtype = MEDIASUBTYPE_ARGB32;
  mt.formattype = FORMAT_VideoInfo;
  CoInitialize(NULL);

  if (flag & SOURCE_WEBCAM){
    if (flag & RESO_VGA){
      v.w = 640; v.h = 480;
    }else if (flag & RESO_HD){
      v.w = 1280; v.h = 720;
    }else if (flag & RESO_FHD){
      v.w = 1920; v.h = 1080;
    }

    IGraphBuilder *pGraph = NULL;
    ICaptureGraphBuilder2 *pBuilder = NULL;
    IBaseFilter *pCap = NULL;
    IBaseFilter *pGrabberF = NULL;
    ISampleGrabber *pGrabber = NULL;
    CoCreateInstance(&CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
                     &IID_IGraphBuilder, (void**)&pGraph);
    CoCreateInstance(&CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER,
                     &IID_ICaptureGraphBuilder2, (void**)&pBuilder);
    pBuilder->lpVtbl->SetFiltergraph(pBuilder, pGraph);
    ICreateDevEnum *pDevEnum = NULL;
    IEnumMoniker *pEnum = NULL;
    CoCreateInstance(&CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
                    &IID_ICreateDevEnum, (void**)&pDevEnum);
    pDevEnum->lpVtbl->CreateClassEnumerator(pDevEnum, &CLSID_VideoInputDeviceCategory, &pEnum, 0);
    IMoniker *pMoniker = NULL;
    while (pEnum->lpVtbl->Next(pEnum, 1, &pMoniker, NULL) == S_OK) {
      IPropertyBag *pPropBag = NULL;
      pMoniker->lpVtbl->BindToStorage(pMoniker, 0,0, &IID_IPropertyBag, (void**)&pPropBag);
      VARIANT varName;
      VariantInit(&varName);
      pPropBag->lpVtbl->Read(pPropBag, L"FriendlyName", &varName, 0);
      if (n_path==0 || wcsstr(varName.bstrVal, wpath) != NULL) {
        pMoniker->lpVtbl->BindToObject(pMoniker, NULL, NULL, &IID_IBaseFilter, (void**)&pCap);
        pGraph->lpVtbl->AddFilter(pGraph, pCap, L"Capture Filter");
        IAMStreamConfig *pConfig = NULL;
        IPin *pPin = NULL;
        IEnumPins *pEnumPins = NULL;
        pCap->lpVtbl->EnumPins(pCap, &pEnumPins);
        while (pEnumPins->lpVtbl->Next(pEnumPins, 1, &pPin, NULL) == S_OK) {
          PIN_INFO pinInfo;
          pPin->lpVtbl->QueryPinInfo(pPin, &pinInfo);
          if (pinInfo.dir == PINDIR_OUTPUT) break;
          pPin->lpVtbl->Release(pPin);
          pPin = NULL;
        }
        pEnumPins->lpVtbl->Release(pEnumPins);
        pPin->lpVtbl->QueryInterface(pPin, &IID_IAMStreamConfig, (void**)&pConfig);
        int iCount = 0, iSize = 0;
        pConfig->lpVtbl->GetNumberOfCapabilities(pConfig, &iCount, &iSize);
        for (int i = 0; i < iCount; i++) {
          AM_MEDIA_TYPE *pmt = NULL;
          BYTE *pSCC = (BYTE*)malloc(iSize);
          if (pConfig->lpVtbl->GetStreamCaps(pConfig, i, &pmt, pSCC) == S_OK) {
            VIDEOINFOHEADER *pVih = (VIDEOINFOHEADER*)pmt->pbFormat;
            if (i == iCount-1 || (pVih->bmiHeader.biWidth == v.w && pVih->bmiHeader.biHeight == v.h)) {
              v.rw = pVih->bmiHeader.biWidth;
              v.rh = pVih->bmiHeader.biHeight;
              pConfig->lpVtbl->SetFormat(pConfig, pmt);
              if (pmt) DeleteMediaType(pmt);
              free(pSCC);
              break;
            }
            if (pmt) DeleteMediaType(pmt);
          }
          free(pSCC);
        }
        pConfig->lpVtbl->Release(pConfig);
        pPin->lpVtbl->Release(pPin);
        VariantClear(&varName);
        pPropBag->lpVtbl->Release(pPropBag);
        break;
      }
      VariantClear(&varName);
    }
    CoCreateInstance(&CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER,
                     &IID_IBaseFilter, (void**)&pGrabberF);
    pGraph->lpVtbl->AddFilter(pGraph, pGrabberF, L"SampleGrabber");
    pGrabberF->lpVtbl->QueryInterface(pGrabberF, &IID_ISampleGrabber, (void**)&pGrabber);
    pGrabber->lpVtbl->SetMediaType(pGrabber, &mt);
    IPin *pCapOut = NULL;
    IPin *pGrabIn = NULL;
    GetPin(pCap, PINDIR_OUTPUT, &pCapOut);
    GetPin((IBaseFilter*)pGrabberF, PINDIR_INPUT, &pGrabIn);
    pGraph->lpVtbl->Connect(pGraph, pCapOut, pGrabIn);
    MyGrabberCB* cb = malloc(sizeof(MyGrabberCB)); 
    cb->lpVtbl = &MyGrabberCB_Vtbl;
    cb->ref = 1;
    cb->idx = vid_cnt++;
    pGrabber->lpVtbl->SetCallback(pGrabber, (ISampleGrabberCB*)cb, 0);
    IMediaControl *pControl = NULL;
    pGraph->lpVtbl->QueryInterface(pGraph, &IID_IMediaControl, (void**)&pControl);
    pControl->lpVtbl->Run(pControl); 
  }else if (flag & SOURCE_FREAD){
    IGraphBuilder *graph = NULL;
    IMediaControl *ctrl = NULL;
    IBaseFilter *src = NULL, *grab = NULL, *nullRend = NULL;
    ISampleGrabber *grabInt = NULL;
    CoCreateInstance(&CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
                     &IID_IGraphBuilder, (void**)&graph);
    graph->lpVtbl->AddSourceFilter(graph, wpath, L"Source", &src);
    CoCreateInstance(&CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER,
                     &IID_IBaseFilter, (void**)&grab);
    graph->lpVtbl->AddFilter(graph, grab, L"Grabber");
    grab->lpVtbl->QueryInterface(grab, &IID_ISampleGrabber, (void**)&grabInt);
    grabInt->lpVtbl->SetMediaType(grabInt, &mt);

    CoCreateInstance(&CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER,
                     &IID_IBaseFilter, (void**)&nullRend);
    graph->lpVtbl->AddFilter(graph, nullRend, L"NullRend");
    IPin *srcOut=NULL, *grabIn=NULL, *grabOut=NULL, *nullIn=NULL;
    GetPin(src, PINDIR_OUTPUT, &srcOut);
    GetPin(grab, PINDIR_INPUT, &grabIn);
    graph->lpVtbl->Connect(graph, srcOut, grabIn);
    srcOut->lpVtbl->Release(srcOut); grabIn->lpVtbl->Release(grabIn);
    GetPin(grab, PINDIR_OUTPUT, &grabOut);
    GetPin(nullRend, PINDIR_INPUT, &nullIn);
    graph->lpVtbl->Connect(graph, grabOut, nullIn);

    AM_MEDIA_TYPE mt1;
    ZeroMemory(&mt1, sizeof(mt1));
    grabInt->lpVtbl->GetConnectedMediaType(grabInt, &mt1);
    if (IsEqualGUID(&mt1.formattype,&FORMAT_VideoInfo) && (mt1.cbFormat >= sizeof(VIDEOINFOHEADER))) {
      VIDEOINFOHEADER *vih = (VIDEOINFOHEADER*)mt1.pbFormat;
      v.w = v.rw = vih->bmiHeader.biWidth;
      v.h = v.rh = vih->bmiHeader.biHeight;
    }else if (IsEqualGUID(&mt1.formattype,&FORMAT_VideoInfo2) && (mt1.cbFormat >= sizeof(VIDEOINFOHEADER2))) {
      VIDEOINFOHEADER2 *vih2 = (VIDEOINFOHEADER2*)mt1.pbFormat;
      v.w = v.rw = vih2->bmiHeader.biWidth;
      v.h = v.rh = vih2->bmiHeader.biHeight;
    }


    grabOut->lpVtbl->Release(grabOut); nullIn->lpVtbl->Release(nullIn);
    MyGrabberCB* cb = malloc(sizeof(MyGrabberCB)); 
    cb->lpVtbl = &MyGrabberCB_Vtbl;
    cb->ref = 1;
    cb->idx = vid_cnt++;
    grabInt->lpVtbl->SetCallback(grabInt, (ISampleGrabberCB*)cb, 1);
    graph->lpVtbl->QueryInterface(graph, &IID_IMediaControl, (void**)&ctrl);
    ctrl->lpVtbl->Run(ctrl);
    IMediaSeeking *pMS = NULL;
    IMediaEventEx *pME = NULL;
    graph->lpVtbl->QueryInterface(graph, &IID_IMediaSeeking, (void**)&pMS);
    graph->lpVtbl->QueryInterface(graph, &IID_IMediaEventEx, (void**)&pME);
    v.pMS = pMS;
    v.pMC = ctrl;
    v.pME = pME;
  }

  ARR_PUSH(vid_t,vids,v);
  *w = v.w;
  *h = v.h;
  return vid_cnt -1;
}

char* vin_impl__read_pixels(int idx, int* w, int* h){
  vid_t* vid = ((vid_t*)vids.data) + idx;
  char* pixels = malloc(vid->w*vid->h*4);
  if (vid->flag & SOURCE_FREAD){
    long evCode;
    LONG_PTR param1, param2;
    if (SUCCEEDED(vid->pME->lpVtbl->GetEvent(vid->pME, &evCode, &param1, &param2, 0))){
      if (evCode == EC_COMPLETE) {
        LONGLONG pos = 0;
        vid->pMS->lpVtbl->SetPositions(
          vid->pMS, &pos, AM_SEEKING_AbsolutePositioning, NULL, AM_SEEKING_NoPositioning
        );
        vid->pMC->lpVtbl->Run(vid->pMC);
      }
    }
  }
  if (vid->data){
    memcpy(pixels,vid->data,vid->w*vid->h*4);
  }
  *w = vid->w;
  *h = vid->h;
  return pixels;  
}
