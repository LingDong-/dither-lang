//CFLAGS+=$([ "$(uname)" == "Darwin" ] && echo "-framework CoreAudio -framework AudioToolbox -framework CoreServices -framework Foundation" || echo "")

#include <math.h>
#include <stdio.h>

#include <CoreAudio/CoreAudio.h>
#include <AudioToolbox/AudioToolbox.h>

int chan = 2;
int rate = 44100;
AudioUnit outputUnit;

#define BUF_SIZE 2048
float buffer[BUF_SIZE];
int64_t curi = 0;
int64_t curo = 0;

static OSStatus renderCallback(
  void *inRefCon,
  AudioUnitRenderActionFlags *ioActionFlags,
  const AudioTimeStamp *inTimeStamp,
  UInt32 inBusNumber,
  UInt32 inNumberFrames,
  AudioBufferList *ioData) {

  float *out = (float *)ioData->mBuffers[0].mData;
  for (UInt32 i = 0; i < inNumberFrames; ++i) {
    for (int j = 0; j < chan; j++){
      if (curo < curi){
        out[i*chan+j] = buffer[curo % BUF_SIZE];
        curo++;
      }else{
        out[i*chan+j] = 0.0;
      }
    }
  }

  return noErr;
}

void snd_impl_init(int _rate, int _chan){
  rate = _rate;
  chan = _chan;
  AudioStreamBasicDescription format = {
    .mSampleRate = rate,
    .mFormatID = kAudioFormatLinearPCM,
    .mFormatFlags = kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked,
    .mBytesPerPacket = chan*sizeof(float),
    .mFramesPerPacket = 1,
    .mBytesPerFrame = chan*sizeof(float),
    .mChannelsPerFrame = chan,
    .mBitsPerChannel = 8 * sizeof(float),
    .mReserved = 0
  };
  AudioComponentDescription outputDesc = {
      .componentType = kAudioUnitType_Output,
      .componentSubType = kAudioUnitSubType_DefaultOutput,
      .componentManufacturer = kAudioUnitManufacturer_Apple
  };
  AudioComponent outputComponent = AudioComponentFindNext(NULL, &outputDesc);
  AudioComponentInstanceNew(outputComponent, &outputUnit);
  AURenderCallbackStruct callbackStruct = {
      .inputProc = renderCallback,
      .inputProcRefCon = NULL
  };
  AudioUnitSetProperty(outputUnit, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input, 0, &callbackStruct, sizeof(callbackStruct));
  AudioUnitSetProperty(outputUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, &format, sizeof(format));
  AudioUnitInitialize(outputUnit);
  AudioOutputUnitStart(outputUnit);
}

void snd_impl_exit(){
  AudioOutputUnitStop(outputUnit);
  AudioUnitUninitialize(outputUnit);
  AudioComponentInstanceDispose(outputUnit);
}

int snd_impl_buffer_full(){
  int64_t n = (int64_t)BUF_SIZE - (curi-curo);
  return (n <= 0);
}

void snd_impl_put_sample(float x){

  int64_t n = (int64_t)BUF_SIZE - (curi-curo);
  if (n > 0){
    buffer[curi%BUF_SIZE] = x;
    curi++;
  }
}


