LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := smoke
LOCAL_C_INCLUDES := /Users/max_ermakov/AndroidProjects/SmokeTunnel/app/src/main/jni/include/
LOCAL_ALLOW_UNDEFINED_SYMBOLS := true
LOCAL_CFLAGS := -DSMOKE_CLIENT
LOCAL_CFLAGS += -std=c++11
LOCAL_LDLIBS := -llog -ldl -L.
LOCAL_SRC_FILES := libs/$(TARGET_ARCH_ABI)/libsmoke.a
LOCAL_LDLIBS := -llog
include $(PREBUILT_STATIC_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := smoker
LOCAL_C_INCLUDES := /Users/max_ermakov/AndroidProjects/SmokeTunnel/app/src/main/jni/include/
LOCAL_ALLOW_UNDEFINED_SYMBOLS := true
LOCAL_CFLAGS := -DSMOKE_CLIENT
LOCAL_CFLAGS += -std=c++11
LOCAL_LDLIBS := -llog -ldl -L.
LOCAL_SRC_FILES := smoker.cpp
LOCAL_SHARED_LIBRARIES := uv smoke
LOCAL_LDLIBS := -llog
include $(BUILD_SHARED_LIBRARY)
