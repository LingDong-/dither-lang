#!/usr/bin/env bash
set -e
UNAME="$(uname -s)"
echo "[build] uname = $UNAME"
case "$UNAME" in
    Darwin)
        PLATFORM=macos
        ;;
    Linux)
        PLATFORM=linux
        ;;
    *)
        echo "Unsupported platform: $UNAME"
        exit 1
        ;;
esac
echo "[build] platform = $PLATFORM"

# ---------- platform-specific std build ----------
pushd std/win/platform > /dev/null
if [ "$PLATFORM" = "macos" ]; then
    make glcocoa coregraphics
elif [ "$PLATFORM" = "linux" ]; then
    make glx
fi
popd > /dev/null

# ---------- normal build ----------
make \
    build/config.h \
    std_all \
    build/vm \
    build/vm_dbg \
    cmdline \
    "$@"

