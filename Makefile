include config.env
ifeq ($(dbg),1)
	OPT = -DDBG
	VM_CHOICE = vm_dbg
else
	OPT = -O3
	VM_CHOICE = vm
endif
build/config.h: config.env
	rm -f build/config.h;
	@set -a; \
	. ./config.env; \
	env | grep -E '^DITHER_' | while IFS='=' read -r key val; do \
		case "$$val" in \
			[0-9]*) echo "#define $$key $$val" ;; \
			*) echo "#define $$key \"$$val\"" ;; \
		esac >> build/config.h; \
	done
std_one: build/config.h
	source config.env;\
	cd std/$(lib);\
	DEADSTRIP="-fdata-sections -ffunction-sections -Wl,--gc-sections";\
	if [ "$$(uname)" == "Darwin" ]; then\
		DEADSTRIP="-dead_strip";\
	fi;\
	f=dynamic.c;\
	eval "$$(cat cflags.txt)";\
	echo $(lib) $$f ":" $$CFLAGS;\
	gcc -include $$DITHER_ROOT/build/config.h $(OPT) $$DEADSTRIP -shared -o $${f%.*}.so -fPIC -fvisibility=hidden $$f $$CFLAGS;
std_all: build/config.h
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
	echo $$DITHER_ROOT;\
	build/$(VM_CHOICE) build/ir.dsm --map build/ir.map;
to_c: ir
	node src/to_c.js build/ir.dsm -o build/out.c;
to_js: ir
	node src/to_js.js build/ir.dsm -o build/out.js;
run_js: to_js
	mkdir -p /tmp/site/examples;\
	cp build/out.js /tmp/site;\
	cp -r examples/assets /tmp/site/examples;\
	cd /tmp/site;\
	echo '<body></body><script src="out.js"></script><script>;;(function(){var script=document.createElement("script");script.onload=function(){var stats=new Stats();document.body.appendChild(stats.dom);stats.dom.style.left=null;stats.dom.style.right=0;requestAnimationFrame(function loop(){stats.update();requestAnimationFrame(loop)});};script.src="//mrdoob.github.io/stats.js/build/stats.min.js";document.head.appendChild(script);})();</script>' > index.html;\
	npx http-server;
run_c: build/config.h to_c
	source config.env;\
	eval $$(head -n 1 "build/out.c" | cut -c 3-);\
	echo $$CFLAGS;\
	gcc -include build/config.h -I. -O3 build/out.c $$CFLAGS -o build/a.out && build/a.out;
nwedit:
	../nwjs-sdk/nwjs.app/Contents/MacOS/nwjs editor/nw
gledit:
	rm -rf /tmp/examples;\
	cp -r examples /tmp/examples;\
	gcc editor/gl/main.c $$([ "$$(uname)" == "Darwin" ] && echo "-framework OpenGL" || echo "-lgl") -lm -o build/gledit && build/gledit /tmp/examples/boids.dh;
webedit:
	node editor/site/make_site.js;\
	npx html-minifier-terser build/site.html -o build/site.min.html --collapse-whitespace --minify-js --minify-css
cmdline:
	cd editor/cmdline;\
	npx pkg package.json;\
	cp ../../build/dither /usr/local/bin
profile: ir
	FILENAME=$$(date +%s%N | shasum -a 256 | head -c 10).trace;\
	arch -arm64 zsh -c "xcrun xctrace record --template 'Time Profiler' --output /tmp/$$FILENAME --launch -- build/vm build/ir.dsm"
	open /tmp/$$FILENAME;