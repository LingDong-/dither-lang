APP_NAME     := DitherIDE
BUNDLE_ID    := com.ditherlang.ditheride
VERSION      := 0.0.1
CLI_TOOL     := ../../build/dither
WEB_FILES    := index.html ../../doc/api.json ../../examples
WIDTH        := 640
HEIGHT       := 720
ICON_PNG     := icon.png
ARCH         := native
BUILD_DIR    := ../../build
BUNDLE       := $(BUILD_DIR)/$(APP_NAME).app
CONTENTS     := $(BUNDLE)/Contents
MACOS_DIR    := $(CONTENTS)/MacOS
RES_DIR      := $(CONTENTS)/Resources
EXECUTABLE   := $(BUILD_DIR)/$(APP_NAME)
INFO_PLIST   := $(BUILD_DIR)/Info.plist
CC           := clang
BASE_FLAGS   := -g -O0 -fsanitize=address -fno-omit-frame-pointer -fobjc-arc -framework Cocoa -framework WebKit -framework UniformTypeIdentifiers -framework AVFoundation -Wno-deprecated-declarations 
ifeq ($(ARCH),universal)
  ARCH_FLAGS := -arch x86_64 -arch arm64
else ifeq ($(ARCH),x86_64)
  ARCH_FLAGS := -arch x86_64
else ifeq ($(ARCH),arm64)
  ARCH_FLAGS := -arch arm64
else
  ARCH_FLAGS :=
endif
CFLAGS       := $(BASE_FLAGS) $(ARCH_FLAGS) -DWK_WIDTH=$(WIDTH) -DWK_HEIGHT=$(HEIGHT)
.PHONY: all clean run

all: $(BUNDLE)
	@du -sh $(BUNDLE)

$(EXECUTABLE) : main.m
	$(CC) $(CFLAGS) main.m -o $(EXECUTABLE)

$(INFO_PLIST): Makefile
	echo '<?xml version="1.0" encoding="UTF-8"?>\
    <!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN"\
    "http://www.apple.com/DTDs/PropertyList-1.0.dtd">\
    <plist version="1.0">\
    <dict>\
      <key>CFBundleName</key><string>$(APP_NAME)</string>\
      <key>CFBundleDisplayName</key><string>$(APP_NAME)</string>\
      <key>CFBundleIdentifier</key><string>$(BUNDLE_ID)</string>\
      <key>CFBundleVersion</key><string>$(VERSION)</string>\
      <key>CFBundleShortVersionString</key><string>$(VERSION)</string>\
      <key>CFBundlePackageType</key><string>APPL</string>\
      <key>CFBundleExecutable</key><string>$(APP_NAME)</string>\
      <key>CFBundleIconFile</key><string>icon</string>\
      <key>LSMinimumSystemVersion</key><string>11.0</string>\
      <key>NSHighResolutionCapable</key><true/>\
      <key>NSSupportsAutomaticGraphicsSwitching</key><true/>\
      <key>CFBundleInfoDictionaryVersion</key><string>6.0</string>\
			<key>NSCameraUsageDescription</key><string>For interactive experiences</string>\
    </dict>\
    </plist>' > $(INFO_PLIST)

$(BUNDLE): $(INFO_PLIST) $(EXECUTABLE) $(WEB_FILES) $(CLI_TOOL)
	@rm -rf $(BUNDLE)
	@mkdir -p $(MACOS_DIR)
	@mkdir -p $(RES_DIR)
	@for f in $(WEB_FILES); do \
		if [ -d "$$f" ]; then \
			cp -r "$$f" "$(RES_DIR)/$$(basename $$f)"; \
		else \
			cp "$$f" "$(RES_DIR)/"; \
		fi \
	done
	@cp "$(CLI_TOOL)" "$(MACOS_DIR)/";
	@cp "$(EXECUTABLE)" "$(MACOS_DIR)/";
	@chmod +x "$(MACOS_DIR)/$$(basename $(CLI_TOOL))";
	@cp $(INFO_PLIST) $(CONTENTS)/Info.plist
	@echo -n "APPL????" > $(CONTENTS)/PkgInfo
	@if [ -n "$(ICON_PNG)" ] && [ -f "$(ICON_PNG)" ]; then \
		rm -rf /tmp/_icon.iconset; \
		mkdir -p /tmp/_icon.iconset; \
		sips -z 16 16     "$(ICON_PNG)" --out /tmp/_icon.iconset/icon_16x16.png      > /dev/null 2>&1; \
		sips -z 32 32     "$(ICON_PNG)" --out /tmp/_icon.iconset/icon_16x16@2x.png   > /dev/null 2>&1; \
		sips -z 32 32     "$(ICON_PNG)" --out /tmp/_icon.iconset/icon_32x32.png      > /dev/null 2>&1; \
		sips -z 64 64     "$(ICON_PNG)" --out /tmp/_icon.iconset/icon_32x32@2x.png   > /dev/null 2>&1; \
		sips -z 128 128   "$(ICON_PNG)" --out /tmp/_icon.iconset/icon_128x128.png    > /dev/null 2>&1; \
		sips -z 256 256   "$(ICON_PNG)" --out /tmp/_icon.iconset/icon_128x128@2x.png > /dev/null 2>&1; \
		sips -z 256 256   "$(ICON_PNG)" --out /tmp/_icon.iconset/icon_256x256.png    > /dev/null 2>&1; \
		sips -z 512 512   "$(ICON_PNG)" --out /tmp/_icon.iconset/icon_256x256@2x.png > /dev/null 2>&1; \
		sips -z 512 512   "$(ICON_PNG)" --out /tmp/_icon.iconset/icon_512x512.png    > /dev/null 2>&1; \
		sips -z 1024 1024 "$(ICON_PNG)" --out /tmp/_icon.iconset/icon_512x512@2x.png > /dev/null 2>&1; \
		iconutil -c icns /tmp/_icon.iconset -o "$(RES_DIR)/icon.icns"; \
		rm -rf /tmp/_icon.iconset; \
	fi
	@find $(BUNDLE) -type f | sed 's/^/  /'

run: $(BUNDLE)
	open $(BUNDLE)

clean:
	rm -rf $(BUNDLE) $(EXECUTABLE) $(INFO_PLIST)