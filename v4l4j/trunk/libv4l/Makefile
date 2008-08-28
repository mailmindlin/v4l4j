#Uncomment the following line to enable debug output in libv4l2
DEBUG=-DDEBUG -ggdb

#
# edit anything below at your own risk
#
VERSION_MAJOR=0
VERSION_MINOR=5
VERSION_RELEASE=3

LIB=libv4l.a
TESTOBJ=test.o list-caps.o
OBJ=v4l2-input.o v4l-control.o v4l1-input.o libv4l.o pwc-probe.o gspca-probe.o qc-probe.o

CC=gcc
OPTIMISATION=-O3
CFLAGS=$(DEBUG) -DVER_MAJ=$(VERSION_MAJOR) -DVER_MIN=$(VERSION_MINOR) -DVER_REL=$(VERSION_RELEASE) $(OPTIMISATION) -fPIC

LD=gcc
LDFLAGS=

all: $(LIB)

$(LIB): $(OBJ)
	ar rc $(LIB) $(OBJ) ; ranlib $(LIB)

test: $(TESTOBJ) $(OBJ)
	$(LD) $(LDFLAGS) test.o $(OBJ) -o test-capture
	$(LD) $(LDFLAGS) list-caps.o $(OBJ) -o list-caps

clean:
	rm -f $(OBJ) $(BIN) $(LIB) $(TESTOBJ) list-caps test-capture ./*~
