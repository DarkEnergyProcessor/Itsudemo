# Itsudemo Makefile

WHERE_ZLIB?=./zlib-1.2.8
WHERE_LODEPNG?=./lodepng
WHERE_TCLAP?=./tclap-1.2.1
CFLAGS?=
NDK_BUILD ?= ndk-build

ifeq ($(OS),Windows_NT)
ADD_WINSOCK := -lws2_32
RC_CMD := windres -O coff Info.rc Info.res
RC_FILE := Info.res
else
ADD_WINSOCK := 
RC_CMD :=
RC_FILE :=
endif

all: gcc

gcc:
	$(RC_CMD)
	g++ -O3 -I$(WHERE_ZLIB) -I$(WHERE_LODEPNG) -I$(WHERE_TCLAP) $(CFLAGS) -c lodepng/lodepng.cpp src/*.cpp
	gcc -O3 -I$(WHERE_ZLIB) -I$(WHERE_LODEPNG) -I$(WHERE_TCLAP) $(CFLAGS) -c zlib-1.2.8/*.c
	g++ -O3 -I$(WHERE_ZLIB) -I$(WHERE_LODEPNG) -I$(WHERE_TCLAP) $(CFLAGS) -o Itsudemo *.o $(ADD_WINSOCK) $(RC_FILE)
	-rm *.o $(RC_FILE)

ndk:
	$(NDK_BUILD) APP_BUILD_SCRIPT=./Android.mk NDK_APPLICATION_MK=./Application.mk NDK_PROJECT_PATH=.
	-mkdir -p bin/jni/{arm64-v8a,armeabi{,-v7a},mips{,64},x86{,_64}}{,/stripped}
	-cp obj/local/arm64-v8a/Itsudemo bin/jni/arm64-v8a/
	-cp obj/local/armeabi/Itsudemo bin/jni/armeabi/
	-cp obj/local/armeabi-v7a/Itsudemo bin/jni/armeabi-v7a/
	-cp obj/local/mips/Itsudemo bin/jni/mips/
	-cp obj/local/mips64/Itsudemo bin/jni/mips64/
	-cp obj/local/x86/Itsudemo bin/jni/x86/
	-cp obj/local/x86_64/Itsudemo bin/jni/x86_64/
	rm -R obj
	-cp libs/arm64-v8a/Itsudemo bin/jni/arm64-v8a/stripped/
	-cp libs/armeabi/Itsudemo bin/jni/armeabi/stripped/
	-cp libs/armeabi-v7a/Itsudemo bin/jni/armeabi-v7a/stripped/
	-cp libs/mips/Itsudemo bin/jni/mips/stripped/
	-cp libs/mips64/Itsudemo bin/jni/mips64/stripped/
	-cp libs/x86/Itsudemo bin/jni/x86/stripped/
	-cp libs/x86_64/Itsudemo bin/jni/x86_64/stripped/
	rm -R libs

vscmd:
	-mkdir -p bin/vscmd
	@where cl.exe
	@where link.exe
	@where rc.exe
	@cl -W3 -Zc:wchar_t -Ox -wd"4996" -D"WIN32" -D"_CONSOLE" -EHsc -MT -c -I$(WHERE_ZLIB) -I$(WHERE_LODEPNG) -I$(WHERE_TCLAP) src\\*.c* zlib-1.2.8\\*.c lodepng\\lodepng.cpp
	@rc -v -l 0 Info.rc
	@link -OUT:"bin\\vscmd\\Itsudemo.exe" -MANIFEST -NXCOMPAT -PDB:"bin\\vscmd\\Itsudemo.pdb" -DEBUG -RELEASE -SUBSYSTEM:CONSOLE *.obj Info.res ws2_32.lib
	@rm *.obj Info.res

.PHONY: all
