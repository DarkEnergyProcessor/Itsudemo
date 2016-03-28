# Itsudemo Makefile

WHERE_ZLIB?=./zlib-1.2.8
WHERE_LODEPNG?=./lodepng
WHERE_TCLAP?=./tclap-1.2.1
WHERE_HONOKAMIKU?=./HonokaMiku-2.3.1
CFLAGS?=
CXXFLAGS?=
LDFLAGS?=
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

# GCC object files
GCC_FILES=adler32.o compress.o crc32.o deflate.o infback.o inffast.o inflate.o inftrees.o trees.o uncompr.o zutil.o lodepng.o
GCC_FILES+=JP_Decrypter.o V2_Decrypter.o TEXBFetch.o TEXBLoad.o TEXBModify.o TEXBPixel.o TEXBSave.o TIMG.o Itsudemo.o
MSVC_FILES=adler32.obj compress.obj crc32.obj deflate.obj infback.obj inffast.obj inflate.obj inftrees.obj trees.obj uncompr.obj zutil.obj lodepng.obj
MSVC_FILES+=JP_Decrypter.obj V2_Decrypter.obj TEXBFetch.obj TEXBLoad.obj TEXBModify.obj TEXBPixel.obj TEXBSave.obj TIMG.obj Itsudemo.obj Info.res


all: itsudemo

itsudemo: $(GCC_FILES)
	$(RC_CMD)
	$(xPREFIX)g++ $(RELEASE_GCC_CMD) $(DEBUG_GCC_CMD) -o Itsudemo$(EXTENSION_APPEND) $(CFLAGS) $(LDFLAGS) $(GCC_FILES) $(RC_FILE)
	-rm $(GCC_FILES) $(RC_FILE)

ndk:
	$(NDK_BUILD) APP_BUILD_SCRIPT=./Android.mk NDK_APPLICATION_MK=./Application.mk NDK_PROJECT_PATH=. $(NDK_DEBUG)
	-mkdir -p bin/jni/{arm64-v8a,armeabi{,-v7a},mips{,64},x86{,_64}}{,/stripped}
	-for arch in {arm{64-v8a,eabi{,-v7a}},mips{,64},x86{,_64}}; do \
		cp obj/local/$$arch/Itsudemo bin/jni/$$arch/; \
		cp libs/$$arch/Itsudemo bin/jni/$$arch/stripped/; \
	done
	-rm -R obj
	-rm -R libs
ifeq ($(VSINSTALLDIR),)
vscmd:
	@echo "Run from Visual Studio command prompt!"
	@false
else
vscmd: $(MSVC_FILES)
	link -OUT:"bin\\vscmd\\Itsudemo.exe" -NXCOMPAT $(DEBUG_MSV_CMD) -RELEASE -SUBSYSTEM:CONSOLE $(LDFLAGS) $(MSVC_FILES)
endif

clean:
	-@rm Info.res
	-@rm $(GCC_FILES) $(MSVC_FILES)
	-@rm -R obj
	-@rm -R libs

#####################
# GCC object files  #
#####################

# zLib files.
adler32.o:
	$(xPREFIX)gcc -c $(RELEASE_GCC_CMD) $(DEBUG_GCC_CMD) $(CFLAGS) $(WHERE_ZLIB)/adler32.c

compress.o:
	$(xPREFIX)gcc -c $(RELEASE_GCC_CMD) $(DEBUG_GCC_CMD) $(CFLAGS) $(WHERE_ZLIB)/compress.c

crc32.o:
	$(xPREFIX)gcc -c $(RELEASE_GCC_CMD) $(DEBUG_GCC_CMD) $(CFLAGS) $(WHERE_ZLIB)/crc32.c

deflate.o:
	$(xPREFIX)gcc -c $(RELEASE_GCC_CMD) $(DEBUG_GCC_CMD) $(CFLAGS) $(WHERE_ZLIB)/deflate.c

infback.o:
	$(xPREFIX)gcc -c $(RELEASE_GCC_CMD) $(DEBUG_GCC_CMD) $(CFLAGS) $(WHERE_ZLIB)/infback.c

inffast.o:
	$(xPREFIX)gcc -c $(RELEASE_GCC_CMD) $(DEBUG_GCC_CMD) $(CFLAGS) $(WHERE_ZLIB)/inffast.c

inflate.o:
	$(xPREFIX)gcc -c $(RELEASE_GCC_CMD) $(DEBUG_GCC_CMD) $(CFLAGS) $(WHERE_ZLIB)/inflate.c

inftrees.o:
	$(xPREFIX)gcc -c $(RELEASE_GCC_CMD) $(DEBUG_GCC_CMD) $(CFLAGS) $(WHERE_ZLIB)/inftrees.c

trees.o:
	$(xPREFIX)gcc -c $(RELEASE_GCC_CMD) $(DEBUG_GCC_CMD) $(CFLAGS) $(WHERE_ZLIB)/trees.c

uncompr.o:
	$(xPREFIX)gcc -c $(RELEASE_GCC_CMD) $(DEBUG_GCC_CMD) $(CFLAGS) $(WHERE_ZLIB)/uncompr.c

zutil.o:
	$(xPREFIX)gcc -c $(RELEASE_GCC_CMD) $(DEBUG_GCC_CMD) $(CFLAGS) $(WHERE_ZLIB)/zutil.c

# lodepng file
lodepng.o:
	$(xPREFIX)g++ -c $(RELEASE_GCC_CMD) $(DEBUG_GCC_CMD) $(CXXFLAGS) $(WHERE_LODEPNG)/lodepng.cpp

# HonokaMiku files if HonokaMiku-enabled is used
JP_Decrypter.o:
	$(xPREFIX)g++ -c $(RELEASE_GCC_CMD) $(DEBUG_GCC_CMD) $(CXXFLAGS) $(WHERE_HONOKAMIKU)/JP_Decrypter.cc

V2_Decrypter.o:
	$(xPREFIX)g++ -c $(RELEASE_GCC_CMD) $(DEBUG_GCC_CMD) $(CXXFLAGS) $(WHERE_HONOKAMIKU)/V2_Decrypter.cc

# Itsudemo and libTEXB files
TEXBFetch.o:
	$(xPREFIX)g++ -c $(RELEASE_GCC_CMD) $(DEBUG_GCC_CMD) $(CXXFLAGS) src/TEXBFetch.cpp

TEXBLoad.o:
	$(xPREFIX)g++ -c $(RELEASE_GCC_CMD) $(DEBUG_GCC_CMD) -I$(WHERE_ZLIB) -I$(WHERE_HONOKAMIKU) $(CXXFLAGS) src/TEXBLoad.cpp

TEXBModify.o:
	$(xPREFIX)g++ -c $(RELEASE_GCC_CMD) $(DEBUG_GCC_CMD) $(CXXFLAGS) src/TEXBModify.cpp

TEXBPixel.o:
	$(xPREFIX)g++ -c $(RELEASE_GCC_CMD) $(DEBUG_GCC_CMD) $(CXXFLAGS) src/TEXBPixel.cpp

TEXBSave.o:
	$(xPREFIX)g++ -c $(RELEASE_GCC_CMD) $(DEBUG_GCC_CMD) -I$(WHERE_ZLIB) -I$(WHERE_HONOKAMIKU) $(CXXFLAGS) src/TEXBSave.cpp

TIMG.o:
	$(xPREFIX)g++ -c $(RELEASE_GCC_CMD) $(DEBUG_GCC_CMD) $(CXXFLAGS) src/TIMG.cpp

Itsudemo.o:
	$(xPREFIX)g++ -c $(RELEASE_GCC_CMD) $(DEBUG_GCC_CMD) -I$(WHERE_ZLIB) -I$(WHERE_LODEPNG) -I$(WHERE_TCLAP) -I$(WHERE_HONOKAMIKU) $(CXXFLAGS) src/Itsudemo.cpp

#####################
# MSVC object files #
#####################

# zLib files.
adler32.obj:
	cl -nologo -W3 -Zc:wchar_t $(RELEASE_MSV_CMD) -wd"4996" -D"WIN32" -D"_CONSOLE" -EHsc -c $(CFLAGS) $(WHERE_ZLIB)/adler32.c

compress.obj:
	cl -nologo -W3 -Zc:wchar_t $(RELEASE_MSV_CMD) -wd"4996" -D"WIN32" -D"_CONSOLE" -EHsc -c $(CFLAGS) $(WHERE_ZLIB)/compress.c

crc32.obj:
	cl -nologo -W3 -Zc:wchar_t $(RELEASE_MSV_CMD) -wd"4996" -D"WIN32" -D"_CONSOLE" -EHsc -c $(CFLAGS) $(WHERE_ZLIB)/crc32.c

deflate.obj:
	cl -nologo -W3 -Zc:wchar_t $(RELEASE_MSV_CMD) -wd"4996" -D"WIN32" -D"_CONSOLE" -EHsc -c $(CFLAGS) $(WHERE_ZLIB)/deflate.c

infback.obj:
	cl -nologo -W3 -Zc:wchar_t $(RELEASE_MSV_CMD) -wd"4996" -D"WIN32" -D"_CONSOLE" -EHsc -c $(CFLAGS) $(WHERE_ZLIB)/infback.c

inffast.obj:
	cl -nologo -W3 -Zc:wchar_t $(RELEASE_MSV_CMD) -wd"4996" -D"WIN32" -D"_CONSOLE" -EHsc -c $(CFLAGS) $(WHERE_ZLIB)/inffast.c

inflate.obj:
	cl -nologo -W3 -Zc:wchar_t $(RELEASE_MSV_CMD) -wd"4996" -D"WIN32" -D"_CONSOLE" -EHsc -c $(CFLAGS) $(WHERE_ZLIB)/inflate.c

inftrees.obj:
	cl -nologo -W3 -Zc:wchar_t $(RELEASE_MSV_CMD) -wd"4996" -D"WIN32" -D"_CONSOLE" -EHsc -c $(CFLAGS) $(WHERE_ZLIB)/inftrees.c

trees.obj:
	cl -nologo -W3 -Zc:wchar_t $(RELEASE_MSV_CMD) -wd"4996" -D"WIN32" -D"_CONSOLE" -EHsc -c $(CFLAGS) $(WHERE_ZLIB)/trees.c

uncompr.obj:
	cl -nologo -W3 -Zc:wchar_t $(RELEASE_MSV_CMD) -wd"4996" -D"WIN32" -D"_CONSOLE" -EHsc -c $(CFLAGS) $(WHERE_ZLIB)/uncompr.c

zutil.obj:
	cl -nologo -W3 -Zc:wchar_t $(RELEASE_MSV_CMD) -wd"4996" -D"WIN32" -D"_CONSOLE" -EHsc -c $(CFLAGS) $(WHERE_ZLIB)/zutil.c

# lodepng file
lodepng.obj:
	cl -nologo -W3 -Zc:wchar_t $(RELEASE_MSV_CMD) -wd"4996" -D"WIN32" -D"_CONSOLE" -EHsc -c $(CXXFLAGS) $(WHERE_LODEPNG)/lodepng.cpp

# HonokaMiku files if HonokaMiku-enabled is used
JP_Decrypter.obj:
	cl -nologo -W3 -Zc:wchar_t $(RELEASE_MSV_CMD) -wd"4996" -D"WIN32" -D"_CONSOLE" -EHsc -c $(CXXFLAGS) $(WHERE_HONOKAMIKU)/JP_Decrypter.cc

V2_Decrypter.obj:
	cl -nologo -W3 -Zc:wchar_t $(RELEASE_MSV_CMD) -wd"4996" -D"WIN32" -D"_CONSOLE" -EHsc -c $(CXXFLAGS) $(WHERE_HONOKAMIKU)/V2_Decrypter.cc

# Itsudemo and libTEXB files
TEXBFetch.obj:
	cl -nologo -W3 -Zc:wchar_t $(RELEASE_MSV_CMD) -wd"4996" -D"WIN32" -D"_CONSOLE" -EHsc -c $(CXXFLAGS) src/TEXBFetch.cpp

TEXBLoad.obj:
	cl -nologo -W3 -Zc:wchar_t $(RELEASE_MSV_CMD) -wd"4996" -D"WIN32" -D"_CONSOLE" -EHsc -c -I$(WHERE_ZLIB) -I$(WHERE_HONOKAMIKU) $(CXXFLAGS) src/TEXBLoad.cpp

TEXBModify.obj:
	cl -nologo -W3 -Zc:wchar_t $(RELEASE_MSV_CMD) -wd"4996" -D"WIN32" -D"_CONSOLE" -EHsc -c $(CXXFLAGS) src/TEXBModify.cpp

TEXBPixel.obj:
	cl -nologo -W3 -Zc:wchar_t $(RELEASE_MSV_CMD) -wd"4996" -D"WIN32" -D"_CONSOLE" -EHsc -c $(CXXFLAGS) src/TEXBPixel.cpp

TEXBSave.obj:
	cl -nologo -W3 -Zc:wchar_t $(RELEASE_MSV_CMD) -wd"4996" -D"WIN32" -D"_CONSOLE" -EHsc -c -I$(WHERE_ZLIB) -I$(WHERE_HONOKAMIKU) $(CXXFLAGS) src/TEXBSave.cpp

TIMG.obj:
	cl -nologo -W3 -Zc:wchar_t $(RELEASE_MSV_CMD) -wd"4996" -D"WIN32" -D"_CONSOLE" -EHsc -c $(CXXFLAGS) src/TIMG.cpp

Itsudemo.obj:
	cl -nologo -W3 -Zc:wchar_t $(RELEASE_MSV_CMD) -wd"4996" -D"WIN32" -D"_CONSOLE" -EHsc -c -I$(WHERE_ZLIB) -I$(WHERE_LODEPNG) -I$(WHERE_TCLAP) -I$(WHERE_HONOKAMIKU) $(CXXFLAGS) src/Itsudemo.cpp

Info.res:
	rc -v -l 0 Info.rc

.PHONY: all itsudemo ndk vscmd clean
