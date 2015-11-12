LOCAL_PATH := $(call my-dir)
CWD := $(shell pwd)

include $(CLEAR_VARS)

LOCAL_MODULE := mcpelauncher_testmod
LOCAL_SRC_FILES := main.cpp
LOCAL_LDLIBS := -L$(LOCAL_PATH)/../../libs/ -ldl -lmcpelauncher_mod -lminecraftpe

include $(BUILD_SHARED_LIBRARY)
