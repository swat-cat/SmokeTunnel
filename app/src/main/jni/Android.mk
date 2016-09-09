LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE    := uv
LOCAL_SRC_FILES := libs/$(TARGET_ARCH_ABI)/libuv.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := smoke
LOCAL_C_INCLUDES := /Users/max_ermakov/AndroidProjects/SmokeTunnel/app/src/main/jni/include/
LOCAL_ALLOW_UNDEFINED_SYMBOLS := true
LOCAL_CFLAGS := -DSMOKE_CLIENT
LOCAL_CFLAGS += -std=c++11
LOCAL_LDLIBS := -llog -ldl -L.
LOCAL_SRC_FILES := galois.c coefficient_table.cpp smoke.cpp
LOCAL_SHARED_LIBRARIES := uv
LOCAL_LDLIBS := -llog
include $(BUILD_SHARED_LIBRARY)
