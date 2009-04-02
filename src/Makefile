# Edit the following line to point to the JDK directory
# It must be a JDK, not a JRE !!!
JDK_HOME=/usr/lib/jvm/java-6-sun
#JDK_HOME=/usr/lib/j2sdk1.6-sun


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
#LOG_LIBV4L        2
#LOG_MEMALLOC      4
#LOG_JPEG          8
#LOG_RGB           16
#LOG_V4L4J         32
#LOG_ALL           63
ifneq ($(V4L4J_LOG_SOURCE),)
LSRC=$(V4L4J_LOG_SOURCE)
else
LSRC=38
endif

ifneq ($(DEBUG),)
DBG=-DDEBUG -Wall -DLOG_SOURCE=$(LSRC)
else
OPTIMISATION=-O3
endif

LIBVIDEO_PATH=../libvideo
LIBVIDEO_NAME=libvideo.a

LIBV4L_CONV_PATH=../libvideo
LIBV4L_CONV_NAME=libv4lconvert.a


LIB=libv4l4j.so
OBJ=jpeg.o rgb.o v4l4j_FrameGrabber.o v4l4j_ImageFormatList.o v4l4j_DeviceInfo.o v4l4j_VideoDevice.o v4l4j_Control.o v4l4j_Tuner.o

CC=gcc
INCLUDES=-I$(JDK_HOME)/include -I$(JDK_HOME)/include/linux/ -I$(LIBVIDEO_PATH) -I.
CFLAGS=$(INCLUDES) $(DBG) $(OPTIMISATION) -fPIC

LD=gcc
LDFLAGS=-shared  -ljpeg

all: $(LIB)

$(LIB): $(OBJ)
	$(LD) $(LDFLAGS) $(OBJ) $(LIBVIDEO_PATH)/$(LIBVIDEO_NAME) $(LIBV4L_CONV_PATH)/$(LIBV4L_CONV_NAME) -o $(LIB)

test:
	$(CC) $(INCLUDES) -DDEBUG -Wall -ggdb -DMAKETEST -c rgb.c
	$(CC) $(CFLAGS) -DDEBUG -Wall -ggdb -DMAKETEST -c jpeg.c
	$(LD) rgb.o -ljpeg -o rgb
	$(LD) jpeg.o -ljpeg -o jpeg

clean:
	rm -f $(OBJ) $(LIB) ./*~ jpeg rgb
