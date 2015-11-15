LOCAL_PATH := $(call my-dir)
CWD := $(shell pwd)

include $(CLEAR_VARS)

LOCAL_MODULE := mcpelauncher_mod
LOCAL_SRC_FILES := main.c

include $(BUILD_SHARED_LIBRARY)
