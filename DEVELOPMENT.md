
## Build Instructions

Subject to change.

Works on mac, windows and linux. On windows, the `make` command should be replaced with

```
nmake /f msvc.mak
```

followed by the same arguments.

### Required tools

- gcc/clang/msvc
- node.js v18+

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
make std_one src=io
```

### Compile the Windowing Backend

For running graphical applications locally (VM/C backends), you need to compile the windowing shared library. To compile Cocoa backend (mac only):

```
cd std/win/platform; make cocoa
```

To compile X11/GLX backend (requires X11 / XQuartz):

```
cd std/win/platform; make glx
```

### Compile the commandline tool as single binary

Requires node pkg. Requires calling `make std_all` and compiling windowing backend first.

```
make cmdline
```

Now the `dither` command will be available globally on your system.

### Other Commands

See the Makefile for more detailed usage.
