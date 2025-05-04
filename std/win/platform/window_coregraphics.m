#define GL_SILENCE_DEPRECATION
#import <Cocoa/Cocoa.h>
#import <OpenGL/gl.h>

#ifndef EXPORTED
#define EXPORTED __attribute__((visibility("default")))
#endif

#include "windowing.h"

NSWindow *g_window = nil;
NSView *g_cgView = nil;

#define MAX_EVENTS 64 

int mouse_x,mouse_y;
int width,height;
CGContextRef cgCtx = NULL;
CGContextRef cgNextCtx = NULL;

event_t event_buffer[MAX_EVENTS];
event_t out_buffer[MAX_EVENTS];
int event_count = 0;

void add_event(int type, int key, float x, float y) {
  if (event_count && type == MOUSE_MOVED && event_buffer[event_count-1].type == type){
    event_buffer[event_count-1].x = x;
    event_buffer[event_count-1].y = y;
    return;
  }
  if (event_count >= MAX_EVENTS) {
    memmove(event_buffer, event_buffer + 1, sizeof(event_t) * (MAX_EVENTS - 1));
    event_count = MAX_EVENTS - 1;
  }
  event_buffer[event_count].type = type;
  event_buffer[event_count].key = key;
  event_buffer[event_count].x = x;
  event_buffer[event_count].y = y;
  event_count++;
}

int map_unichar_to_keycode(unichar key) {
  switch (key) {
    case NSF1FunctionKey:  return KEY_F1;
    case NSF2FunctionKey:  return KEY_F2;
    case NSF3FunctionKey:  return KEY_F3;
    case NSF4FunctionKey:  return KEY_F4;
    case NSF5FunctionKey:  return KEY_F5;
    case NSF6FunctionKey:  return KEY_F6;
    case NSF7FunctionKey:  return KEY_F7;
    case NSF8FunctionKey:  return KEY_F8;
    case NSF9FunctionKey:  return KEY_F9;
    case NSF10FunctionKey: return KEY_F10;
    case NSF11FunctionKey: return KEY_F11;
    case NSF12FunctionKey: return KEY_F12;
    case NSLeftArrowFunctionKey:  return KEY_LARR;
    case NSUpArrowFunctionKey:    return KEY_UARR;
    case NSRightArrowFunctionKey: return KEY_RARR;
    case NSDownArrowFunctionKey:  return KEY_DARR;
    default: return key;
  }
}

@interface MyWindowDelegate : NSObject <NSWindowDelegate>
@end

@implementation MyWindowDelegate
- (BOOL)windowShouldClose:(id)sender {
  [NSApp terminate:nil];
  return YES;
}
@end

@interface MyCGView : NSView
@end
@implementation MyCGView
- (BOOL)acceptsFirstResponder { return YES; }
- (void)drawRect:(NSRect)dirtyRect {
  CGContextRef ctx = [[NSGraphicsContext currentContext] CGContext];
  CGContextSaveGState(ctx);
  CGContextTranslateCTM(ctx,0,height);
  CGContextScaleCTM(ctx,1,-1);
  CGImageRef img = CGBitmapContextCreateImage(cgCtx);
  CGContextDrawImage(ctx, CGRectMake(0, 0, width, height), img);
  CGImageRelease(img);
  CGContextRestoreGState(ctx);
}
- (void)setFrameSize:(NSSize)newSize {
  [super setFrameSize:newSize];
  width = newSize.width;
  height = newSize.height;
  size_t bytesPerPixel = 4;
  size_t bytesPerRow = width * bytesPerPixel;
  size_t bitsPerComponent = 8;
  CGColorSpaceRef colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);
  cgNextCtx = CGBitmapContextCreate(NULL, width, height, bitsPerComponent, bytesPerRow, colorSpace, kCGImageAlphaPremultipliedLast);
  CGColorSpaceRelease(colorSpace);
}

- (void)keyDown:(NSEvent *)event {
  NSString *chars = [event characters];
  if ([chars length] > 0) {
    unichar key = [chars characterAtIndex:0];
    
    add_event(KEY_PRESSED, map_unichar_to_keycode(key), mouse_x, mouse_y);
  }
}

- (void)keyUp:(NSEvent *)event {
  NSString *chars = [event characters];
  if ([chars length] > 0) {
    unichar key = [chars characterAtIndex:0];
    add_event(KEY_RELEASED, map_unichar_to_keycode(key), mouse_x, mouse_y);
  }
}

static NSEventModifierFlags prev_modifiers = 0;

- (void)flagsChanged:(NSEvent *)event {
  NSEventModifierFlags new_mods = [event modifierFlags];
  NSEventModifierFlags old_mods = prev_modifiers;
  NSUInteger keyCode = [event keyCode];
  int x = mouse_x;
  int y = mouse_y;
  int down = (new_mods > old_mods) ? KEY_PRESSED : KEY_RELEASED;
  switch (keyCode) {
    case 56: add_event(down, KEY_LSHIFT, x, y); break;
    case 60: add_event(down, KEY_RSHIFT, x, y); break;
    case 59: add_event(down, KEY_LCTRL, x, y); break;
    case 62: add_event(down, KEY_RCTRL, x, y); break;
    case 58: add_event(down, KEY_LALT, x, y); break;
    case 61: add_event(down, KEY_RALT, x, y); break;
    case 55: add_event(down, KEY_LCMD, x, y); break;
    case 54: add_event(down, KEY_RCMD, x, y); break;
    default: break;
  }
  prev_modifiers = new_mods;
}

- (void)mouseDown:(NSEvent *)event {
  NSPoint p = [event locationInWindow];
  NSRect bounds = [self bounds];
  mouse_x = (int)p.x;
  mouse_y = (int)(bounds.size.height - p.y);
  add_event(MOUSE_PRESSED, MOUSE_LEFT, mouse_x, mouse_y); 
}
- (void)mouseUp:(NSEvent *)event {
  NSPoint p = [event locationInWindow];
  NSRect bounds = [self bounds];
  mouse_x = (int)p.x;
  mouse_y = (int)(bounds.size.height - p.y);
  add_event(MOUSE_RELEASED, MOUSE_LEFT, mouse_x, mouse_y);
}
- (void)rightMouseDown:(NSEvent *)event {
  NSPoint p = [event locationInWindow];
  NSRect bounds = [self bounds];
  mouse_x = (int)p.x;
  mouse_y = (int)(bounds.size.height - p.y);
  add_event(MOUSE_PRESSED, MOUSE_RIGHT, mouse_x, mouse_y);
}
- (void)rightMouseUp:(NSEvent *)event { 
  NSPoint p = [event locationInWindow];
  NSRect bounds = [self bounds];
  mouse_x = (int)p.x;
  mouse_y = (int)(bounds.size.height - p.y);
  add_event(MOUSE_RELEASED, MOUSE_RIGHT, mouse_x, mouse_y); 
}
- (void)otherMouseDown:(NSEvent *)event {
  NSPoint p = [event locationInWindow];
  NSRect bounds = [self bounds];
  mouse_x = (int)p.x;
  mouse_y = (int)(bounds.size.height - p.y);
  add_event(MOUSE_PRESSED, (int)[event buttonNumber] + 1, mouse_x, mouse_y); 
}
- (void)otherMouseUp:(NSEvent *)event {
  NSPoint p = [event locationInWindow];
  NSRect bounds = [self bounds];
  mouse_x = (int)p.x;
  mouse_y = (int)(bounds.size.height - p.y);
  add_event(MOUSE_RELEASED, (int)[event buttonNumber] + 1, mouse_x, mouse_y); 
}

- (void)scrollWheel:(NSEvent *)event {
  add_event(WHEEL_SCROLLED, 0, (int)[event scrollingDeltaX], (int)[event scrollingDeltaY]);
}

- (void)mouseMoved:(NSEvent *)event {
  NSPoint p = [event locationInWindow];
  NSRect bounds = [self bounds];
  mouse_x = (int)p.x;
  mouse_y = (int)(bounds.size.height - p.y);
  add_event(MOUSE_MOVED, 0, mouse_x, mouse_y);
}
- (void)mouseDragged:(NSEvent *)event {
  NSPoint p = [event locationInWindow];
  NSRect bounds = [self bounds];
  mouse_x = (int)p.x;
  mouse_y = (int)(bounds.size.height - p.y);
  add_event(MOUSE_MOVED, 0, mouse_x, mouse_y);
}
- (void)rightMouseDragged:(NSEvent *)event {
  NSPoint p = [event locationInWindow];
  NSRect bounds = [self bounds];
  mouse_x = (int)p.x;
  mouse_y = (int)(bounds.size.height - p.y);
  add_event(MOUSE_MOVED, 0, mouse_x, mouse_y);
}
- (void)otherMouseDragged:(NSEvent *)event {
  NSPoint p = [event locationInWindow];
  NSRect bounds = [self bounds];
  mouse_x = (int)p.x;
  mouse_y = (int)(bounds.size.height - p.y);
  add_event(MOUSE_MOVED, 0, mouse_x, mouse_y);
}
@end


EXPORTED void** window_init(int _width, int _height) {
  width = _width;
  height = _height;

  @autoreleasepool {
    [NSApplication sharedApplication];

    // NSRect frame = NSMakeRect(0, 0, width, height);
    NSScreen *mainScreen = [NSScreen mainScreen];
    NSRect screenRect = [mainScreen frame];
    CGFloat x = NSMidX(screenRect) - width / 2;
    CGFloat y = NSMidY(screenRect) - height / 2;
    NSRect frame = NSMakeRect(x, y, width, height);

    NSWindowStyleMask style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable;

    g_cgView = [[MyCGView alloc] initWithFrame:frame];

    g_window = [[NSWindow alloc] initWithContentRect:frame
                          styleMask:style
                          backing:NSBackingStoreBuffered
                          defer:NO];
    [g_window setContentView:g_cgView];
    [g_window setDelegate:[MyWindowDelegate new]];
    // [g_window setTitle:[NSString stringWithUTF8String:title]];
    [g_window setAcceptsMouseMovedEvents:YES];

    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    [NSApp activateIgnoringOtherApps:YES];

    [g_window makeKeyAndOrderFront:nil];
    [g_window makeMainWindow];
    [g_window makeFirstResponder:g_cgView];

    size_t bytesPerPixel = 4;
    size_t bytesPerRow = width * bytesPerPixel;
    size_t bitsPerComponent = 8;

    CGColorSpaceRef colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceSRGB);
    cgCtx = CGBitmapContextCreate(NULL, width, height, bitsPerComponent, bytesPerRow, colorSpace, kCGImageAlphaPremultipliedLast);
    CGColorSpaceRelease(colorSpace);
  }
  return (void**)&cgCtx;
}


EXPORTED event_t* window_poll(int* out_count) {
  @autoreleasepool {
    if (cgNextCtx){
      CGContextRelease(cgCtx);
      cgCtx = cgNextCtx;
      cgNextCtx = NULL;
      add_event(WINDOW_RESIZED, 0, width, height);
    }
    [g_cgView setNeedsDisplay:YES];
    
    NSEvent *event;
    while ((event = [NSApp nextEventMatchingMask:NSEventMaskAny
                      untilDate:nil
                      inMode:NSDefaultRunLoopMode
                      dequeue:YES])) {
      [NSApp sendEvent:event];
    }

    if (*out_count == 0){
      *out_count = event_count;
    }
    if (event_count){
      memcpy(out_buffer, event_buffer, (*out_count)*sizeof(event_t));
      memmove(event_buffer, event_buffer + (*out_count), (MAX_EVENTS-*out_count)*sizeof(event_t));
      event_count -= *out_count;
    }else{
      *out_count = 0;
    }
  }
  return out_buffer;
}

EXPORTED void window_exit(void) {
  CGContextRelease(cgCtx);
  [NSApp terminate:nil];
}


