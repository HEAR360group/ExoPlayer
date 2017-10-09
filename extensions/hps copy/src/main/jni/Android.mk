
WORKING_DIR := $(call my-dir)
include $(CLEAR_VARS)

########################################################################################################################

LOCAL_PATH := $(WORKING_DIR)

########################################################################################################################

warnings = -Werror -Wall -Wextra

morewarnings = -Wundef -Wpointer-arith -Wcast-qual -Wcast-align -Wwrite-strings -Wno-undef -Wno-reorder -Wno-cast-qual -Wno-unused-but-set-parameter -Wno-unused-parameter -Wno-sequence-point -Wno-unknown-pragmas -Wno-unused-variable

########################################################################################################################

hear360_plugin_files :=\
hear360/algr/Base/DSPUtils.cpp\
hear360/algr/Delay/HPSStaticDelay.cpp\
hear360/algr/Convolution/CRFilter.cpp\
hear360/algr/Equalizer/HPSEqualizerBand.cpp\
hear360/algr/Equalizer/HPSEqualizer4Band.cpp\
hear360/dsp/os/memory.cpp\
hear360/dsp/os/subnormal.cpp\
hear360/dsp/low/equalizerband.cpp\
hear360/dsp/low/monoequalizer.cpp\
hear360/dsp/low/stereoequalizer.cpp\
hear360/plugin/generic/dsp/interleave.cpp

hear360_plugin_hrir_folddown_files :=\
hear360/dsp/high/hrirfolddown.cpp\
hear360/dsp/high/hrirfolddownsimple.cpp\
hear360/dsp/high/convolutioncore.cpp\
hear360/plugin/generic/dsp/hrirfolddown.cpp\
hear360/plugin/generic/dsp/convolutioncore.cpp\
hear360/plugin/generic/dll/hps-hrirfolddown.cpp

#########################################################################################################################

include $(CLEAR_VARS)

#########################################################################################################################

LOCAL_MODULE    := libckfft_android
ifeq ($(TARGET_ARCH),arm64)
	LOCAL_SRC_FILES := libs/libckfft_android_arm64.a
else ifeq ($(TARGET_ARCH),armeabi-v7a)
	LOCAL_SRC_FILES := libs/libckfft_android_v7a.a
else
	LOCAL_SRC_FILES := libs/libckfft_android.a
endif

#########################################################################################################################

include $(PREBUILT_STATIC_LIBRARY)

#########################################################################################################################

include $(CLEAR_VARS)

########################################################################################################################

LOCAL_MODULE := hps-hrir-folddown

########################################################################################################################

LOCAL_C_INCLUDES :=

LOCAL_CPP_FEATURES := exceptions

LOCAL_CPPFLAGS := -std=c++0x

LOCAL_CFLAGS := -O3 $(warnings) $(morewarnings)

LOCAL_SRC_FILES := $(hear360_plugin_files) $(hear360_plugin_hrir_folddown_files)

LOCAL_STATIC_LIBRARIES := libckfft_android cpufeatures

########################################################################################################################

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/cpufeatures)

#########################################################################################################################
