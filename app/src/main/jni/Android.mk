LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := uv
LOCAL_SRC_FILES := libs/$(TARGET_ARCH_ABI)/libuv/libuv.a
LOCAL_LDLIBS := -llog
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := galois
LOCAL_SRC_FILES := libs/$(TARGET_ARCH_ABI)/galois/libgalois.so
LOCAL_LDLIBS := -llog
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := smoker
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include/ -all
LOCAL_CPP_FEATURES := rtti
LOCAL_CPPFLAGS :=  -std=c++11 -DSMOKE_CLIENT
LOCAL_LDLIBS := -llog
LOCAL_SRC_FILES := coefficient_table.cpp smoke.cpp smoker.cpp`
LOCAL_STATIC_LIBRARIES := uv galois
LOCAL_LDLIBS := -llog
include $(BUILD_SHARED_LIBRARY)
