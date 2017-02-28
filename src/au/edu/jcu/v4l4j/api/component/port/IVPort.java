package au.edu.jcu.v4l4j.api.component.port;

import au.edu.jcu.v4l4j.api.ImageCodingType;
import au.edu.jcu.v4l4j.api.component.ComponentPort;

/**
 * Superinterface for both VideoPort and ImagePort, because they share a lot
 * of methods.
 * @author mailmindlin
 */
public interface IVPort extends ComponentPort {
	
	/**
	 * Get the width of the frame that 
	 * May be 0 if unknown/don't care/variable
	 */
	int getFrameWidth();
	
	/**
	 * Get the height of a frame that is produced/consumed by this port.
	 * May be 0 if unknown/don't care/variable
	 */
	int getFrameHeight();
	
	/**
	 * Number of bytes per span of the image.
	 * This can return a negative number, which indicates that the image
	 * is bottom-up.
	 */
	int getStride();
	
	/**
	 * Height used when encoding slices
	 */
	int getSliceHeight();
	
	/**
	 * Uncompressed 
	 */
	ImagePalette getColorFormat();
}
