#include <jpeglib.h>
#include "../types.h"

#if JPEG_LIB_VERSION < 80 && !defined(MEM_SRCDEST_SUPPORTED)

void jpeg_mem_src (j_decompress_ptr cinfo, const u8 * buffer, unsigned long bufsize);

void jpeg_mem_dest (j_compress_ptr cinfo, u8 ** outbuffer, unsigned long * outsize);

#endif
