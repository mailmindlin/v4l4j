# Edit the following line to point to the JDK directory
# It must be a JDK, not a JRE !!!
JDK_HOME=/usr/lib/jvm/java-6-sun
#JDK_HOME=/usr/lib/j2sdk1.6-sun

#uncomment the following line to enable debug output from libv4l4j
#DEBUG=-DDEBUG


#
# edit anything below at your own risk
#
LIBV4L_PATH=../libv4l
LIBV4L_NAME=libv4l.a

LIB=libv4l4j.so
OBJ=v4l4j_FrameGrabber.o jpeg.o 

CC=gcc
INCLUDES=-I$(JDK_HOME)/include -I$(JDK_HOME)/include/linux/ -I$(LIBV4L_PATH) -I.
OPTIMISATION=-O3
CFLAGS=$(INCLUDES) $(DEBUG) $(OPTIMISATION)

LD=gcc
LDFLAGS=-shared  -ljpeg

all: $(LIB)

$(LIB): $(OBJ)
	$(LD) $(LDFLAGS) $(OBJ) $(LIBV4L_PATH)/$(LIBV4L_NAME) -o $(LIB)

clean:
	rm -f $(OBJ) $(LIB) ./*~
