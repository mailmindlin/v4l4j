/*
* Copyright (C) 2007-2008 Gilles Gigan (gilles.gigan@gmail.com)
* eResearch Centre, James Cook University (eresearch.jcu.edu.au)
*
* This program was developed as part of the ARCHER project
* (Australian Research Enabling Environment) funded by a   
* Systemic Infrastructure Initiative (SII) grant and supported by the Australian
* Department of Innovation, Industry, Science and Research
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public  License as published by the
* Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
* or FITNESS FOR A PARTICULAR PURPOSE.  
* See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/
package au.edu.jcu.v4l4j;


/**
 * Objects of this class are used to retrieve JPEG-encoded frames from a 
 * {@link VideoDevice}. v4l4j also provide {@link FrameGrabber} objects to
 * retrieve images in a native format. 
 * <code>JPEGFrameGrabber</code> objects are not 
 * instantiated directly. Instead, the 
 * {@link VideoDevice#getJPEGFrameGrabber(int, int, int, int, int) getJPEGFrameGrabber()}
 * method must be called on the associated {@link VideoDevice}. Requested height
 * and width may be adjusted to the closest supported values. The adjusted
 * width and height can be retrieved by calling {@link #getWidth()} and 
 * {@link #getHeight()}.<br>
 * A typical <code>JPEGFrameGrabber</code> use is as follows:<br><br>
 * <code>//create a new video device<br>
 * VideoDevice vd = new VideoDevice("/dev/video0");<br>
 * <br>//Create an instance of FrameGrabber
 * <br>FrameGrabber f = vd.getJPEGFrameGrabber(320, 240, 0, 0, 80);
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
 * Once the frame grabber is released with 
 * {@link VideoDevice#releaseFrameGrabber()}, it can be re-initialised again 
 * with {@link VideoDevice#getJPEGFrameGrabber(int, int, int, int, int) getJPEGFrameGrabber()}.
 * Similarly, when the capture is stopped with {@link #stopCapture()}, it can be
 * started again with {@link #stopCapture()} without having to create a new 
 * <code>FrameGrabber</code>.
 * @see FrameGrabber
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
	JPEGFrameGrabber(long o, int w, int h, int ch, int std, int q, Tuner t){
		super(o,w,h,ch,std,
				q<V4L4JConstants.MIN_JPEG_QUALITY? V4L4JConstants.MIN_JPEG_QUALITY: 
					(q>V4L4JConstants.MAX_JPEG_QUALITY ? V4L4JConstants.MAX_JPEG_QUALITY : q), t );
	}
	
	/**
	 * This method sets the desired JPEG quality.
	 * @param q the quality (between 
	 * {@link V4L4JConstants#MIN_JPEG_QUALITY} and 
	 * {@link V4L4JConstants#MAX_JPEG_QUALITY} inclusive)
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
	 * This method returns the current JPEG quality.
	 * @return the JPEG quality
	 */
	public int getJPGQuality(){
		return quality;
	}
}
