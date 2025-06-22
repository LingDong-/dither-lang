glcocoa:
	clang -O3 -dead_strip -shared -o glcocoa.so -fPIC -fvisibility=hidden window_glcocoa.m -framework Cocoa -framework OpenGL
glx:
	gcc -O3 -dead_strip -shared -o glx.so -fPIC -fvisibility=hidden window_glx.c -I/opt/X11/include -L/opt/X11/lib -lX11 -lXext -lGl
glx_linux:
	gcc -O3 -fdata-sections -ffunction-sections -Wl,--gc-sections -shared -fPIC -fvisibility=hidden window_glx.c -o glx.so -lX11 -lXext -lGL
coregraphics:
	gcc -O3 -dead_strip -shared -o coregraphics.so -fPIC -fvisibility=hidden window_coregraphics.m -framework Cocoa -framework CoreGraphics