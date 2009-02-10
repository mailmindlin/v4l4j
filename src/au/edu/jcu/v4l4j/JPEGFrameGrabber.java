package au.edu.jcu.v4l4j;

import au.edu.jcu.v4l4j.exceptions.V4L4JException;

/**
 * Objects of this class are used to retrieve JPEG-encoded frames from a <code>VideoDevice</code>. <code>JPEGFrameGrabber</code> objects 
 * are not instantiated directly. Instead, the <code>getJPEGFrameGrabber()</code> method
 * must be called on the associated <code>VideoDevice</code>. Requested height and width may be adjusted
 * to the closest supported values. The adjusted width and height can be retreived by calling <code>getWidth()</code>
 * and <code>getHeight()</code>.<br>
 * A typical <code>JPEGFrameGrabber</code> use is as follows:<br><br>
 * <code>//create a new video device<br>
 * VideoDevice vd = new VideoDevice("/dev/video0");<br>
 * <br>//Create an instance of FrameGrabber
 * <br>FrameGrabber f = vd.getRawFrameGrabber(320, 240, 0, 0, 80);
 * <br> //the framegrabber will use the first image format supported by the device, as returned by
 * <br> //<code>vd.getDeviceInfo().getFormats().get(0)</code>
 * <br>
 * <br> //Start the frame capture 
 * <br>f.startCapture();
 * <br>while (!stop) {
 * <br>&nbsp;&nbsp; ByteBuffer b= f.getFrame(); //Get a frame
 * <br>&nbsp;&nbsp; //frame size is b.limit()
 * <br>&nbsp;&nbsp; //do something useful with b
 * <br>}<br>
 * <br>//Stop the capture
 * <br>f.stopCapture();<br>
 * <br>//Free capture resources and release the FrameGrabber
 * <br>vd.releaseFrameGrabber();<br>
 * <br>//release VideoDevice
 * <br>vd.release();
 * </code><br><br>
 * 
 * Once the frame grabber is released with <code>vd.releaseFrameGrabber()</code>, it can be
 * re-initialised again with <code>vd.getXXXFrameGrabber()</code>. Similarly, when the capture is stopped
 * with <code>FrameGrabber.stopCapture()</code>, it can be started again with <code>FrameGrabber.stopCapture()</code>
 * without having to create a new <code>FrameGrabber</code>.
 * @author gilles
 *
 */
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
				q<V4L4JConstants.MIN_JPEG_QUALITY? V4L4JConstants.MIN_JPEG_QUALITY: 
					(q>V4L4JConstants.MAX_JPEG_QUALITY ? V4L4JConstants.MAX_JPEG_QUALITY : q) );
	}
	
	/**
	 * This method sets the desired JPEG quality
	 * @param q the quality (between 0 and 100 inclusive)
	 * @throws V4L4JException if the JPEG quality control is disabled because of
	 * the chosen image format
	 */
	public void setJPGQuality(int q){
		if(q<V4L4JConstants.MIN_JPEG_QUALITY)
			q =V4L4JConstants.MIN_JPEG_QUALITY; 
		if(q>V4L4JConstants.MAX_JPEG_QUALITY)
			q = V4L4JConstants.MAX_JPEG_QUALITY;
		
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
