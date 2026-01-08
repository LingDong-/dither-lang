echo [build] platform = windows (msvc)

setlocal EnableDelayedExpansion

@REM ---------- platform-specific std build ----------
cd std/win/platform && nmake wgl winuser && cd ../../../

@REM --------- download third-party headers ----------
for /f "usebackq delims=" %%L in ("third_party/pull.sh") do (
    set "line=%%L"
    if "!line:~0,4!"=="curl" (
        set "line=!line: > = -o !"
    )
    echo !line! >> "third_party/pull.bat"
)
cd third_party && call pull.bat && cd ../

@REM ---------- normal build ----------
nmake /f msvc.mak ^
    build\config.h ^
    std_all ^
    build\vm.exe ^
    build\vm_dbg.exe ^
    cmdline ^
    %*

endlocal