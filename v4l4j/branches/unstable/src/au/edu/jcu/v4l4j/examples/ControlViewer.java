package au.edu.jcu.v4l4j.examples;

import au.edu.jcu.v4l4j.FrameGrabber;
import au.edu.jcu.v4l4j.ImageFormat;
import au.edu.jcu.v4l4j.VideoDevice;
import au.edu.jcu.v4l4j.exceptions.V4L4JException;

public class ControlViewer implements ImageProcessor{
	
	/**
	 * Builds a WebcamViewer object
	 * @param v the video device 
	 */
    public ControlViewer(VideoDevice v) {
    	new VideoViewer(v, this).initGUI(new Object[] {},640,480,"");    	      
    }

	@Override
	public FrameGrabber getGrabber(ImageFormat i) throws V4L4JException {
		// we shouldnt be called
		return null;
	}

	@Override
	public void processImage(byte[] b) {
		// we shouldnt be called
		
	}

}
