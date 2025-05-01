export DITHER_ROOT := $(shell pwd)

ifeq ($(dbg),1)
	OPT = -DDBG
	VM_CHOICE = vm_dbg
else
	OPT = -O3
	VM_CHOICE = vm
endif
std_one:
	cd std/$(lib);\
	DEADSTRIP="-fdata-sections -ffunction-sections -Wl,--gc-sections";\
	if [ "$$(uname)" == "Darwin" ]; then\
		DEADSTRIP="-dead_strip";\
	fi;\
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
to_js: ir
	node src/to_js.js build/ir.dsm -o build/out.js;
run_js: to_js
	mkdir -p /tmp/site;\
	cp build/out.js /tmp/site;\
	cd /tmp/site;\
	echo '<body></body><script src="out.js"></script><script>;;(function(){var script=document.createElement("script");script.onload=function(){var stats=new Stats();document.body.appendChild(stats.dom);stats.dom.style.left=null;stats.dom.style.right=0;requestAnimationFrame(function loop(){stats.update();requestAnimationFrame(loop)});};script.src="//mrdoob.github.io/stats.js/build/stats.min.js";document.head.appendChild(script);})();</script>' > index.html;\
	npx http-server;
run_c: to_c
	CFLAGS="";\
	eval $$(head -n 1 "build/out.c" | cut -c 3-);\
	echo $$CFLAGS;\
	gcc -I. -O3 build/out.c $$CFLAGS -o build/a.out && build/a.out;
nwedit:
	../nwjs-sdk/nwjs.app/Contents/MacOS/nwjs editor/nw
gledit:
	rm -rf /tmp/examples;\
	cp -r examples /tmp/examples;\
	gcc editor/gl/main.c $$([ "$$(uname)" == "Darwin" ] && echo "-framework OpenGL" || echo "-lgl") -lm -o build/gledit && build/gledit /tmp/examples/boids.dh;
