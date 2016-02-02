
VERSION_MAJOR=1
VERSION_MINOR=0

PATH_TO_LIBVIDEO=../..

BIN=light_cap

CC=gcc
OBJ=main.o jpeg.o

INCLUDES=-I$(PATH_TO_LIBVIDEO) -I$(PATH_TO_LIBVIDEO)/linux

ifneq ($(DEBUG),)
DBG=-DDEBUG -ggdb -Wall
else
OPTIMISATION=-O3
endif

CFLAGS=$(DBG) -DVER_MAJ=$(VERSION_MAJOR) -DVER_MIN=$(VERSION_MINOR) $(OPTIMISATION) $(INCLUDES)

LD=gcc
LIBPATH=-L$(PATH_TO_LIBVIDEO)
LIB=-ljpeg -lvideo -lv4lconvert -pthread
LDFLAGS=$(LIBPATH) $(LIB)


all: $(OBJ) libvideo.a
	$(LD) $(OBJ) $(LDFLAGS) -o $(BIN)

libvideo.a:
	make -C $(PATH_TO_LIBVIDEO) all

main.o: main.c utils.h log.h
	$(CC) $(CFLAGS) -c main.c

jpeg.o: jpeg.c
	$(CC) $(CFLAGS) -c jpeg.c

clean:
	rm -f *.o $(BIN)
	make -C $(PATH_TO_LIBVIDEO) clean
