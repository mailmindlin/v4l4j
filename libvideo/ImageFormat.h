#ifndef __LIBVIDEO_IMAGEFORMAT_H
#define __LIBVIDEO_IMAGEFORMAT_H

typedef struct ImageFormat {
	/**
	 * libvideo palette id
	 */
	unsigned int paletteId;
	enum ImageFormatType {
		IMAGE_FORMAT_TYPE_PIXEL,
		IMAGE_FORMAT_TYPE_PLANAR,
		IMAGE_FORMAT_TYPE_COMPRESSED,
		IMAGE_FORMAT_TYPE_STREAM,
		IMAGE_FORMAT_TYPE_OTHER
	} type;
	unsigned int width;
	unsigned int height;
	union {
		struct {
			/**
			 * For padding; 0 if unused
			 */
			unsigned int bytesPerLine;
		} pixelData;
		struct {
			
		} planarData;
		u8 userDefined[32];
	};
} ImageFormat;

#endif