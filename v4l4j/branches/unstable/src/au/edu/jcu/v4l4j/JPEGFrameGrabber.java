package au.edu.jcu.v4l4j;

import au.edu.jcu.v4l4j.exceptions.V4L4JException;

public class JPEGFrameGrabber extends FrameGrabber {
	
	/**
	 * This constructor builds a FrameGrabber object used to capture JPEG frames from a video source
	 * @param w the requested frame width 
	 * @param h the requested frame height
	 * @param ch the input index, as returned by <code>InputInfo.getIndex()</code>
	 * @param std the video standard, as returned by <code>InputInfo.getSupportedStandards()</code>
	 * (see V4L4JConstants)
	 * @param q the JPEG image quality (the higher, the better the quality)
	 */
	JPEGFrameGrabber(long o, int w, int h, int ch, int std, int q){
		super(o,w,h,ch,std,
				q<V4l4JConstants.MIN_JPEG_QUALITY? V4l4JConstants.MIN_JPEG_QUALITY: 
					(q>V4l4JConstants.MAX_JPEG_QUALITY ? V4l4JConstants.MAX_JPEG_QUALITY : q) );
	}
	
	/**
	 * This method sets the desired JPEG quality
	 * @param q the quality (between 0 and 100 inclusive)
	 * @throws V4L4JException if the JPEG quality control is disabled because of
	 * the chosen image format
	 */
	public void setJPGQuality(int q){
		if(q<V4l4JConstants.MIN_JPEG_QUALITY)
			q =V4l4JConstants.MIN_JPEG_QUALITY; 
		if(q>V4l4JConstants.MAX_JPEG_QUALITY)
			q = V4l4JConstants.MAX_JPEG_QUALITY;
		
		setQuality(object, q);
		quality = q;
	}
	
	/**
	 * This method returns the current JPEG quality 
	 * @return the JPEG quality
	 */
	public int getJPGQuality(){
		return quality;
	}
}
