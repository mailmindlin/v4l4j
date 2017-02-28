package au.edu.jcu.v4l4j.api;

/**
 * An enum of image coding (compression) types.
 * @author mailmindlin
 */
public enum ImageCodingType {
	/**
	 * No image coding is used
	 */
	UNUSED,
	/**
	 * The image coding will be auto-detected.
	 * Note that this option may not work for all components
	 */
	AUTO_DETECT,
	/**
	 * JPEG encoding
	 */
	JPEG,
	JPEG2K,
	EXIF,
	TIFF,
	GIF,
	PNG,
	LZW,
	BMP;
}
