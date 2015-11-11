LOCAL_PATH := $(call my-dir)
CWD := $(shell pwd)

include $(CLEAR_VARS)

LOCAL_MODULE := empty
LOCAL_SRC_FILES := empty.c

include $(BUILD_SHARED_LIBRARY)
