package au.edu.jcu.v4l4j.examples;

import au.edu.jcu.v4l4j.FrameGrabber;
import au.edu.jcu.v4l4j.ImageFormat;
import au.edu.jcu.v4l4j.exceptions.V4L4JException;

/**
 * Objects implementing this interface can produce a {@link FrameGrabber} (or
 * one of its subclasses), and can handle images as they are captured.
 * @author gilles
 *
 */
public interface ImageProcessor {
	/**
	 * This method is called on this processor whenever a {@link FrameGrabber}
	 * (or one of its subclasses) must be created for a given {@link ImageFormat}
	 * @param i the Image format the frmae grabber must be created for
	 * @return the frame grabber
	 * @throws V4L4JException if there is an error creating the frame grabber
	 */
	public FrameGrabber getGrabber(ImageFormat i)  throws V4L4JException;
	
	/**
	 * this method is called on this image processor whenever a new image is
	 * available for processing
	 * @param b the image data as a byte array
	 */
	public void processImage(byte[] b);
}
