#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <float.h>

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
    nanosleep(&sleep_time, NULL);
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
  nanosleep(&ts, NULL);
}


double time_impl_stamp(){
  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
}

void time_impl_local(double timestamp, int* year, int* month, int* day, int *hour, int *minute, int *second){
  time_t seconds = (time_t)timestamp;
  struct tm *t = localtime(&seconds);
  *year = t->tm_year + 1900;
  *month = t->tm_mon + 1;
  *day = t->tm_mday;
  *hour = t->tm_hour;
  *minute = t->tm_min;
  *second = t->tm_sec;

  // printf("%f %d %d %d %d %d %d\n",timestamp,*year,*month,*day,*hour,*minute,*second);
}
