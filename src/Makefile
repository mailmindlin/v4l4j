# If the env variable JDK_HOME is not defined, try to autodetect it (http://stackoverflow.com/a/20653441/2759984)
ifndef JDK_HOME
JDK_HOME := $(dirname $(dirname $(readlink -f $(which javac))))
endif


#
# edit anything below at your own risk
#

# THE LOG SOURCE CAN BE CHANGED ON THE COMMAND LINE BY
# DEFINING THE V4L4J_LOG_SOURCE VARIABLE.
#
# for example: 
# V4L4J_LOG_SOURCE=63 ant clean all 
#
#LOG_CALLS         1
#LOG_LIBVIDEO      2
#LOG_MEMALLOC      4
#LOG_JPEG          8
#LOG_RGB           16
#LOG_V4L4J         32
#LOG_ALL           63
#V4L4J_LOG_SOURCE ?= 38
V4L4J_LOG_SOURCE ?= 63

ARCH ?= $(shell arch)
HARDWARE_FLAGS = 
ifneq ($(findstring x86_64, $(ARCH)),)
	LIBPIXFC ?= pixfc-sse/libpixfc-sse-x86_64.a
else ifeq ($(shell arch | grep -Eic "^i?.86"), 1)
	LIBPIXFC ?= pixfc-sse/libpixfc-sse-x86.a
else ifneq ($(findstring armv7, $(ARCH)),)
	#Detect if hard float
	ifneq ($(wildcard /lib/arm-linux-gnueabihf/.),)
		LIBPIXFC ?= pixfc-sse/libpixfc-sse-armv7-hf.a
		HARDWARE_FLAGS += -mfloat-abi=hard
	else
		LIBPIXFC ?= pixfc-sse/libpixfc-sse-armv7.a
		HARDWARE_FLAGS += -mfloat-abi=softfps
	endif
else ifneq ($(findstring armv6, $(ARCH)),)
	#Detect if hard float
	ifneq ($(wildcard /lib/arm-linux-gnueabihf/.),)
		LIBPIXFC ?= pixfc-sse/libpixfc-sse-armv6l-hardfp.a
		HARDWARE_FLAGS += -mfloat-abi=hard
	else
		LIBPIXFC ?= pixfc-sse/libpixfc-sse-armv6l-softfp.a
		HARDWARE_FLAGS += -mfloat-abi=softfp
	endif
else ifneq ($(findstring arm, $(ARCH)),)
	# Fallback for ARM
	LIBPIXFC ?= pixfc-sse/libpixfc-sse-armv6l-softfp.a
else ifeq ($(origin LIBPIXFC), undefined)
	$(error Unable to determine architecture to select the right libpixfc)
endif

ifeq ($(shell arch),$(ARCH))
	ifeq ($(findstring march, $(HARDWARE_FLAGS)),)
		HARDWARE_FLAGS += -march=native
		ifeq ($(findstring mcpu, $(HARDWARE_FLAGS)),)
			HARDWARE_FLAGS += -mcpu=native
			ifeq ($(findstring mtune, $(HARDWARE_FLAGS)),)
				HARDWARE_FLAGS += -mtune=native
			endif
		endif
	endif
	ifneq ($(findstring arm, $(ARCH)),)
		ifeq ($(findstring -mfpu, $(HARDWARE_FLAGS)),)
			CPUINFO = $(shell cat /proc/cpuinfo | grep Features)
			ifneq ($(findstring neon, $(CPUINFO)),)
				ifneq ($(findstring vfpv4, $(CPUINFO)),)
					HARDWARE_FLAGS += -mfpu=neon-vfpv4
				else ifneq ($(findstring armv8, $(ARCH)),)
					# Maybe should be neon-fp-armv8
					HARDWARE_FLAGS += -mfpu=crypto-neon-fp-armv8
				else
					HARDWARE_FLAGS += -mfpu=neon
				endif
			else ifneq ($(findstring armv8, $(ARCH)),)
				HARDWARE_FLAGS += -mfpu=fp-armv8
			else ifneq ($(findstring vfpv4, $(CPUINFO)),)
				HARDWARE_FLAGS += -mfpu=vfpv4
			else ifneq ($(findstring vfpv3d16, $(CPUINFO)),)
				HARDWARE_FLAGS += -mfpu=vfpv3-d16
			else ifneq ($(findstring vfpv3, $(CPUINFO)))
				HARDWARE_FLAGS += -mfpu=vfpv3
			else ifneq ($(findstring vfp, $(CPUINFO)),)
				HARDWARE_FLAGS += -mfpu=vfp
			endif
		endif
		ifeq ($(findstring -mfloat-abi, $(HARDWARE_FLAGS)),)
			ifneq ($(wildcard /lib/arm-linux-gnueabihf/.),)
				HARDWARE_FLAGS += -mfloat-abi=hard
			else
				HARDWARE_FLAGS += -mfloat-abi=softfp
			endif
		endif
	endif
endif


ifneq ($(DEBUG),)
DBG := -DDEBUG -Wall -DLOG_SOURCE=$(V4L4J_LOG_SOURCE) -ggdb
else
OPTIMISATION := -Os
endif

LIBVIDEO_NAME := video
LIBVIDEO_PATH := ../libvideo


LIB := libv4l4j.so
OBJ := jpeg.o jniutils.o \
	v4l4j_FrameGrabber.o v4l4j_ImageFormatList.o \
	v4l4j_DeviceInfo.o v4l4j_VideoDevice.o v4l4j_Control.o v4l4j_Tuner.o \
	v4l4j_ResolutionInfo.o v4l4j_FrameInterval.o \
	au_edu_jcu_v4l4j_encoder_V4lconvertBuffer.o au_edu_jcu_v4l4j_encoder_ImageFormatConverter.o
	
#  v4l4j_H264Parameters.o v4l4j_H264Picture.o v4l4j_H264Encoder.o


INCLUDES := -I$(JDK_HOME)/include -I$(JDK_HOME)/include/linux/ \
  -I$(LIBVIDEO_PATH) -I$(LIBVIDEO_PATH)/pixfc-sse -I. -I/usr/local/include
CFLAGS += $(HARDWARE_FLAGS) $(INCLUDES) $(DBG) $(OPTIMISATION) -fPIC -fno-stack-protector -std=gnu11 -pthread -ftree-vectorize -fPIC -fpredictive-commoning -fdiagnostics-color=auto -flto

LDFLAGS += $(HARDWARE_FLAGS) -shared -ljpeg -L$(LIBVIDEO_PATH) -l$(LIBVIDEO_NAME) -L/usr/local/lib -z noexecstack -flto=3

all: $(LIB)

$(LIB): $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) -o $(LIB)

test-debug:  $(LIBVIDEO_PATH)/lib$(LIBVIDEO_NAME).so
	$(CC) $(INCLUDES) -DDEBUG -Wall -ggdb -DMAKETEST -c jpeg.c
	$(LD) jpeg.o -ljpeg -L$(LIBVIDEO_PATH) -l$(LIBVIDEO_NAME) -o jpeg

test: $(LIBVIDEO_PATH)/lib$(LIBVIDEO_NAME).so
	$(CC) $(INCLUDES) -O3 -Wall -DMAKETEST -c jpeg.c
	$(LD) jpeg.o -ljpeg -L$(LIBVIDEO_PATH) -l$(LIBVIDEO_NAME) -o jpeg


clean:
	rm -f $(OBJ) $(LIB) ./*~ jpeg

.PHONY: all test clean
