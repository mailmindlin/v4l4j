# Edit the following line to point to the JDK directory
# It must be a JDK, not a JRE !!!
JDK_HOME=/usr/lib/jvm/java-6-sun

#uncomment the following line to enable debug output from libv4l4j
#DEBUG=-DDEBUG


#
# edit anything below at your own risk
#
LIBV4L2_PATH=../libv4l2

LIB=libv4l4j.so
OBJ=v4l4j_FrameGrabber.o $(LIBV4L2_PATH)/libv4l2.a

CC=gcc
INCLUDES=-I$(JDK_HOME)/include -I$(JDK_HOME)/include/linux/ -I$(LIBV4L2_PATH)
OPTIMISATION=-O3
CFLAGS=$(INCLUDES) $(DEBUG) $(OPTIMISATION)

LD=gcc
LDFLAGS=-shared -ljpeg

all: $(LIB)

$(LIB): $(OBJ)
	$(LD) $(LDFLAGS) v4l4j_FrameGrabber.o $(LIBV4L2_PATH)/libv4l2.a -o $(LIB)

v4l4j_FrameGrabber.o: v4l4j_FrameGrabber.c
	$(CC) $(CFLAGS) -c v4l4j_FrameGrabber.c

clean:
	rm -f $(OBJ) $(LIB) ./*~
