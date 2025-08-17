#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>

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
  AVAssetReader* reader;
  AVAssetReaderTrackOutput* trackOutput;
  AVAsset* asset;
  AVAssetTrack* videoTrack;
  NSDictionary* settings;
} vid_t;

ARR_DEF(vid_t);

vid_t_arr_t vids = {0};

@interface FrameGrabber : NSObject <AVCaptureVideoDataOutputSampleBufferDelegate>
@property int name;
- (instancetype)initWithName:(int)name;
@end

@implementation FrameGrabber
- (instancetype)initWithName:(int)theName {
  if (self = [super init]) self.name = theName;
  return self;
}

void processBuffer(int idx,CMSampleBufferRef sampleBuffer){
  CVPixelBufferRef pixelBuffer = CMSampleBufferGetImageBuffer(sampleBuffer);
  CVPixelBufferLockBaseAddress(pixelBuffer, kCVPixelBufferLock_ReadOnly);
  size_t width  = CVPixelBufferGetWidth(pixelBuffer);
  size_t height = CVPixelBufferGetHeight(pixelBuffer);
  size_t bytesPerRow = CVPixelBufferGetBytesPerRow(pixelBuffer);
  void *base = CVPixelBufferGetBaseAddress(pixelBuffer);
  vid_t* vid = ((vid_t*)vids.data) + idx;
  if (vid->data == NULL){
    vid->data = malloc(vid->w*vid->h*4);
  }
  int wh = (float)width/(float)vid->w > (float)height/(float)vid->h;
  uint8_t *p = (uint8_t *)base;
  for (int y = 0; y < vid->h; y++) {
    uint8_t* row;
    if (wh){
      row = p + ((y*height)/vid->h) * bytesPerRow;
    }else{
      row = p + ((y*width)/vid->w) * bytesPerRow;
    }
    for (int x = 0; x < vid->w; x++) {
      int col;
      if (wh){
        col = ((x * height)/vid->h)*4;
      }else{
        col = ((x * width)/vid->w)*4;
      }
      int xd = x;
      if (vid->flag & EFFECT_MIRROR){
        xd = vid->w-x-1;
      }
      ((uint8_t*)(vid->data))[(y*vid->w+xd)*4+0] = row[col+2];
      ((uint8_t*)(vid->data))[(y*vid->w+xd)*4+1] = row[col+1];
      ((uint8_t*)(vid->data))[(y*vid->w+xd)*4+2] = row[col+0];
      ((uint8_t*)(vid->data))[(y*vid->w+xd)*4+3] = row[col+3];
    }
  }
  CVPixelBufferUnlockBaseAddress(pixelBuffer, kCVPixelBufferLock_ReadOnly);
}


- (void)captureOutput:(AVCaptureOutput *)output
        didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer
        fromConnection:(AVCaptureConnection *)connection{
  processBuffer(self.name,sampleBuffer);
}
@end

int vin_impl_create(int flag, char* path, int* w, int* h){
  vid_t v;
  v.data = NULL;
  v.flag = flag;

  if (flag & SOURCE_WEBCAM){
    AVCaptureDevice *device = nil;
    if (!strlen(path)){
      device = [AVCaptureDevice defaultDeviceWithMediaType:AVMediaTypeVideo];
    }else{
      #pragma clang diagnostic push
      #pragma clang diagnostic ignored "-Wdeprecated-declarations"
      for (AVCaptureDevice *dev in [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo]) {
        NSString *desiredName = [NSString stringWithUTF8String:path];
        if ([dev.localizedName containsString:desiredName]) {
          device = dev;
          break;
        }
      }
      #pragma clang diagnostic pop
    }
    NSError *err = nil;
    AVCaptureDeviceInput *input = [AVCaptureDeviceInput deviceInputWithDevice:device error:&err];
    AVCaptureSession *session = [[AVCaptureSession alloc] init];
    if (flag & RESO_VGA){
      [session setSessionPreset:AVCaptureSessionPreset640x480];
      v.w = 640; v.h = 480;
    }else if (flag & RESO_HD){
      [session setSessionPreset:AVCaptureSessionPreset1280x720];
      v.w = 1280; v.h = 720;
    }else if (flag & RESO_FHD){
      [session setSessionPreset:AVCaptureSessionPreset1920x1080];
      v.w = 1920; v.h = 1080;
    }

    [session addInput:input];

    AVCaptureVideoDataOutput *output = [[AVCaptureVideoDataOutput alloc] init];
    output.videoSettings = @{
      (NSString*)kCVPixelBufferPixelFormatTypeKey : @(kCVPixelFormatType_32BGRA)
    };
    dispatch_queue_t q = dispatch_queue_create([[device localizedName] UTF8String], NULL);
    FrameGrabber *grabber = [[FrameGrabber alloc] initWithName:(vid_cnt++)];
    [output setSampleBufferDelegate:grabber queue:q];
    [session addOutput:output];
    [session startRunning];

  }else if (flag & SOURCE_FREAD){
    NSString *filePath = [NSString stringWithUTF8String:path];
    NSURL *fileURL = [NSURL fileURLWithPath:filePath];
    AVAsset *asset = [AVAsset assetWithURL:fileURL];
    NSError *err = nil;
    AVAssetReader *reader = [AVAssetReader assetReaderWithAsset:asset error:&err];
    AVAssetTrack *videoTrack = [[asset tracksWithMediaType:AVMediaTypeVideo] firstObject];
    CGSize videoSize = videoTrack.naturalSize;
    v.w = videoSize.width;
    v.h = videoSize.height;

    NSDictionary *settings = @{
      (id)kCVPixelBufferPixelFormatTypeKey : @(kCVPixelFormatType_32BGRA)
    };
    AVAssetReaderTrackOutput *trackOutput =
    [AVAssetReaderTrackOutput assetReaderTrackOutputWithTrack:videoTrack
                              outputSettings:settings];
    [reader addOutput:trackOutput];
    [reader startReading];
    v.reader = reader;
    v.trackOutput = trackOutput;
    v.asset = asset;
    v.settings = settings;
    v.videoTrack = videoTrack;
    vid_cnt++;
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
    if (vid->reader.status == AVAssetReaderStatusReading) {
      CMSampleBufferRef sampleBuffer = [vid->trackOutput copyNextSampleBuffer];
      if (sampleBuffer){
        processBuffer(idx,sampleBuffer);
        CFRelease(sampleBuffer);
      }
    }else if (vid->reader.status == AVAssetReaderStatusCompleted) {
      NSError *err = nil;
      vid->reader = [AVAssetReader assetReaderWithAsset:vid->asset error:&err];
      vid->trackOutput = [AVAssetReaderTrackOutput assetReaderTrackOutputWithTrack:vid->videoTrack
                                                   outputSettings:vid->settings];
      [vid->reader addOutput:vid->trackOutput];
      [vid->reader startReading];
    }
  }
  if (vid->data){
    memcpy(pixels,vid->data,vid->w*vid->h*4);
  }
  *w = vid->w;
  *h = vid->h;
  return pixels;  
}
