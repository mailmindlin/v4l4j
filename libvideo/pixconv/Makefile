
ifneq ($(DEBUG),)
DBG := -DDEBUG -ggdb -Wall -DLIBVIDEO_LOG_LEVEL=65535 \
  -DLIBVIDEO_LOG_SOURCE=65535
else
OPTIMISATION := -O3
endif

LIB_NAME := pixconv
LIB := lib$(LIB_NAME).a
TESTOBJ := test.o
OBJ := pixconv.o yuv_to_rgb.o

CC := gcc
CFLAGS := $(DBG) $(OPTIMISATION) -msse2 -fPIC -I.. -I.

LD := gcc
LDFLAGS := -lrt

all: $(LIB)

$(LIB): $(OBJ)
	$(AR) cqs $@ $^

test: $(LIB) $(TESTOBJ)
	$(LD) $(OBJ) test.o -lrt -o test

clean:
	rm -f $(OBJ) $(LIB) ./*.o test ./*~

.PHONY: all test clean

