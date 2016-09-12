# Copyright (C) 2015 The CyanogenMod Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

LOCAL_PATH := $(ANDROID_BUILD_TOP)

# VectorImpl

include $(CLEAR_VARS)

LU_PATH := system/core/libutils
LOC_PATH := device/motorola/msm8960_jbbl-common/libmotou

LOCAL_SRC_FILES := \
        $(LU_PATH)/BasicHashtable.cpp \
        $(LU_PATH)/BlobCache.cpp \
        $(LU_PATH)/CallStack.cpp \
        $(LU_PATH)/FileMap.cpp \
        $(LU_PATH)/JenkinsHash.cpp \
        $(LU_PATH)/LinearTransform.cpp \
        $(LU_PATH)/Log.cpp \
        $(LU_PATH)/Looper.cpp \
        $(LU_PATH)/NativeHandle.cpp \
        $(LU_PATH)/Printer.cpp \
        $(LU_PATH)/ProcessCallStack.cpp \
        $(LU_PATH)/PropertyMap.cpp \
        $(LU_PATH)/RefBase.cpp \
        $(LU_PATH)/SharedBuffer.cpp \
        $(LU_PATH)/Static.cpp \
        $(LU_PATH)/StopWatch.cpp \
        $(LU_PATH)/String8.cpp \
        $(LU_PATH)/String16.cpp \
        $(LU_PATH)/SystemClock.cpp \
        $(LU_PATH)/Threads.cpp \
        $(LU_PATH)/Timers.cpp \
        $(LU_PATH)/Tokenizer.cpp \
        $(LU_PATH)/Trace.cpp \
        $(LU_PATH)/Unicode.cpp \
        $(LU_PATH)/misc.cpp \
        $(LOC_PATH)/MotoVectorImpl.cpp

LOCAL_MODULE := libmotou
LOCAL_MODULE_TAGS := optional
LOCAL_C_INCLUDES += external/safe-iop/include
LOCAL_SHARED_LIBRARIES := libbacktrace libcutils libdl liblog

include $(BUILD_SHARED_LIBRARY)
