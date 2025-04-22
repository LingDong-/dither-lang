#ifndef WINDOWING_H
#define WINDOWING_H

#define MOUSE_LEFT     1
#define MOUSE_RIGHT    2
#define MOUSE_PRESSED  1
#define MOUSE_RELEASED 2
#define MOUSE_MOVED    3
#define KEY_PRESSED    4
#define KEY_RELEASED   5
#define WHEEL_SCROLLED 6
#define WINDOW_RESIZED 7

#define KEY_F1     0xffbe
#define KEY_F2     0xffbf
#define KEY_F3     0xffc0
#define KEY_F4     0xffc1
#define KEY_F5     0xffc2
#define KEY_F6     0xffc3
#define KEY_F7     0xffc4
#define KEY_F8     0xffc5
#define KEY_F9     0xffc6
#define KEY_F10    0xffc7
#define KEY_F11    0xffc8
#define KEY_F12    0xffc9
#define KEY_LARR   0xff51
#define KEY_UARR   0xff52
#define KEY_RARR   0xff53
#define KEY_DARR   0xff54
#define KEY_LSHIFT 0xffe1
#define KEY_RSHIFT 0xffe2
#define KEY_LCTRL  0xffe3
#define KEY_RCTRL  0xffe4
#define KEY_LALT   0xffe9
#define KEY_RALT   0xffea
#define KEY_LCMD   0xffeb
#define KEY_RCMD   0xffec

typedef struct {
    int type;
    int key;
    float x;
    float y;
} event_t;

void (*windowing_init)(int, int);
event_t* (*windowing_poll)(int* n);
void (*windowing_exit)(void);

#endif