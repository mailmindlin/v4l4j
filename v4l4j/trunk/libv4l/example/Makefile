#DEBUG=-DDEBUG -ggdb -Wall

VERSION_MAJOR=0
VERSION_MINOR=7

PATH_TO_LIBV4L2=../

BIN=light_cap

CC=gcc
OBJ=main.o

OPTIMISATION=-O3
INCLUDES=-I$(PATH_TO_LIBV4L2)
CFLAGS=$(DEBUG) -DVER_MAJ=$(VERSION_MAJOR) -DVER_MIN=$(VERSION_MINOR) $(OPTIMISATION) $(INCLUDES)

LD=gcc
LIBPATH=-L$(PATH_TO_LIBV4L2)
LIB=-ljpeg -lv4l -pthread
LDFLAGS=$(LIBPATH) $(LIB)


all: $(OBJ) libv4l.a
	$(LD) $(OBJ) $(LDFLAGS) -o $(BIN)

libv4l.a:
	make -C $(PATH_TO_LIBV4L2) all

main.o: main.c utils.h log.h
	$(CC) $(CFLAGS) -c main.c 

clean:
	rm -f *.o $(BIN)
	make -C $(PATH_TO_LIBV4L2) clean
