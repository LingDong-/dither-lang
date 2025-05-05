
## Build Instructions

Subject to change.

### Required tools

- gcc/clang
- node.js v18+

Tested on macOS most extensively -- should also work on Linux. For windows, WSL is recommended.

### Run an example

Compile to C and run:

```
make run_c src=examples/helloworld.dh
```

Compile to JavaScript and run: (starts a http server on localhost)

```
make run_js src=examples/helloworld.dh
```

Run in VM (requires standard library compilation, see below)

```
make run_vm src=examples/helloworld.dh
```

### Configure

Edit `config.env` to configure backends for various components. Some backends are platform specific -- pick one that you have on your system.

### Compile the Standard Library

This step is only required for running scripts on the VM.

Compile all standard libraries:

```
make std_all
```

Compile one standard library:

```
make std_one lib=io
```

Currently, on non-macOS systems, you might need to install OpenGL/GLX for `win` and `gx` libraries, and portaudio for `snd` library.

### Compile the Windowing Backend

For running graphical applications locally (VM/C backends), you need to compile the windowing shared library. To compile Cocoa backend (mac only):

```
cd std/win/platform; make cocoa
```

To compile X11/GLX backend (requires X11 / XQuartz):

```
cd std/win/platform; make glx
```

### Other Commands

See the Makefile for more detailed usage.
