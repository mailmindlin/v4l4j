package au.edu.jcu.v4l4j.api.component.port;

import au.edu.jcu.v4l4j.api.ImagePalette;

public interface IVPortDefinition extends PortDefinition {
	/**
	 * Get the width of the frame that 
	 * May be 0 if unknown/don't care/variable
	 * @return Frame width (pixels).
	 */
	int getFrameWidth();
	
	/**
	 * Get the height of a frame that is produced/consumed by this port.
	 * May be 0 if unknown/don't care/variable
	 * @return Frame width (pixels).
	 */
	int getFrameHeight();
	
	/**
	 * Number of bytes per span of the image.
	* <br/>
	 * Note: This method may return a negative number, which indicates that the
	 * image is bottom-up.
	 * 
	 * @return Bytes per span of image.
	 */
	int getStride();
	
	/**
	 * Height used when encoding slices
	 */
	int getSliceHeight();
	
	/**
	 * Uncompressed color format
	 * @return Color format
	 */
	ImagePalette getColorFormat();
}
