#Uncomment the following line to enable debug output in libv4l2
#DEBUG=-DDEBUG -ggdb

#
# edit anything below at your own risk
#
VERSION_MAJOR=0
VERSION_MINOR=2

LIB=libv4l2.a
BIN=test_libv4l2
OBJ=frame.o v4l2-input.o v4l2-probepwc.o v4l2-control.o

CC=gcc
OPTIMISATION=-O3
CFLAGS=$(DEBUG) -DVER_MAJ=$(VERSION_MAJOR) -DVER_MIN=$(VERSION_MINOR) $(OPTIMISATION) -c

LD=gcc

all: $(LIB)

$(LIB): $(OBJ)
	ar rc $(LIB) $(OBJ); ranlib $(LIB)

frame.o: frame.c frame.h
	$(CC) $(CFLAGS) frame.c

v4l2-input.o: v4l2-input.c v4l2-input.h
	$(CC) $(CFLAGS) v4l2-input.c

v4l2-control.o: v4l2-control.c v4l2-input.h libv4l2_log.h v4l2-probe.h
	$(CC) $(CFLAGS) v4l2-control.c

v4l2-probepwc.o: v4l2-probepwc.c v4l2-probe.h pwc_probe.h
	$(CC) $(CFLAGS) v4l2-probepwc.c

test: test.o $(OBJ)
	$(CC) test.o $(OBJ) -o $(BIN)

test.o: test.c
	$(CC) $(CFLAGS) test.c

clean:
	rm -f $(OBJ) $(BIN) $(LIB) test.o ./*~
