CONVERT_NEON_ASM_OBJS = rgbyuv_neon.o
AFLAGS += -mcpu=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard -march=armv7-a -mapcs-reentrant -fPIC
AS = as

all:
	$(AS) $(AFLAGS) rgbyuv_neon.s -o rgbyuv_neon.o