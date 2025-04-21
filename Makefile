ifeq ($(dbg),1)
	OPT = -DDBG
	VM_CHOICE = vm_dbg
else
	OPT = -O3
	VM_CHOICE = vm
endif
std_one:
	cd std/$(lib);\
	f=dynamic.c;\
	CFLAGS="";\
	eval $$(head -n 1 "$$f" | cut -c 3-);\
	echo $(lib) $$f ":" $$CFLAGS;\
	gcc $(OPT) $$DEADSTRIP -shared -o $${f%.*}.so -fPIC -fvisibility=hidden $$f $$CFLAGS;
std_all:
	for folder in std/*; do\
		name=$$(basename $$folder);\
		make std_one lib=$$name;\
	done;
build/vm: $(wildcard src/*)
	gcc -O3 src/run.c -lm -o build/vm;
build/vm_dbg: $(wildcard src/*)
	gcc -DDBG src/run.c -lm -o build/vm_dbg;
ir:
	node src/parser.js $(src) -o build/ir.dsm --map build/ir.map;
run_vm: build/vm build/vm_dbg ir
	build/$(VM_CHOICE) build/ir.dsm --map build/ir.map;
to_c: ir
	node src/to_c.js build/ir.dsm -o build/out.c;
run_c: to_c
	CFLAGS="";\
	eval $$(head -n 1 "build/out.c" | cut -c 3-);\
	echo $$CFLAGS;\
	gcc -I. -O3 build/out.c $$CFLAGS -o build/a.out && build/a.out;
nwedit:
	../nwjs-sdk/nwjs.app/Contents/MacOS/nwjs editor/nw
	