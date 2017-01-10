package au.edu.jcu.v4l4j.encoder;

import au.edu.jcu.v4l4j.ImagePalette;
import au.edu.jcu.v4l4j.V4L4JConstants;
import au.edu.jcu.v4l4j.exceptions.JNIException;

public class JPEGEncoder extends ImageTransformer {
	
	public static JPEGEncoder from(int width, int height, ImagePalette from) {
		return new JPEGEncoder(width, height, from);
	}
	
	protected JPEGEncoder(long ptr) {
		super(ptr);
	}
	
	protected JPEGEncoder(int width, int height, ImagePalette from) {
		super(ImageTransformer.lookupConverterByConversion(from, ImagePalette.JPEG), width, height);
	}
	
	/**
	 * Set encoded JPEG quality
	 * @param quality
	 */
	public native void setQuality(int quality);
	/**
	 * Get encoded JPEG quality
	 * @return
	 */
	public native int getQuality();
}
