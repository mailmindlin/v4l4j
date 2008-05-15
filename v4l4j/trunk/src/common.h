#ifndef H_COMMON__
#define H_COMMON__

#include <stdio.h>
#include <jpeglib.h>

struct v4l4j_device;

struct jpeg {
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	struct jpeg_destination_mgr destmgr;
	//struct v4l4j_device, void *src, void *dst
	void (*jpeg_encode) (struct v4l4j_device *, void *, void *);
};

struct v4l4j_device {
	struct capture_device *c;	//the V4L2 struct
	unsigned char **bufs;		//the buffers holding the RGB data
	struct jpeg j;				//the jpeg compressor
	int jpeg_quality;			//the jpeg quality
	int len;					//the size of the last compressed frame
	int buf_id;					//the index of the buffer where the next frame goes
}; 


#endif /*H_COMMON_*/
