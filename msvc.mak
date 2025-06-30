!if "$(dbg)" == "1"
OPT = /DDBG
VM_CHOICE = vm_dbg.exe
!else
OPT = /O2
VM_CHOICE = vm.exe
!endif
MSVCFLAGS = /Fo:%TEMP%\a.obj /wd4098 /wd4133 /wd4047 /wd4477 /wd4311 /wd4113
build\config.bat: config.env
	node -e "const fs=require('fs');const s=fs.readFileSync('config.env').toString().replace(/\$$\(pwd\)/g, process.cwd()).split('\n').map(l=>l&&!l.startsWith('#')?'set '+l:'').join('\n');fs.writeFileSync('build\\config.bat', s);"
build\config.h: config.env
	node -e "const fs=require('fs');const s=fs.readFileSync('config.env').toString().replace(/\$$\(pwd\)/g, JSON.stringify(process.cwd())).split('\n').map(l=>l&&!l.startsWith('#')?'#define '+l.split('=').join(' '):'').join('\n');fs.writeFileSync('build\\config.h', s);"
build\vm.exe: src\run.c src\common.h src\interp.c
	cl src\run.c /Fe:build\vm.exe $(MSVCFLAGS)
build\vm_dbg.exe: src\run.c src\common.h src\interp.c
	cl src\run.c /DDBG /Fe:build\vm_dbg.exe $(MSVCFLAGS)
ir:
	node src\parser.js $(src) -o build\ir.dsm --map build\ir.map
std_one: build\config.h
	cmd /V:ON /C "set CFLAGS= && call build\config.bat && cd std\$(li6) && copy /Y cflags.txt cflags.bat && call cflags.bat && del cflags.bat && cl /FI !DITHER_ROOT!\build\config.h $(OPT) /LD dynamic.c /Fe:dynamic.dll $(MSVCFLAGS) !CFLAGS! && del dynamic.lib && del dynamic.exp"
run_vm: build\vm.exe build\vm_dbg.exe ir
	build\$(VM_CHOICE) build\ir.dsm --map build\ir.map
