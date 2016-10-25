package au.edu.jcu.v4l4j.encoder;

import au.edu.jcu.v4l4j.BaseVideoFrame;
import au.edu.jcu.v4l4j.ImagePalette;
import au.edu.jcu.v4l4j.V4L4JConstants;
import au.edu.jcu.v4l4j.VideoFrame;
import au.edu.jcu.v4l4j.exceptions.V4L4JException;

public class JPEGEncoder extends ImageFormatConverter {
	
	protected int quality;
	
	public static JPEGEncoder from(int width, int height, ImagePalette from) {
		return new JPEGEncoder(width, height, from);
	}
	
	public static JPEGEncoder to(int width, int height, ImagePalette to) {
		return new JPEGEncoder(width, height, ImagePalette.JPEG, to);
	}
	
	protected JPEGEncoder(int width, int height, ImagePalette from, ImagePalette to) {
		super(ImageFormatConverter.lookupConverterByConversion(from.getIndex(), to.getIndex()), width, height);
	}
	
	public JPEGEncoder(int width, int height, ImagePalette from) {
		this(width, height, from, ImagePalette.JPEG);
	}

	public void setQuality(int quality) {
		if (quality < V4L4JConstants.MIN_JPEG_QUALITY)
			quality = V4L4JConstants.MIN_JPEG_QUALITY;
		if (quality > V4L4JConstants.MAX_JPEG_QUALITY)
			quality = V4L4JConstants.MAX_JPEG_QUALITY;
		this.quality = quality;
	}
}
