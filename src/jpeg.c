#include "common.h"
#include "debug.h"
#include "libv4l.h"

/* Dummy JPEG methods */
void init_destination( j_compress_ptr cinfo ){}
boolean empty_output_buffer( j_compress_ptr cinfo ){return TRUE;}
void term_destination( j_compress_ptr cinfo ){}

/* Encodes a YUV planar frame of width "d->c->width and height "d->c->height" at "src" straight 
 * into a JPEG frame at "dst" (must be allocated y caller). "d->len" is set to the
 * length of the compressed JPEG frame. "d->j" contains the JPEG compressor and 
 * must be initialised correctly by the caller
 */
void jpeg_encode_yuv420(struct v4l4j_device *d, void *src, void *dst) {
	//Code for this function is taken from Motion
	//Credit to them !!!
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);
	
	JSAMPROW y[16],cb[16],cr[16]; 
	JSAMPARRAY data[3]; 
	int i, line, rgb_size, width, height;
	width = d->c->width;
	height = d->c->height ;
	
	data[0] = y;
	data[1] = cb;
	data[2] = cr;
	
	//init JPEG dest mgr
	rgb_size = width * height * 3;
	d->j.destmgr.next_output_byte = dst;
	d->j.destmgr.free_in_buffer = rgb_size;
	jpeg_set_quality(&(d->j.cinfo),d->jpeg_quality,TRUE);
	
	dprint(LOG_JPEG, "[JPEG] Starting compression (%d bytes)\n", d->c->imagesize);	
	jpeg_start_compress( &(d->j.cinfo), TRUE );
	for (line=0; line<height; line+=16) {
		for (i=0; i<16; i++) {
			y[i] = (unsigned char *)src + width*(i+line);
			if (i%2 == 0) {
				cb[i/2] = (unsigned char *)src + width*height + width/2*((i+line)/2);
				cr[i/2] = (unsigned char *)src + width*height + width*height/4 + width/2*((i+line)/2);
			}
		}
		jpeg_write_raw_data(&(d->j.cinfo), data, 16);
	}
	jpeg_finish_compress(&(d->j.cinfo));
	d->len = rgb_size - d->j.cinfo.dest->free_in_buffer;
	dprint(LOG_JPEG, "[JPEG] Finished compression (%d bytes)\n", d->len);
}

void jpeg_encode_rgb24(struct v4l4j_device *d, void *src, void *dst){
	JSAMPROW row_ptr[1];
	int	width = d->c->width;
	int height = d->c->height ;
	int rgb_size = width * height * 3, stride = d->j.cinfo.image_width * 3, bytes=0;

	//init JPEG dest mgr
	dprint(LOG_CALLS, "[CALL] Entering %s\n",__PRETTY_FUNCTION__);

	d->j.destmgr.next_output_byte = dst;
	d->j.destmgr.free_in_buffer = rgb_size;
	jpeg_set_quality(&d->j.cinfo, d->jpeg_quality,TRUE);
	jpeg_start_compress( &d->j.cinfo, TRUE );
	dprint(LOG_JPEG, "[JPEG] Starting compression (%d bytes)\n", d->c->imagesize);
	while(d->j.cinfo.next_scanline < d->j.cinfo.image_height ) {
		bytes += stride;
		row_ptr[0] = src + d->j.cinfo.next_scanline * stride;
		jpeg_write_scanlines( &d->j.cinfo, row_ptr, 1 );
	}
	jpeg_finish_compress( &d->j.cinfo );

	d->len = rgb_size - d->j.cinfo.dest->free_in_buffer;
	dprint(LOG_JPEG, "[JPEG] Finished compression (%d bytes)\n", d->len);
}

int init_jpeg_compressor(struct v4l4j_device *d, int q){
	dprint(LOG_JPEG, "[JPEG] Initialising up the JPEG compressor\n");
	d->j.cinfo.err = jpeg_std_error(&d->j.jerr);
 	jpeg_create_compress(&d->j.cinfo);
 	d->j.destmgr.init_destination = init_destination;
 	d->j.destmgr.empty_output_buffer = empty_output_buffer;
 	d->j.destmgr.term_destination = term_destination;
 	d->j.cinfo.dest = &d->j.destmgr;

 	d->j.cinfo.image_width = d->c->width;
 	d->j.cinfo.image_height = d->c->height;
 	d->j.cinfo.input_components = 3;

	if(d->c->palette == YUV420) {
		dprint(LOG_JPEG, "[JPEG] Setting up the JPEG compressor for YUV420\n");	
		jpeg_set_defaults(&d->j.cinfo);
		jpeg_set_colorspace(&d->j.cinfo, JCS_YCbCr);
		d->j.cinfo.raw_data_in = TRUE; // supply downsampled data
		d->j.cinfo.comp_info[0].h_samp_factor = 2;
		d->j.cinfo.comp_info[0].v_samp_factor = 2;
		d->j.cinfo.comp_info[1].h_samp_factor = 1;
		d->j.cinfo.comp_info[1].v_samp_factor = 1;
		d->j.cinfo.comp_info[2].h_samp_factor = 1;
		d->j.cinfo.comp_info[2].v_samp_factor = 1;
		d->j.cinfo.dct_method = JDCT_FASTEST;
		d->j.jpeg_encode = jpeg_encode_yuv420;
	} else if (d->c->palette == RGB24) {
		dprint(LOG_JPEG, "[JPEG] Setting up the JPEG compressor for RGB24\n");
		d->j.cinfo.in_color_space = JCS_RGB;
		jpeg_set_defaults(&d->j.cinfo) ;
		d->j.jpeg_encode = jpeg_encode_rgb24;
	} else {
		dprint(LOG_JPEG, "[JPEG] Unsupported palette\n");
		return -1;
	}
	
	d->jpeg_quality=q;
	return 0;
}

void destroy_jpeg_compressor(struct v4l4j_device *d){
	dprint(LOG_JPEG, "[JPEG] Destroying JPEG compressor\n");
	jpeg_destroy_compress(&d->j.cinfo);	
}




