# change the value of LOG_LEVEL to alter the king of debug statements
# to be printed
#
# LOG_LEVEL_INFO            1
# LOG_LEVEL_DEBUG           2
# LOG_LEVEL_DEBUG1          4
# LOG_LEVEL_DEBUG2          8
# LOG_LEVEL_ERR             16
# LOG_LEVEL_ALL             31
LOG_LEVEL=23


# change the value of LOG_SOURCE to control which parts of v4l4j will
# print debug statements
#
# LOG_SOURCE_VIDEO_DEVICE   1
# LOG_SOURCE_QUERY          2
# LOG_SOURCE_CAPTURE        4
# LOG_SOURCE_CONTROL        8
# LOG_SOURCE_MEMALLOC       16
# LOG_SOURCE_CTRL_PROBE     32
# LOG_SOURCE_TUNER          64
# LOG_SOURCE_ALL            127
LOG_SOURCE=63


#
# edit anything below at your own risk
#


ifneq ($(DEBUG),)
DBG=-DDEBUG -ggdb -Wall -DLIBV4L_LOG_LEVEL=$(LOG_LEVEL) -DLIBV4L_LOG_SOURCE=$(LOG_SOURCE)
else
OPTIMISATION=-O3
endif

VERSION_MAJOR=0
VERSION_MINOR=8


LIB=libv4l.a
TESTOBJ=test-capture.o dump-capture.o list-caps.o
OBJ=v4l2-input.o v4l1-input.o libv4l.o pwc-probe.o gspca-probe.o qc-probe.o fps-param-probe.o v4l1-query.o v4l2-query.o v4l1-tuner.o v4l2-tuner.o

CC=gcc
CFLAGS=$(DBG) -DVER_MAJ=$(VERSION_MAJOR) -DVER_MIN=$(VERSION_MINOR) $(OPTIMISATION) -fPIC -I.

LD=gcc
LDFLAGS=

all: $(LIB)

$(LIB): $(OBJ)
	ar rc $(LIB) $(OBJ) ; ranlib $(LIB)

test: $(TESTOBJ) $(OBJ)
	$(LD) $(LDFLAGS) test-capture.o $(OBJ) -o test-capture
	$(LD) $(LDFLAGS) list-caps.o $(OBJ) -o list-caps
	$(LD) $(LDFLAGS) dump-capture.o $(OBJ) -o dump-capture

clean:
	rm -f $(OBJ) $(BIN) $(LIB) $(TESTOBJ) list-caps test-capture dump-capture version.h ./*~
