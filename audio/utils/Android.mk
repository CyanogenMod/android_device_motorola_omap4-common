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

include $(CLEAR_VARS)

LOCAL_MODULE := libtiaudioutils

LOCAL_SRC_FILES := \
	src/Base.cpp \
	src/Pcm.cpp \
	src/NullPcm.cpp \
	src/ALSAPcm.cpp \
	src/ALSAMixer.cpp \
	src/SimpleStream.cpp \
	src/MumStream.cpp \
	src/Stream.cpp \
	src/Resampler.cpp \
	src/MonoPipe.cpp

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/include \
	external/tinyalsa/include \
	external/speex/include \
	system/media/audio_utils/include \
	system/media/audio_route/include \
	frameworks/av/include

LOCAL_SHARED_LIBRARIES := \
	liblog \
	libtinyalsa \
	libspeexresampler \
	libaudioutils \
	libaudioroute \
	libnbaio \
	libutils

LOCAL_SHARED_LIBRARIES += libstlport
include external/stlport/libstlport.mk

LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)
