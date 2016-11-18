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
#Directory where the libv4l helpers will be installed
LIBV4L_HELPERS_INSTALL_DIR := $(PREFIX)/lib/v4l4j



#
# edit anything below at your own risk
#

USER_INCLUDES := libvideo.h libvideo-err.h libvideo-palettes.h

VERSION_MAJOR := 0
VERSION_MINOR := 9
LIB_RELEASE := 0

LIBV4LCONVERT_DIR := libv4lconvert
LIBV4LCONVERT_AR := $(LIBV4LCONVERT_DIR)/libv4lconvert.a
LIBV4LCONVERT_NAME := v4lconvert
LIBV4LCONVERT_HELPERS := ov511-decomp ov518-decomp

ARCH ?= $(shell arch)
HARDWARE_FLAGS = 
ifneq ($(findstring x86_64, $(ARCH)),)
	LIBPIXFC ?= pixfc-sse/libpixfc-sse-x86_64.a
else ifeq ($(shell arch | grep -Eic "^i?.86"), 1)
	LIBPIXFC ?= pixfc-sse/libpixfc-sse-x86.a
else ifneq ($(findstring armv7, $(ARCH)),)
	#Detect if hard float
	ifneq ($(wildcard /lib/arm-linux-gnueabihf/.),)
		LIBPIXFC ?= pixfc-sse/libpixfc-sse-armv7-hf.a
		HARDWARE_FLAGS += -mfloat-abi=hard
	else
		LIBPIXFC ?= pixfc-sse/libpixfc-sse-armv7.a
		HARDWARE_FLAGS += -mfloat-abi=softfps
	endif
else ifneq ($(findstring armv6, $(ARCH)),)
	#Detect if hard float
	ifneq ($(wildcard /lib/arm-linux-gnueabihf/.),)
		LIBPIXFC ?= pixfc-sse/libpixfc-sse-armv6l-hardfp.a
		HARDWARE_FLAGS += -mfloat-abi=hard
	else
		LIBPIXFC ?= pixfc-sse/libpixfc-sse-armv6l-softfp.a
		HARDWARE_FLAGS += -mfloat-abi=softfp
	endif
else ifneq ($(findstring arm, $(ARCH)),)
	# Fallback for ARM
	LIBPIXFC ?= pixfc-sse/libpixfc-sse-armv6l-softfp.a
else ifeq ($(origin LIBPIXFC), undefined)
	$(error Unable to determine architecture to select the right libpixfc)
endif

ifeq ($(shell arch),$(ARCH))
	ifeq ($(findstring march, $(HARDWARE_FLAGS)),)
		HARDWARE_FLAGS += -march=native
		ifeq ($(findstring mcpu, $(HARDWARE_FLAGS)),)
			HARDWARE_FLAGS += -mcpu=native
			ifeq ($(findstring mtune, $(HARDWARE_FLAGS)),)
				HARDWARE_FLAGS += -mtune=native
			endif
		endif
	endif
	ifneq ($(findstring arm, $(ARCH)),)
		ifeq ($(findstring -mfpu, $(HARDWARE_FLAGS)),)
			CPUINFO = $(shell cat /proc/cpuinfo | grep Features)
			ifneq ($(findstring neon, $(CPUINFO)),)
				ifneq ($(findstring vfpv4, $(CPUINFO)),)
					HARDWARE_FLAGS += -mfpu=neon-vfpv4
				else ifneq ($(findstring armv8, $(ARCH)),)
					# Maybe should be neon-fp-armv8
					HARDWARE_FLAGS += -mfpu=crypto-neon-fp-armv8
				else
					HARDWARE_FLAGS += -mfpu=neon
				endif
			else ifneq ($(findstring armv8, $(ARCH)),)
				HARDWARE_FLAGS += -mfpu=fp-armv8
			else ifneq ($(findstring vfpv4, $(CPUINFO)),)
				HARDWARE_FLAGS += -mfpu=vfpv4
			else ifneq ($(findstring vfpv3d16, $(CPUINFO)),)
				HARDWARE_FLAGS += -mfpu=vfpv3-d16
			else ifneq ($(findstring vfpv3, $(CPUINFO)))
				HARDWARE_FLAGS += -mfpu=vfpv3
			else ifneq ($(findstring vfp, $(CPUINFO)),)
				HARDWARE_FLAGS += -mfpu=vfp
			endif
		endif
		ifeq ($(findstring -mfloat-abi, $(HARDWARE_FLAGS)),)
			ifneq ($(wildcard /lib/arm-linux-gnueabihf/.),)
				HARDWARE_FLAGS += -mfloat-abi=hard
			else
				HARDWARE_FLAGS += -mfloat-abi=softfp
			endif
		endif
	endif
endif

LIB_NAME := video
LIB := lib$(LIB_NAME).so
TESTOBJ := test-capture.o dump-capture.o list-caps.o

OBJ := v4l2-input.o v4l1-input.o libvideo.o pwc-probe.o gspca-probe.o \
	qc-probe.o fps-param-probe.o v4l1-query.o v4l2-query.o v4l1-tuner.o \
	v4l2-tuner.o palettes.o

WARNINGS +=  -Wall -Wpointer-arith -Wextra -Wmissing-prototypes -Wstrict-prototypes -Wconversion -Wunused-function
CFLAGS += $(HARDWARE_FLAGS) $(WARNINGS) -DVER_MAJ=$(VERSION_MAJOR) -DVER_MIN=$(VERSION_MINOR) -I. -std=gnu11
CFLAGS += -fPIC -fdiagnostics-color=auto -ftree-vectorize -flto
#CFLAGS += -funsafe-math-optimizations
AFLAGS += $(HARDWARE_FLAGS) -mapcs-reentrant -k --warn --statistics
LDFLAGS += $(HARDWARE_FLAGS) -lm -lrt -shared -ljpeg -lc -flto

ifneq ($(DEBUG),)
	CFLAGS += -DDEBUG -ggdb -DLIBVIDEO_LOG_LEVEL=$(LIBVIDEO_LOG_LEVEL) -DLIBVIDEO_LOG_SOURCE=$(LIBVIDEO_LOG_SOURCE)
else
	CFLAGS += -Os -g
endif


all: $(LIB).$(LIB_RELEASE)

$(LIB).$(LIB_RELEASE): version.h $(OBJ) $(LIBV4LCONVERT_AR)
	$(CC) $(OBJ) -Wl,-soname,$(LIB).$(LIB_RELEASE) \
		-o $(LIB).$(LIB_RELEASE) $(LDFLAGS) \
		$(LIBPIXFC) $(LIBV4LCONVERT_AR)

$(LIB): $(LIB).$(LIB_RELEASE)
	ln -f -s $(LIB).$(LIB_RELEASE) $(LIB)

$(LIBV4LCONVERT_AR): FORCE

FORCE:
	export CFLAGS
	export HARDWARE_FLAGS
	@LINKTYPE=static LIBDIR=$(LIBV4L_HELPERS_INSTALL_DIR) $(MAKE) --no-print-directory -C $(LIBV4LCONVERT_DIR)

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
	$(CC) test-capture.o -lm -lrt -L. -l$(LIB_NAME) -o test-capture
	$(CC) list-caps.o -lm -lrt -L. -l$(LIB_NAME) -o list-caps
	$(CC) dump-capture.o -lm -lrt -L. -l$(LIB_NAME) -o dump-capture

clean:
	$(MAKE) -C $(LIBV4LCONVERT_DIR) clean
	rm -f $(OBJ) $(BIN) $(LIB) $(LIB).$(LIB_RELEASE) $(TESTOBJ) $(LIBV4LCONVERT_AR) ./*.o list-caps test-capture dump-capture ./*~ version.h

version.h:
	@echo "#define VER_REV \"$(git rev-parse --abbrev-ref HEAD)$(git rev-parse --short=7 HEAD)\"" > version.h

.PHONY: all install uninstall test clean
