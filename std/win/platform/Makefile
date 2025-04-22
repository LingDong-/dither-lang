cocoa:
	clang -O3 -dead_strip -shared -o windowing.so -fPIC -fvisibility=hidden window_cocoa.m -framework Cocoa -framework OpenGL
glx:
	gcc -O3 -dead_strip -shared -o windowing.so -fPIC -fvisibility=hidden window_glx.c -I/opt/X11/include -L/opt/X11/lib -lX11 -lXext -lGl