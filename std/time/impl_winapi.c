#include <time.h>
#include <windows.h>
#include <stdio.h>
#include <float.h>
#include <stdint.h>

#define CLOCK_MONOTONIC 1
#define CLOCK_REALTIME  0

int clock_gettime(int clk_id, struct timespec* tp) {
  static LARGE_INTEGER freq;
  static BOOL initialized = FALSE;
  LARGE_INTEGER count;
  if (!initialized) {
    QueryPerformanceFrequency(&freq);
    initialized = TRUE;
  }
  if (clk_id == CLOCK_MONOTONIC) {
    QueryPerformanceCounter(&count);
    tp->tv_sec = (time_t)(count.QuadPart / freq.QuadPart);
    tp->tv_nsec = (long)(((count.QuadPart % freq.QuadPart) * 1000000000LL) / freq.QuadPart);
    return 0;
  } else if (clk_id == CLOCK_REALTIME) {
    FILETIME ft;
    ULARGE_INTEGER uli;
    GetSystemTimeAsFileTime(&ft);
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;
    uint64_t t = (uli.QuadPart - 116444736000000000ULL) * 100;
    tp->tv_sec = t / 1000000000;
    tp->tv_nsec = t % 1000000000;
    return 0;
  } else {
    return -1;
  }
}

int high_precision_sleep(const struct timespec *req) {
  static LARGE_INTEGER freq;
  LARGE_INTEGER start, now;
  double target_ns = req->tv_sec * 1e9 + req->tv_nsec;
  double elapsed_ns = 0;
  static BOOL initialized = FALSE;
  if (!initialized) {
    QueryPerformanceFrequency(&freq);
    initialized = TRUE;
  }
  QueryPerformanceCounter(&start);
  int64_t coarse_sleep_ms = (int64_t)(target_ns / 1e6);
  if (coarse_sleep_ms > 100) {
    Sleep(coarse_sleep_ms - 10);
  }
  do {
    QueryPerformanceCounter(&now);
    elapsed_ns = ((now.QuadPart - start.QuadPart) * 1e9) / freq.QuadPart;
  } while (elapsed_ns < target_ns);
  return 0;
}



float time_impl_fps(float target_fps) {
  if (target_fps < 0) target_fps = FLT_MAX;
  static struct timespec last_time = {0};
  struct timespec current_time, sleep_time;
  clock_gettime(CLOCK_MONOTONIC, &current_time);
  if (last_time.tv_sec == 0 && last_time.tv_nsec == 0) {
    last_time = current_time;
    return target_fps;
  }
  double elapsed = (current_time.tv_sec - last_time.tv_sec) +
                   (current_time.tv_nsec - last_time.tv_nsec) / 1e9;
  double target_frame_time = 1.0 / target_fps;
  double sleep_time_sec = target_frame_time - elapsed;
  sleep_time_sec -= 0.0005;

  if (sleep_time_sec > 0) {
    sleep_time.tv_sec = (time_t)sleep_time_sec;
    sleep_time.tv_nsec = (long)((sleep_time_sec - sleep_time.tv_sec) * 1e9);
    high_precision_sleep(&sleep_time);
    clock_gettime(CLOCK_MONOTONIC, &current_time);
    elapsed = (current_time.tv_sec - last_time.tv_sec) +
              (current_time.tv_nsec - last_time.tv_nsec) / 1e9;
  }
  last_time = current_time;
  return 1.0 / elapsed;
}


double time_impl_millis() {
  static struct timespec start_time = {0};
  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  if (start_time.tv_sec == 0 && start_time.tv_nsec == 0) {
      start_time = now;
  }
  double elapsed_sec = (now.tv_sec - start_time.tv_sec) +
                       (now.tv_nsec - start_time.tv_nsec) / 1e9;
  return (double)(elapsed_sec * 1000.0);
}

void time_impl_delay(float ms){
  ms -= 1;
  if (ms <= 0.0f) return;
  struct timespec ts;
  ts.tv_sec = (time_t)(ms / 1000.0f);
  ts.tv_nsec = (long)((ms - (ts.tv_sec * 1000.0f)) * 1e6f);
  high_precision_sleep(&ts);
}


double time_impl_stamp(){
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

void time_impl_local(double timestamp, int* year, int* month, int* day, int *hour, int *minute, int *second){
  struct tm tm_info;
  time_t now = time(NULL);
  localtime_s(&tm_info, &now);
  struct tm *t = &tm_info;

  *year = t->tm_year + 1900;
  *month = t->tm_mon + 1;
  *day = t->tm_mday;
  *hour = t->tm_hour;
  *minute = t->tm_min;
  *second = t->tm_sec;

  // printf("%f %d %d %d %d %d %d\n",timestamp,*year,*month,*day,*hour,*minute,*second);
}
