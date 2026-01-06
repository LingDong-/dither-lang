@echo off
setlocal

echo [build] windows (MSVC)

nmake /f MSVC.mak ^
    build\config.h ^
    std_all ^
    build\vm.exe ^
    build\vm_dbg.exe ^
    cmdline ^
    %*

