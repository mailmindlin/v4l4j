#Uncomment the following line to enable debug output in libv4l2
#DEBUG=-DDEBUG -ggdb -Wall

#
# edit anything below at your own risk
#


# LOG_LEVEL_INFO            1
# LOG_LEVEL_DEBUG           2
# LOG_LEVEL_DEBUG1          4
# LOG_LEVEL_DEBUG2          8
# LOG_LEVEL_ERR             16
# LOG_LEVEL_ALL             31			
LOG_LEVEL=23

# LOG_SOURCE_VIDEO_DEVICE   1
# LOG_SOURCE_QUERY          2
# LOG_SOURCE_CAPTURE        4
# LOG_SOURCE_CONTROL        8
# LOG_SOURCE_MEMALLOC       16
# LOG_SOURCE_CTRL_PROBE     32
# LOG_SOURCE_TUNER          64
# LOG_SOURCE_ALL            127
LOG_SOURCE=63

ifneq ($(DEBUG),)
DBG=$(DEBUG) -DLIBV4L_LOG_LEVEL=$(LOG_LEVEL) -DLIBV4L_LOG_SOURCE=$(LOG_SOURCE)
endif

VERSION_MAJOR=0
VERSION_MINOR=7
VERSION_RELEASE=1

LIB=libv4l.a
TESTOBJ=test.o list-caps.o
OBJ=v4l2-input.o v4l1-input.o libv4l.o pwc-probe.o gspca-probe.o qc-probe.o fps-param-probe.o v4l1-query.o v4l2-query.o v4l1-tuner.o v4l2-tuner.o

CC=gcc
OPTIMISATION=-O3
CFLAGS=$(DBG) -DVER_MAJ=$(VERSION_MAJOR) -DVER_MIN=$(VERSION_MINOR) -DVER_REL=$(VERSION_RELEASE) $(OPTIMISATION) -fPIC -I.

LD=gcc
LDFLAGS=

all: $(LIB)

$(LIB): $(OBJ)
	ar rc $(LIB) $(OBJ) ; ranlib $(LIB)

test: $(TESTOBJ) $(OBJ)
	$(LD) $(LDFLAGS) test.o $(OBJ) -o test-capture
	$(LD) $(LDFLAGS) list-caps.o $(OBJ) -o list-caps

clean:
	rm -f $(OBJ) $(BIN) $(LIB) $(TESTOBJ) list-caps test-capture test-list-caps ./*~
