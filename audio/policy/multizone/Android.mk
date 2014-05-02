# Copyright (C) 2013 Texas Instruments
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

LOCAL_PATH := $(call my-dir)

# --------------------------------
#  Multizone Audio Policy Manager
# --------------------------------

include $(CLEAR_VARS)

LOCAL_MODULE := libaudiopolicy_ti_multizone
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := \
	AudioPolicyManager.cpp \
	AudioPolicyParser.cpp
LOCAL_SHARED_LIBRARIES := libcutils libutils liblog
LOCAL_STATIC_LIBRARIES := libmedia_helper
LOCAL_WHOLE_STATIC_LIBRARIES := libaudiopolicy_legacy_base

include $(BUILD_STATIC_LIBRARY)


# -----------------
#  TI Audio Policy
# -----------------

include $(CLEAR_VARS)

LOCAL_MODULE := audio_policy.$(TARGET_BOARD_PLATFORM)
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := TIAudioPolicyManager.cpp
LOCAL_SHARED_LIBRARIES := libcutils libutils liblog
LOCAL_STATIC_LIBRARIES := libmedia_helper
LOCAL_WHOLE_STATIC_LIBRARIES := libaudiopolicy_ti_multizone

include $(BUILD_SHARED_LIBRARY)
