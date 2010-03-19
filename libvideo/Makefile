# Makefile for the Linux version of libvideo 

# change the value of LOG_LEVEL to alter the kind of debug statements
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
#Directory where the libv4l helpers will be installed
LIBV4L_HELPERS_INSTALL_DIR := $(PREFIX)/lib/v4l4j



#
# edit anything below at your own risk
#


# local directories
LINUX_DIR := linux
TEST_APP_DIR := test

ifneq ($(DEBUG),)
DBG := -DDEBUG -ggdb -Wall -DLIBVIDEO_LOG_LEVEL=$(LIBVIDEO_LOG_LEVEL) \
  -DLIBVIDEO_LOG_SOURCE=$(LIBVIDEO_LOG_SOURCE)
else
OPTIMISATION := -O3
endif

USER_INCLUDES := libvideo.h libvideo-err.h libvideo-palettes.h

VERSION_MAJOR := 0
VERSION_MINOR := 8
LIB_RELEASE := 0

LIBV4LCONVERT_DIR := $(LINUX_DIR)/libv4lconvert
LIBV4LCONVERT_AR := libv4lconvert.a
LIBV4LCONVERT_NAME := v4lconvert
LIBV4LCONVERT_HELPERS := ov511-decomp ov518-decomp

LIB_NAME := video
LIB := lib$(LIB_NAME).so
TESTOBJ := $(TEST_APP_DIR)/list-devices.o\
 $(TEST_APP_DIR)/test-capture.o\
 $(TEST_APP_DIR)/dump-capture.o $(TEST_APP_DIR)/list-caps.o
OBJ := $(LINUX_DIR)/v4l2-input.o $(LINUX_DIR)/v4l1-input.o libvideo.o  \
  $(LINUX_DIR)/pwc-probe.o $(LINUX_DIR)/gspca-probe.o $(LINUX_DIR)/qc-probe.o \
  $(LINUX_DIR)/v4l1-query.o  $(LINUX_DIR)/libvideo-linux.o\
  $(LINUX_DIR)/v4l1-ctrl.o $(LINUX_DIR)/v4l1-list.o \
  $(LINUX_DIR)/v4l2-query.o $(LINUX_DIR)/v4l1-tuner.o $(LINUX_DIR)/v4l2-tuner.o \
  $(LINUX_DIR)/v4l2-ctrl.o $(LINUX_DIR)/v4l2-list.o palettes.o

CC := gcc
CFLAGS := $(DBG) -DVER_MAJ=$(VERSION_MAJOR) -DVER_MIN=$(VERSION_MINOR) \
  $(OPTIMISATION) -fPIC -I. -I$(LINUX_DIR)

LD := gcc
LDFLAGS := -lm -lrt -shared

all: $(LIB).$(LIB_RELEASE)

$(LIB).$(LIB_RELEASE): $(LINUX_DIR)/version.h $(OBJ) $(LIBV4LCONVERT_AR)
	$(LD) $(OBJ) $(LIBV4LCONVERT_AR) -Wl,-soname,$(LIB).$(LIB_RELEASE) \
  $(LDFLAGS) -o $(LIB).$(LIB_RELEASE)
	ln -f -s $(LIB).$(LIB_RELEASE) $(LIB)

$(LIBV4LCONVERT_AR):
	LINKTYPE=static LIBDIR=$(LIBV4L_HELPERS_INSTALL_DIR) \
		make -C $(LIBV4LCONVERT_DIR)
	cp $(LIBV4LCONVERT_DIR)/$(LIBV4LCONVERT_AR) .

install: $(LIB).$(LIB_RELEASE)
	mkdir -p $(DESTDIR)$(INC_INSTALL_DIR)
	install -p -m 644 $(USER_INCLUDES) $(DESTDIR)$(INC_INSTALL_DIR)
	mkdir -p $(DESTDIR)$(LIB_INSTALL_DIR)
	install -m 755 $(LIB).$(LIB_RELEASE) $(DESTDIR)$(LIB_INSTALL_DIR)
	cd $(DESTDIR)$(LIB_INSTALL_DIR) && ln -f -s $(LIB).$(LIB_RELEASE) $(LIB)
	mkdir -p $(DESTDIR)$(LIBV4L_HELPERS_INSTALL_DIR)/libv4l
	install -m 755 $(LIBV4LCONVERT_DIR)/*-decomp \
		$(DESTDIR)$(LIBV4L_HELPERS_INSTALL_DIR)/libv4l

uninstall:
	for f in $(USER_INCLUDES); do rm -f $(DESTDIR)$(INC_INSTALL_DIR)/$${f}; done
	rm -f $(DESTDIR)$(LIB_INSTALL_DIR)/$(LIB)
	rm -f $(DESTDIR)$(LIB_INSTALL_DIR)/$(LIB).$(LIB_RELEASE)
	rm -f $(DESTDIR)$(LIBV4L_HELPERS_INSTALL_DIR)/libv4l/*-decomp

test: $(LIB).$(LIB_RELEASE) $(TESTOBJ)
	$(LD) $(TEST_APP_DIR)/list-devices.o -lm -lrt -L. -l$(LIB_NAME) -o list-devices
	$(LD) $(TEST_APP_DIR)/test-capture.o -lm -lrt -L. -l$(LIB_NAME) -o test-capture
	$(LD) $(TEST_APP_DIR)/list-caps.o -lm -lrt -L. -l$(LIB_NAME) -o list-caps
	$(LD) $(TEST_APP_DIR)/dump-capture.o -lm -lrt -L. -l$(LIB_NAME) -o dump-capture

$(LINUX_DIR)/version.h:
	@sh $(LINUX_DIR)/svn-version.sh

clean:
	make -C $(LIBV4LCONVERT_DIR) clean
	rm -f $(OBJ) $(BIN) $(LIB) $(LIB).$(LIB_RELEASE) $(TESTOBJ) \
  $(LIBV4LCONVERT_AR) ./*.o list-caps test-capture dump-capture list-devices $(LINUX_DIR)/version.h ./*~

.PHONY: all install uninstall test clean

