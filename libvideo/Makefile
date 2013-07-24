# change the value of LOG_LEVEL to alter the king of debug statements
# to be printed
#
# THE LOG SOURCE & LEVEL CAN BE CHANGED ON THE COMMAND LINE BY
# DEFINING THE LIBVIDEO_LOG_LEVEL OR LIBVIDEO_LOG_SOURCE VARIABLES.
#
# for example: 
# LIBVIDEO_LOG_LEVEL=16 LIBVIDEO_LOG_SOURCE=127 make clean all
#
# LOG_LEVEL_INFO            1
# LOG_LEVEL_DEBUG           2
# LOG_LEVEL_DEBUG1          4
# LOG_LEVEL_DEBUG2          8
# LOG_LEVEL_ERR             16
# LOG_LEVEL_ALL             31
LIBVIDEO_LOG_LEVEL ?= 31



# change the value of LOG_SOURCE to control which parts of libvideo will
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
LIBVIDEO_LOG_SOURCE ?= 127

#Installation directory prefix
PREFIX:=/usr
#Directory where libvideo.so will be installed
LIB_INSTALL_DIR := $(PREFIX)/lib
#Directory where the include files will be installed
INC_INSTALL_DIR := $(PREFIX)/include



#
# edit anything below at your own risk
#


ifneq ($(DEBUG),)
DBG := -DDEBUG -ggdb -Wall -DLIBVIDEO_LOG_LEVEL=$(LIBVIDEO_LOG_LEVEL) \
  -DLIBVIDEO_LOG_SOURCE=$(LIBVIDEO_LOG_SOURCE)
else
OPTIMISATION := -O3
endif

USER_INCLUDES := libvideo.h libvideo-err.h libvideo-palettes.h

VERSION_MAJOR := 0
VERSION_MINOR := 9
LIB_RELEASE := 0

LIB_NAME := video
LIB := lib$(LIB_NAME).so
TESTOBJ := test-capture.o dump-capture.o list-caps.o
OBJ := v4l2-input.o v4l1-input.o libvideo.o pwc-probe.o gspca-probe.o \
  qc-probe.o fps-param-probe.o v4l1-query.o v4l2-query.o v4l1-tuner.o \
  v4l2-tuner.o palettes.o

CFLAGS += $(DBG) -DVER_MAJ=$(VERSION_MAJOR) -DVER_MIN=$(VERSION_MINOR) \
  $(OPTIMISATION) -fPIC -I.

LDFLAGS += -lm -lrt -shared -ljpeg -lv4lconvert

all: $(LIB).$(LIB_RELEASE)

$(LIB).$(LIB_RELEASE): version.h $(OBJ)
	$(CC) $(OBJ) -Wl,-soname,$(LIB).$(LIB_RELEASE) \
  $(LDFLAGS) -o $(LIB).$(LIB_RELEASE)
	ln -f -s $(LIB).$(LIB_RELEASE) $(LIB)

install: $(LIB).$(LIB_RELEASE)
	mkdir -p $(DESTDIR)$(INC_INSTALL_DIR)
	install -p -m 644 $(USER_INCLUDES) $(DESTDIR)$(INC_INSTALL_DIR)
	mkdir -p $(DESTDIR)$(LIB_INSTALL_DIR)
	install -m 755 $(LIB).$(LIB_RELEASE) $(DESTDIR)$(LIB_INSTALL_DIR)
	cd $(DESTDIR)$(LIB_INSTALL_DIR) && ln -f -s $(LIB).$(LIB_RELEASE) $(LIB)

uninstall:
	for f in $(USER_INCLUDES); do rm -f $(DESTDIR)$(INC_INSTALL_DIR)/$${f}; done
	rm -f $(DESTDIR)$(LIB_INSTALL_DIR)/$(LIB)
	rm -f $(DESTDIR)$(LIB_INSTALL_DIR)/$(LIB).$(LIB_RELEASE)

test: $(LIB).$(LIB_RELEASE) $(TESTOBJ)
	$(CC) test-capture.o -lm -lrt -L. -l$(LIB_NAME) -o test-capture
	$(CC) list-caps.o -lm -lrt -L. -l$(LIB_NAME) -o list-caps
	$(CC) dump-capture.o -lm -lrt -L. -l$(LIB_NAME) -o dump-capture

version.h:
	@sh svn-version.sh

clean:
	rm -f $(OBJ) $(BIN) $(LIB) $(LIB).$(LIB_RELEASE) $(TESTOBJ) \
  ./*.o list-caps test-capture dump-capture version.h ./*~

.PHONY: all install uninstall test clean

