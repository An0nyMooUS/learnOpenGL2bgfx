# Copyright (c) 2012-2018 Daniele Bartolini and individual contributors.
# License: https://github.com/dbartolini/crown/blob/master/LICENSE
#

UNAME := $(shell uname)
ifeq ($(UNAME), $(filter $(UNAME), Linux))
	OS=linux
else
	OS=windows
endif

GENIE=3rdparty/bx/tools/bin/$(OS)/genie
MAKE_JOBS=9

NDKABI=14
NDKCC=$(ANDROID_NDK_ARM)/bin/arm-linux-androideabi-
NDKFLAGS=--sysroot $(ANDROID_NDK_ROOT)/platforms/android-$(NDKABI)/arch-arm
NDKARCH=-march=armv7-a -mfloat-abi=softfp -mfpu=neon -mthumb -Wl,--fix-cortex-a8


build/projects/android:
	mkdir -p build/android-arm/bin
	$(GENIE) --file=scripts/genie.lua  --compiler=android-arm gmake
android-arm-debug: build/projects/android 
	$(MAKE) -j$(MAKE_JOBS) -R -C build/projects/android config=debug
android-arm-development: build/projects/android 
	$(MAKE) -j$(MAKE_JOBS) -R -C build/projects/android config=development
android-arm-release: build/projects/android 
	$(MAKE) -j$(MAKE_JOBS) -R -C build/projects/android config=release
android-arm: android-arm-debug android-arm-development android-arm-release

build/linux64/bin/texturec:
	mkdir -p build/linux64/bin
	$(MAKE) -j$(MAKE_JOBS) -R -C 3rdparty/bgfx/.build/projects/gmake-linux config=release64 texturec
	cp -r 3rdparty/bgfx/.build/linux64_gcc/bin/texturecRelease $@
build/linux64/bin/shaderc:
	mkdir -p build/linux64/bin
	$(MAKE) -j$(MAKE_JOBS) -R -C 3rdparty/bgfx/.build/projects/gmake-linux config=release64 shaderc
	cp -r 3rdparty/bgfx/.build/linux64_gcc/bin/shadercRelease $@

build/projects/linux:
	$(GENIE) --file=3rdparty/bgfx/scripts/genie.lua --with-tools --gcc=linux-gcc gmake
	$(GENIE)  --with-tools --compiler=linux-gcc gmake
linux-debug32: build/projects/linux 
	$(MAKE) -j$(MAKE_JOBS) -R -C build/projects/linux config=debug32
linux-development32: build/projects/linux 
	$(MAKE) -j$(MAKE_JOBS) -R -C build/projects/linux config=development32
linux-release32: build/projects/linux 
	$(MAKE) -j$(MAKE_JOBS) -R -C build/projects/linux config=release32
linux-debug64: build/projects/linux  build/linux64/bin/texturec build/linux64/bin/shaderc
	$(MAKE) -j$(MAKE_JOBS) -R -C build/projects/linux config=debug64
linux-development64: build/projects/linux  build/linux64/bin/texturec build/linux64/bin/shaderc
	$(MAKE) -j$(MAKE_JOBS) -R -C build/projects/linux config=development64
linux-release64: build/projects/linux  build/linux64/bin/texturec build/linux64/bin/shaderc
	$(MAKE) -j$(MAKE_JOBS) -R -C build/projects/linux config=release64
linux: linux-debug32 linux-development32 linux-release32 linux-debug64 linux-development64 linux-release64

build/projects/mingw:
	$(GENIE) --file=scripts/genie.lua  --with-tools --compiler=mingw-gcc gmake
mingw-debug32: build/projects/mingw 
	$(MAKE) -j$(MAKE_JOBS) -R -C build/projects/mingw config=debug32
mingw-development32: build/projects/mingw 
	$(MAKE) -j$(MAKE_JOBS) -R -C build/projects/mingw config=development32
mingw-release32: build/projects/mingw 
	$(MAKE) -j$(MAKE_JOBS) -R -C build/projects/mingw config=release32
mingw-debug64: build/projects/mingw 
	$(MAKE) -j$(MAKE_JOBS) -R -C build/projects/mingw config=debug64
mingw-development64: build/projects/mingw 
	$(MAKE) -j$(MAKE_JOBS) -R -C build/projects/mingw config=development64
mingw-release64: build/projects/mingw 
	$(MAKE) -j$(MAKE_JOBS) -R -C build/projects/mingw config=release64
mingw: mingw-debug32 mingw-development32 mingw-release32 mingw-debug64 mingw-development64 mingw-release64

build/win64/bin/texturec.exe:
	-mkdir "build/win64/bin"
	devenv 3rdparty/bgfx/.build/projects/vs2017/bgfx.sln /Build "Release|x64" /Project shaderc.vcxproj
	cp -r 3rdparty/bgfx/.build/win64_vs2017/bin/texturecRelease.exe $@
build/win64/bin/shaderc.exe:
	devenv 3rdparty/bgfx/.build/projects/vs2017/bgfx.sln /Build "Release|x64" /Project texturec.vcxproj
	cp -r 3rdparty/bgfx/.build/win64_vs2017/bin/shadercRelease.exe $@

build/projects/vs2017:
	$(GENIE) --file=3rdparty\\bgfx\\scripts\\genie.lua --with-tools vs2017
	$(GENIE) --with-tools --no-level-editor vs2017
windows-debug32: build/projects/vs2017 
	devenv build/projects/vs2017/crown.sln /Build "debug|Win32"
windows-development32: build/projects/vs2017 
	devenv build/projects/vs2017/crown.sln /Build "development|Win32"
windows-release32: build/projects/vs2017 
	devenv build/projects/vs2017/crown.sln /Build "release|Win32"
windows-debug64: build/projects/vs2017  build/win64/bin/texturec.exe build/win64/bin/shaderc.exe
	devenv build/projects/vs2017/crown.sln /Build "debug|x64"
windows-development64: build/projects/vs2017  build/win64/bin/texturec.exe build/win64/bin/shaderc.exe
	devenv build/projects/vs2017/crown.sln /Build "development|x64"
windows-release64: build/projects/vs2017  build/win64/bin/texturec.exe build/win64/bin/shaderc.exe
	devenv build/projects/vs2017/crown.sln /Build "release|x64"

.PHONY: rebuild-glib-resources
rebuild-glib-resources:
	$(MAKE) -j$(MAKE_JOBS) -R -C tools rebuild

tools-linux-debug64: linux-development64
	$(MAKE) -j$(MAKE_JOBS) -R -C build/projects/linux level-editor config=debug
tools-linux-release64: linux-development64
	$(MAKE) -j$(MAKE_JOBS) -R -C build/projects/linux level-editor config=release

tools-mingw-debug64: mingw-development64
	$(MAKE) -j$(MAKE_JOBS) -R -C build/projects/mingw level-editor config=debug
tools-mingw-release64: mingw-development64
	$(MAKE) -j$(MAKE_JOBS) -R -C build/projects/mingw level-editor config=release

.PHONY: docs
docs:
	$(MAKE) -C docs/ html
	doxygen docs/doxygen/Doxyfile.doxygen

.PHONY: clean
clean:
	@echo Cleaning...
	-@rm -rf build
	:@rm -rf 3rdparty/bgfx/.build
