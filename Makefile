# Itsudemo Makefile

WHERE_ZLIB?=./zlib-1.2.8
WHERE_LODEPNG?=./lodepng
WHERE_TCLAP?=./tclap-1.2.1
CFLAGS?=
NDK_BUILD ?= ndk-build

# Append dash
ifdef PREFIX
xPREFIX = $(PREFIX)-
endif

# Check if we are compiling for Windows
ifeq ($(OS),Windows_NT)
# However, if PREFIX is set, it's possible that we are cross-compiling, so don't set it if prefix is set
ifndef PREFIX
RC_CMD := windres -O coff Info.rc Info.res
RC_FILE := Info.res
else
ifneq (,$(findstring mingw32,$(PREFIX)))
RC_CMD := $(xPREFIX)windres -O coff Info.rc Info.res
RC_FILE := Info.res
else
RC_CMD :=
RC_FILE :=
endif
endif
else
ifneq (,$(findstring mingw32,$(PREFIX)))
RC_CMD := $(xPREFIX)windres -O coff Info.rc Info.res
RC_FILE := Info.res
# MinGW32 Cross compiler doesn't automatically append .exe
EXTENSION_APPEND := .exe
else
RC_CMD :=
RC_FILE :=
endif
endif

# Debug flags
RELEASE_GCC_CMD := -O3
RELEASE_MSV_CMD := -Ox -MT
DEBUG_GCC_CMD :=
DEBUG_MSV_CMD :=
NDK_DEBUG :=

all: gcc

gcc:
	$(RC_CMD)
	$(xPREFIX)g++ $(RELEASE_GCC_CMD) $(DEBUG_GCC_CMD) -I$(WHERE_ZLIB) -I$(WHERE_LODEPNG) -I$(WHERE_TCLAP) $(CFLAGS) -c lodepng/lodepng.cpp src/*.cpp
	$(xPREFIX)gcc $(RELEASE_GCC_CMD) $(DEBUG_GCC_CMD) -I$(WHERE_ZLIB) -I$(WHERE_LODEPNG) -I$(WHERE_TCLAP) $(CFLAGS) -c zlib-1.2.8/*.c
	#Something is really wrong with MinGW. Attempt 5 times to generate the executable.
	$(eval EXPAND_LINK = $(xPREFIX)g++ $(RELEASE_GCC_CMD) $(DEBUG_GCC_CMD) -I$(WHERE_ZLIB) -I$(WHERE_LODEPNG) -I$(WHERE_TCLAP) $(CFLAGS) -o Itsudemo$(EXTENSION_APPEND) *.o $(RC_FILE))
	$(EXPAND_LINK) || ($(EXPAND_LINK) || ($(EXPAND_LINK) || ($(EXPAND_LINK) || ($(EXPAND_LINK)))))
	-rm *.o $(RC_FILE)

ndk:
	$(NDK_BUILD) APP_BUILD_SCRIPT=./Android.mk NDK_APPLICATION_MK=./Application.mk NDK_PROJECT_PATH=. $(NDK_DEBUG)
	-mkdir -p bin/jni/{arm64-v8a,armeabi{,-v7a},mips{,64},x86{,_64}}{,/stripped}
	-for arch in {arm{64-v8a,eabi{,-v7a}},mips{,64},x86{,_64}}; do \
		cp obj/local/$$arch/Itsudemo bin/jni/$$arch/; \
		cp libs/$$arch/Itsudemo bin/jni/$$arch/stripped/; \
	done
	-rm -R obj
	-rm -R libs

vscmd:
	-mkdir -p bin/vscmd
	@where cl.exe
	@where link.exe
	@where rc.exe
	@cl -W3 -Zc:wchar_t $(RELEASE_MSV_CMD) -wd"4996" -D"WIN32" -D"_CONSOLE" -EHsc -c -I$(WHERE_ZLIB) -I$(WHERE_LODEPNG) -I$(WHERE_TCLAP) src\\*.c* zlib-1.2.8\\*.c lodepng\\lodepng.cpp
	@rc -v -l 0 Info.rc
	@link -OUT:"bin\\vscmd\\Itsudemo.exe" -MANIFEST -NXCOMPAT $(DEBUG_MSV_CMD) -RELEASE -SUBSYSTEM:CONSOLE *.obj Info.res
	@rm *.obj Info.res

clean:
	-rm *.obj *.o Info.res
	-rm -R obj
	-rm -R libs

debug:
	@echo Debug build.
	$(eval RELEASE_GCC_CMD = -O0)
	$(eval RELEASE_MSV_CMD = -Od -D"_DEBUG" -MTd)
	$(eval DEBUG_GCC_CMD = -g -D_DEBUG)
	$(eval DEBUG_MSV_CMD = -PDB:"bin\\vscmd\\Itsudemo.pdb" -DEBUG)
	$(eval NDK_DEBUG = NDK_DEBUG=1)

.PHONY: all gcc ndk vscmd debug
