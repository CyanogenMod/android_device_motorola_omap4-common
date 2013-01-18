#  Copyright (C) 2008 The Android Open Source Project
#
#  Copyright (C) 2010 Motorola
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
#
#

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

ifeq ($(wildcard bootable/recovery/devutils),bootable/recovery/devutils)
HAVE_DEVUTILS ?= true
else
HAVE_DEVUTILS ?= false
endif

TOOLS := \
	test \
	getconfig \
	ptf

LOCAL_STATIC_LIBRARIES := libcutils

ifeq ($(HAVE_DEVUTILS),true)
  TOOLS += \
	setconfig \
	masterclear
  LOCAL_C_INCLUDES := bootable/recovery/bcbmsg
  LOCAL_SHARED_LIBRARIES += libbcbmsg
  LOCAL_CFLAGS := -DHAVE_DEVUTILS
endif

LOCAL_SRC_FILES:= \
	motobox.c \
	$(patsubst %,%.c,$(TOOLS))

LOCAL_MODULE_TAGS:= optional
LOCAL_MODULE:= motobox

# Including this will define $(intermediates).
#
include $(BUILD_EXECUTABLE)

MOTOBOX_SYMLINKS := $(addprefix $(TARGET_OUT)/bin/,$(TOOLS))
	    	
$(MOTOBOX_SYMLINKS): MOTOBOX_BINARY := $(LOCAL_MODULE)
$(MOTOBOX_SYMLINKS): $(LOCAL_INSTALLED_MODULE) $(LOCAL_PATH)/Android.mk \
			MOTOBOX_TEST_SYMLINK
	@echo "Symlink: $@ -> $(MOTOBOX_BINARY)"
	@mkdir -p $(dir $@)
	@rm -rf $@
	$(hide) ln -sf $(MOTOBOX_BINARY) $@

MOTOBOX_TEST_SYMLINK:
	@echo "Symlink: $(TARGET_OUT)/bin/[ -> $(MOTOBOX_BINARY)"
	@mkdir -p $(TARGET_OUT)/bin
	$(hide) ln -sf $(MOTOBOX_BINARY) $(TARGET_OUT)/bin/[

ALL_DEFAULT_INSTALLED_MODULES += $(MOTOBOX_SYMLINKS)

# We need this so that the installed files could be picked up based on the
# local module name
ALL_MODULES.$(LOCAL_MODULE).INSTALLED := \
    $(ALL_MODULES.$(LOCAL_MODULE).INSTALLED) $(SYMLINKS)
