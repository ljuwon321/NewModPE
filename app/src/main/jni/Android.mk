LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE    := com.rmpi.newmodpe
LOCAL_SRC_FILES_RAW := $(wildcard $(LOCAL_PATH)/*.cpp) $(wildcard $(LOCAL_PATH)/*.c)
LOCAL_SRC_FILES := $(LOCAL_SRC_FILES_RAW:$(LOCAL_PATH)/%=%)
LOCAL_LDLIBS    := -L$(LOCAL_PATH) -llog -ldl -lminecraftpe -lmcpelauncher_new -lmcpelauncher_tinysubstrate
include $(BUILD_SHARED_LIBRARY)