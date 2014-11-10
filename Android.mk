# Copyright (C) 2013 The CyanogenMod Project
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

ifeq ($(BOARD_VENDOR),motorola-omap4)

LOCAL_PATH := $(call my-dir)

#Creating Gralloc SymLink
include $(CLEAR_VARS)

LOCAL_MODULE := gralloc.omap4.so
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := FAKE

include $(BUILD_SYSTEM)/base_rules.mk

$(LOCAL_BUILT_MODULE): GRALLOC_FILE := gralloc.omap4430.so
$(LOCAL_BUILT_MODULE): SYMLINK := $(TARGET_OUT_VENDOR)/lib/hw/$(LOCAL_MODULE)
$(LOCAL_BUILT_MODULE): $(LOCAL_PATH)/Android.mk
$(LOCAL_BUILT_MODULE):
	$(hide) echo "Symlink: $(SYMLINK) -> $(GRALLOC_FILE)"
	$(hide) mkdir -p $(dir $@)
	$(hide) mkdir -p $(dir $(SYMLINK))
	$(hide) rm -rf $@
	$(hide) rm -rf $(SYMLINK)
	$(hide) ln -sf $(GRALLOC_FILE) $(SYMLINK)
	$(hide) touch $@

include $(call all-makefiles-under,$(LOCAL_PATH))

endif
