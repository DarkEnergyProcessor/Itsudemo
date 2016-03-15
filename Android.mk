LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm
LOCAL_MODULE := Itsudemo
LOCAL_SRC_FILES := $(wildcard src/*.cpp)
LOCAL_SRC_FILES += $(wildcard zlib-1.2.8/*.c)
LOCAL_SRC_FILES += lodepng/lodepng.cpp
LOCAL_SRC_FILES += $(wildcard HonokaMiku-2.3.1/*.cc)
LOCAL_C_INCLUDES += tclap-1.2.1
LOCAL_C_INCLUDES += zlib-1.2.8
LOCAL_C_INCLUDES += lodepng
LOCAL_C_INCLUDES += HonokaMiku-2.3.1
LOCAL_CPPFLAGS := -fexceptions
LOCAL_LDLIBS := -L$(SYSROOT)/usr/lib
LOCAL_CFLAGS += -fPIE
LOCAL_LDFLAGS += -fPIE -pie

include $(BUILD_EXECUTABLE)