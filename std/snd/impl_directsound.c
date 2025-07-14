#include <windows.h>
#include <dsound.h>
#include <math.h>
#include <stdio.h>
#include <stdint.h>

#pragma comment(lib, "dsound.lib")

#define BUF_SIZE (2048)
#define DS_BUF_SIZE (rate*chan)

int rate = 44100;
int chan = 2;
int godie = 0;

float buffer[BUF_SIZE];
int64_t curi = 0;
int64_t curo = 0;

LPDIRECTSOUND ds = NULL;
LPDIRECTSOUNDBUFFER ds_buffer = NULL;

void audio_callback(short* out, int frameCount) {
  for (int i = 0; i < frameCount; ++i) {
    for (int j = 0; j < chan; j++){
      if (curo < curi){
        out[i*chan+j] = (short)(buffer[curo % BUF_SIZE]*32767.0);
        curo++;
      }else{
        out[i*chan+j] = 0.0;
      }
    }
  }
}

DWORD WINAPI audio_thread(LPVOID arg) {
  DWORD bufSize = DS_BUF_SIZE;
  DWORD lastWriteCursor = 0;
  while (1) {
    DWORD playCursor, writeCursor;
    IDirectSoundBuffer_GetCurrentPosition(ds_buffer, &playCursor, &writeCursor);
    DWORD bytesFree = (playCursor + DS_BUF_SIZE - lastWriteCursor) % DS_BUF_SIZE;
    // bytesFree -= 512;
    int frames = curi-curo;
    DWORD writeAheadBytes = frames*2;
    if ((frames < BUF_SIZE && !godie) || writeAheadBytes > bytesFree){
      continue;
    }
    void *ptr1, *ptr2;
    DWORD size1, size2;
    if (writeAheadBytes > 0) {
      if (IDirectSoundBuffer_Lock(ds_buffer, lastWriteCursor, writeAheadBytes,
                                &ptr1, &size1, &ptr2, &size2, 0) == DS_OK) {
        if (size1 > 0) {
          audio_callback((short*)ptr1,size1/chan/2);
        }
        if (size2 > 0) {
          audio_callback((short*)ptr2,size2/chan/2);
        }
        IDirectSoundBuffer_Unlock(ds_buffer, ptr1, size1, ptr2, size2);
        lastWriteCursor = (lastWriteCursor + writeAheadBytes) % bufSize;
      }
    }
  }
  return 0;
}

void snd_impl_init(int _rate, int _chan) {
  godie = 0;
  rate = _rate;
  chan = _chan;
  if (DirectSoundCreate(NULL, &ds, NULL) != DS_OK) {
    return;
  }
  if (IDirectSound_SetCooperativeLevel(ds, GetConsoleWindow(), DSSCL_PRIORITY) != DS_OK) {
    return;
  }
  WAVEFORMATEX format = {0};
  format.wFormatTag = WAVE_FORMAT_PCM;
  format.nChannels = chan;
  format.nSamplesPerSec = rate;
  format.wBitsPerSample = 16;
  format.nBlockAlign = chan*2;
  format.nAvgBytesPerSec = rate*chan*2;
  DSBUFFERDESC desc = {0};
  desc.dwSize = sizeof(DSBUFFERDESC);
  desc.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLVOLUME | DSBCAPS_LOCSOFTWARE;
  desc.dwBufferBytes = DS_BUF_SIZE;
  desc.lpwfxFormat = &format;
  IDirectSound_CreateSoundBuffer(ds, &desc, &ds_buffer, NULL);
  void *ptr1, *ptr2;
  DWORD size1, size2;

  IDirectSoundBuffer_Play(ds_buffer, 0, 0, DSBPLAY_LOOPING);
  
  CreateThread(NULL, 0, audio_thread, NULL, 0, NULL);
}

void snd_impl_exit(){
  godie = 1;
  while ((curi - curo) > 0) {
    Sleep(1);
  }
  IDirectSoundBuffer_Stop(ds_buffer);
  IDirectSoundBuffer_Release(ds_buffer);
  IDirectSound_Release(ds);
}


int snd_impl_buffer_full(){
  int64_t n = (int64_t)BUF_SIZE - (curi-curo);
  return (n < chan);
}

void snd_impl_put_sample(float x){
  int64_t n = (int64_t)BUF_SIZE - (curi-curo);
  if (n > 0){
    buffer[curi%BUF_SIZE] = x;
    curi++;
  }
}
