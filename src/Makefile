# Edit the following line to point to the JDK directory
# It must be a JDK, not a JRE !!!
JDK_HOME=/usr/lib/jvm/java-6-sun
#JDK_HOME=/usr/lib/j2sdk1.6-sun

#uncomment the following line to enable debug output from the JNI libv4l4j
#DEBUG=-DDEBUG -Wall


#
# edit anything below at your own risk
#


#LOG_CALLS         1
#LOG_LIBV4L        2
#LOG_MEMALLOC      4
#LOG_JPEG          8
#LOG_RGB           16
#LOG_V4L4J         32
#LOG_ALL           63
LOG_SOURCE=63

ifneq ($(DEBUG),)
DBG=$(DEBUG) -DLOG_SOURCE=$(LOG_SOURCE)
endif

LIBV4L_PATH=../libv4l
LIBV4L_NAME=libv4l.a

LIB=libv4l4j.so
OBJ=v4l4j_FrameGrabber.o jpeg.o rgb.o tinyjpeg.o jidctflt.o v4l4j_ImageFormatList.o v4l4j_DeviceInfo.o v4l4j_VideoDevice.o v4l4j_Control.o v4l4j_Tuner.o

CC=gcc
INCLUDES=-I$(JDK_HOME)/include -I$(JDK_HOME)/include/linux/ -I$(LIBV4L_PATH) -I.
OPTIMISATION=-O3
CFLAGS=$(INCLUDES) $(DBG) $(OPTIMISATION) -fPIC

LD=gcc
LDFLAGS=-shared  -ljpeg

all: $(LIB)

$(LIB): $(OBJ)
	$(LD) $(LDFLAGS) $(OBJ) $(LIBV4L_PATH)/$(LIBV4L_NAME) -o $(LIB)

test: $(OBJ)
	$(CC) $(CFLAGS) -DMAKETEST -c rgb.c
	$(CC) $(CFLAGS) -DMAKETEST -c jpeg.c
	$(LD) rgb.o tinyjpeg.o jidctflt.o -o rgb
	$(LD) jpeg.o -ljpeg -o jpeg

clean:
	rm -f $(OBJ) $(LIB) ./*~ jpeg rgb
