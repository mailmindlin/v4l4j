#ifndef __LIBSTREAMING_LIBSTREAMING_H
#define __LIBSTREAMING_LIBSTREAMING_H

#include "types.h"

typedef struct {
	u32 urlLength;
	u16 port;
	char* url;
} streaming_data_t;

void streaming_init(streaming_data_t* data);
void streaming_startStreaming() {
	
}

#endif