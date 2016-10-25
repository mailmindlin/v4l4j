# If the env variable JDK_HOME is not defined,
# use the following one (works on Ubuntu with OpenJDK).
ifndef JDK_HOME
JDK_HOME := /usr/lib/jvm/jdk-8-oracle-arm32-vfp-hflt
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


ifneq ($(DEBUG),)
DBG := -DDEBUG -Wall -DLOG_SOURCE=$(V4L4J_LOG_SOURCE) -ggdb
else
OPTIMISATION := -O2
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
CFLAGS += $(INCLUDES) $(DBG) $(OPTIMISATION) -fPIC -fno-stack-protector -std=gnu11 -pthread -ftree-vectorize -fPIC -mcpu=native -mfpu=neon-vfpv4 -mfloat-abi=hard -fpredictive-commoning

LDFLAGS += -shared -ljpeg -L$(LIBVIDEO_PATH) -l$(LIBVIDEO_NAME) -L/usr/local/lib -lx264 -z noexecstack

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
