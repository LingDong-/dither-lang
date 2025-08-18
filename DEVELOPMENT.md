## **Build Instructions**

These instructions are subject to change. They are compatible with macOS, Windows, and Linux.

On Windows, replace the `make` command with `nmake /f msvc.mak` followed by the same arguments.

### **Required Tools**

You'll need the following to get started:

* A C/C++ compiler: **GCC**, **Clang**, or **MSVC**
* **Node.js v18+**

### **Quick Start Examples**

Here are some examples to help you compile and run a basic `helloworld` file.

* **Compile to C and run:**
    `make run_c src=examples/helloworld.dh`

* **Compile to JavaScript and run:** (This starts an HTTP server on localhost)
    `make run_js src=examples/helloworld.dh`

* **Run in VM:** (Requires standard library compilation, see below)
    `make run_vm src=examples/helloworld.dh`

---

### **Advanced Configuration**

#### **Configure Backends**

To configure backends for various components, edit the `config.env` file. A backend is a specific implementation of a feature or a platform. Some backends are platform-specific, so choose one that is available on your system.

#### **Compile the Standard Library**

This step is only required if you plan to run scripts on the VM.

* **Compile all standard libraries:**
    `make std_all`
* **Compile a single standard library:**
    `make std_one src=io`

#### **Compile the Windowing Backend**

If you want to run graphical applications locally (using the VM or C backends), you need to compile a windowing shared library.

* **For macOS (Cocoa backend):**
    `cd std/win/platform; make cocoa`
* **For Linux/X11 (X11/GLX backend):**
    This requires X11 development libraries. On Debian-based systems, you can install them with `sudo apt-get install libx11-dev libgl-dev`. Then, run the following command:
    `cd std/win/platform; make glx`

---

### **Building a Single Binary**

You can compile the command-line tool into a single, standalone binary. This process requires the `pkg` tool, which is a Node.js package.

1.  First, ensure you have compiled all standard libraries by running `make std_all`.
2.  Next, compile the windowing backend as described in the previous section.
3.  Install the `pkg` tool globally:
    `npm install -g pkg`
4.  Finally, run the `make` command:
    `make cmdline`

The `dither` command will now be available globally on your system. To see all available options, you can try running `dither --help`.
