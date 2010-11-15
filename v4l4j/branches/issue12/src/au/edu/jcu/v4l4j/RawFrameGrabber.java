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

import au.edu.jcu.v4l4j.exceptions.ImageFormatException;
import au.edu.jcu.v4l4j.exceptions.StateException;

/**
 * This class provides methods to capture raw frames from a {@link VideoDevice}.
 * Raw means that the image format will be left untouched and passed on straight
 * away to the caller. v4l4j also provides additional frame grabber classes 
 * which encodes frames in a specific format before handing them out.
 * <code>FrameGrabber</code> objects are not instantiated directly. Instead, the 
 * {@link VideoDevice#getRawFrameGrabber(int, int, int, int) getRawFrameGrabber()}
 * method must be called on the associated {@link VideoDevice}. Requested height
 * and width may be adjusted to the closest supported values. The adjusted width 
 * and height can be retrieved by calling {@link #getWidth()} and 
 * {@link #getHeight()}.<br>
 * A typical <code>FrameGrabber</code> use is as follows:<br><br>
 * <code>//create a new video device<br>
 * VideoDevice vd = new VideoDevice("/dev/video0");<br>
 * <br>//Create an instance of FrameGrabber
 * <br>FrameGrabber f = vd.getRawFrameGrabber(320, 240, 0, 0);
 * <br> //the framegrabber will use the first image format supported by the 
 * device, as returned by
 * <br> //<code>vd.getDeviceInfo().getFormatList().getList().get(0)</code>
 * <br>
 * <br> //Start the frame capture 
 * <br>f.startCapture();
 * <br>while (!stop) {
 * <br>&nbsp;&nbsp; VideoFrame frame= f.getFrame(); //Get a frame
 * <br>&nbsp;&nbsp; //do something useful with it
 * <br>&nbsp;&nbsp; //then recycle it with:
 * <br>&nbsp;&nbsp; frame.recycle();
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
 * with one of the <code>getFrameGrabber()</code> method again. Similarly,
 * when the capture is stopped with {@link #stopCapture()}, it can be started 
 * again with {@link #stopCapture()} without having to create a new 
 * <code>FrameGrabber</code>.
 * 
 * @see JPEGFrameGrabber {@link RGBFrameGrabber}
 * @author gilles
 *
 */
public class RawFrameGrabber extends AbstractGrabber {
	/**
	 * This constructor builds a raw FrameGrabber object used to raw frames 
	 * from a video source.
	 * @param di the DeviceInfo of the VideoDevice who created this frame grabber
	 * @param o a JNI pointer to the v4l4j_device structure
	 * @param w the requested frame width 
	 * @param h the requested frame height
	 * @param ch the input index, as returned by
	 * <code>InputInfo.getIndex()</code>
	 * @param std the video standard, as returned by 
	 * <code>InputInfo.getSupportedStandards()</code> (see V4L4JConstants)
	 * @param t the {@link Tuner} associated with this frame grabber or
	 * <code>null</code>.
	 * @param imf the image format frames should be captured in
	 * @throws ImageFormatException if the image format is null and a RAW frame 
	 * grabber is to be created  
	 */
	RawFrameGrabber(DeviceInfo di, long o, int w, int h, int ch, 
			int std, Tuner t, ImageFormat imf) throws ImageFormatException{
		super(di, o,w,h,ch,std,t,imf, RAW_GRABBER);
	}
	
	/**
	 * This method returns the native image format used by this 
	 * FrameGrabber. The returned format specifies the image format the capture
	 * uses
	 * @return the native image format used by this FrameGrabber.
	 * @throws StateException if this 
	 * <code>FrameGrabber</code> has been already released, and therefore must
	 * not be used anymore.
	 */
	public ImageFormat getImageFormat(){
		state.checkReleased();
		return dInfo.getFormatList().getNativeFormat(format);
	}

	@Override
	protected void createBuffers(int bufferSize) {
		int numberOfBuffers = nbV4LBuffers;
		
		while(numberOfBuffers-- > 0)
			videoFrames.add( new UncompressedVideoFrame(this, bufferSize, null,null)	);
	}
}

