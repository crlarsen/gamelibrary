LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_ARM_MODE := arm
LOCAL_MODULE := openal
SRC_FILES := $(wildcard $(LOCAL_PATH)/../../../common/openal/*.c)
SRC_FILES := $(SRC_FILES:$(LOCAL_PATH)/%=%) 
LOCAL_SRC_FILES = $(SRC_FILES)
LOCAL_LDLIBS = -llog
LOCAL_CFLAGS := -O3 -mno-thumb
LOCAL_CPPFLAGS := -O3 -mno-thumb
include $(BUILD_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_ARM_MODE := arm
LOCAL_MODULE   := templateApp
LOCAL_CFLAGS   := -O3 -mno-thumb 
LOCAL_CPPFLAGS := $(LOCAL_CFLAGS)
LOCAL_C_INCLUDES = $(LOCAL_PATH)/../../../common/openal/
SRC_FILES := $(wildcard $(LOCAL_PATH)/../../*.cpp)
SRC_FILES += $(wildcard $(LOCAL_PATH)/../../../common/*.cpp)
SRC_FILES += $(wildcard $(LOCAL_PATH)/../../../common/bullet/*.cpp)
SRC_FILES += $(wildcard $(LOCAL_PATH)/../../../common/ttf/*.cpp)
SRC_FILES += $(wildcard $(LOCAL_PATH)/../../../common/nvtristrip/*.cpp)
SRC_FILES += $(wildcard $(LOCAL_PATH)/../../../common/png/*.c)
SRC_FILES += $(wildcard $(LOCAL_PATH)/../../../common/zlib/*.c)
SRC_FILES += $(wildcard $(LOCAL_PATH)/../../../common/detour/*.cpp)
SRC_FILES += $(wildcard $(LOCAL_PATH)/../../../common/recast/*.cpp)
SRC_FILES += $(wildcard $(LOCAL_PATH)/../../../common/vorbis/*.c)
SRC_FILES := $(SRC_FILES:$(LOCAL_PATH)/%=%)
LOCAL_SRC_FILES := $(SRC_FILES)
LOCAL_SHARED_LIBRARIES := libopenal
LOCAL_LDLIBS := -L$(LOCAL_PATH)/../libs/armeabi 
LOCAL_LDLIBS += -lz -lm -ldl -lGLESv2 -lEGL -llog -lopenal
LOCAL_CFLAGS := -O3 -mno-thumb -Wno-write-strings
LOCAL_CPPFLAGS := -O3 -mno-thumb -Wno-write-strings
include $(BUILD_SHARED_LIBRARY)
