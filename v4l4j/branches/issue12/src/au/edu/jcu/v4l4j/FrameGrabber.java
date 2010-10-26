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

import java.nio.ByteBuffer;

import au.edu.jcu.v4l4j.FrameInterval.DiscreteInterval;
import au.edu.jcu.v4l4j.exceptions.InvalidValue;
import au.edu.jcu.v4l4j.exceptions.NoTunerException;
import au.edu.jcu.v4l4j.exceptions.StateException;
import au.edu.jcu.v4l4j.exceptions.UnsupportedMethod;
import au.edu.jcu.v4l4j.exceptions.V4L4JException;

/**
 * Objects implementing this interface provide methods to capture frames from 
 * a {@link VideoDevice}. <code>FrameGrabber</code>s are not instantiated 
 * directly. Instead, one of the <code>getXXXFrameGrabber()</code> methods
 * must be called on a {@link VideoDevice} to obtain one. Requested height
 * and width may be adjusted to the closest supported values. The adjusted width 
 * and height can be retrieved by calling {@link #getWidth()} and 
 * {@link #getHeight()}.<br>
 * A typical <code>FrameGrabber</code> use is as follows:<br><br>
 * <code><br>
 * Bytebuffer b;<br>
 * //create a new video device<br>
 * VideoDevice vd = new VideoDevice("/dev/video0");<br>
 * <br>//Create an instance of FrameGrabber
 * <br>FrameGrabber f = vd.getRawFrameGrabber(320, 240, 0, 0, 80);
 * <br> //the framegrabber will use the first image format supported by the 
 * device, as returned by
 * <br> //<code>vd.getDeviceInfo().getFormatList().getNativeFormats().get(0)</code>
 * <br>
 * <br> //Start the frame capture 
 * <br>f.startCapture();
 * <br>while (!stop) {
 * <br>&nbsp;&nbsp; b= f.getFrame(); //Get a frame
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
 * with one of the <code>getXXXFrameGrabber()</code> methods again. Similarly,
 * when the capture is stopped with {@link #stopCapture()}, it can be started 
 * again with {@link #stopCapture()} without having to create a new 
 * <code>FrameGrabber</code>.
 * 
 * @see RawFrameGrabber
 * @see JPEGFrameGrabber
 * @see RGBFrameGrabber
 * @see BGRFrameGrabber
 * @see YUVFrameGrabber
 * @see YVUFrameGrabber
 * @author gilles
 *
 */
public interface FrameGrabber {

	/**
	 * This method returns the native image format used by this 
	 * FrameGrabber. The returned format specifies the image format the capture
	 * uses.
	 * @return the native image format used by this FrameGrabber.
	 * @throws StateException if this 
	 * <code>FrameGrabber</code> has been already released, and therefore must
	 * not be used anymore.
	 */
	public ImageFormat getImageFormat();
	
	/**
	 * This method sets the frame interval used for capture. The frame interval
	 * defined the lapse of time (in second) between two captured frames and is 
	 * the inverse of the frame rate. It may or may not be supported by the 
	 * underlying hardware/driver. If not supported, calling this method 
	 * throws a {@link UnsupportedMethod} exception. This method will throw an
	 * {@link InvalidValue} exception if the given frame interval value is not
	 * supported.
	 * As a guide, you can check the {@link ResolutionInfo} objects associated
	 * with the video device to find out whether frame intervals are at all 
	 * supported, and if they are, what values (or range of values) is accepted.
	 * {@link ResolutionInfo} objects can be obtained for each 
	 * {@link ImageFormat}. See the {@link ImageFormat} and 
	 * {@link ResolutionInfo} documentation for more information.
	 * @param num the numerator of the frame interval to be set
	 * @param denom the denominator of the frame interval to be set
	 * @throws UnsupportedMethod if setting the frame interval is not supported.
	 * @throws InvalidValue if the given frame interval value is invalid.
	 * @throws StateException if capture is ongoing (the frame interval must be
	 * set when not capturing), or if this 
	 * <code>FrameGrabber</code> has been already released, and therefore must
	 * not be used anymore.
	 */
	public void setFrameInterval(int num, int denom) throws InvalidValue;
	
	/**
	 * This method returns the current frame interval used for capture. 
	 * It may or may not be supported by the 
	 * underlying hardware/driver. If not supported, calling this method 
	 * throws a {@link UnsupportedMethod} exception.
	 * @throws UnsupportedMethod if setting the frame interval is not supported.
	 * @throws StateException if capture is ongoing (the frame interval must be
	 * queried when not capturing), or if this 
	 * <code>FrameGrabber</code> has been already released, and therefore must
	 * not be used anymore.
	 */
	public DiscreteInterval getFrameInterval();

	/**
	 * This method returns the {@link Tuner} associated with the input of this 
	 * <code>FrameGrabber</code>, or throws a {@link NoTunerException} if there
	 * is none.   
	 * @return the {@link Tuner} object associated with the chosen input.
	 * @throws NoTunerException if the selected input does not have a tuner
	 * @throws StateException if this 
	 * <code>FrameGrabber</code> has been already released, and therefore must
	 * not be used anymore.
	 */
	public Tuner getTuner() throws NoTunerException;

	/**
	 * This method starts the capture. After this call, frames can be retrieved
	 * with {@link #getFrame()}.
	 * @throws V4L4JException if the capture cant be started
	 * @throws StateException if this <code>FrameGrabber</code> has been already
	 * released, and therefore must not be used anymore
	 */
	public void startCapture() throws V4L4JException;

	/**
	 * This method retrieves one frame from the video source. The ByteBuffer 
	 * {@link ByteBuffer#limit() limit()} is set to the size of the captured
	 * frame. Note that the returned ByteBuffer is not backed by an array.
	 * This is a JNI limitation (not v4l4j).
	 * @return a ByteBuffer containing frame data.
	 * @throws V4L4JException if there is an error capturing from the source.
	 * @throws StateException if the capture has not been started or if this 
	 * <code>FrameGrabber</code> has been already released, and therefore must 
	 * not be used anymore.
	 */
	public ByteBuffer getFrame() throws V4L4JException;

	/**
	 * This method stops the capture.
	 * @throws StateException if the capture has not been started or if this 
	 * <code>FrameGrabber</code> has been already released, and therefore must
	 * not be used anymore.
	 */
	public void stopCapture();

	/**
	 * This method returns the actual height of captured frames.
	 * @return the height
	 * @throws StateException if this 
	 * <code>FrameGrabber</code> has been already released, and therefore must
	 * not be used anymore.
	 */
	public int getHeight();

	/**
	 * This method returns the actual width of captured frames.
	 * @return the width
	 * @throws StateException if this 
	 * <code>FrameGrabber</code> has been already released, and therefore must
	 * not be used anymore.
	 */
	public int getWidth();

	/**
	 * This method returns the video channel used to capture frames.
	 * @return the channel
	 * @throws StateException if this 
	 * <code>FrameGrabber</code> has been already released, and therefore must
	 * not be used anymore.
	 */
	public int getChannel();

	/**
	 * This method returns the actual video standard: 
	 * {@link V4L4JConstants#STANDARD_NTSC}, {@link V4L4JConstants#STANDARD_PAL}
	 * , {@link V4L4JConstants#STANDARD_SECAM} or 
	 * {@link V4L4JConstants#STANDARD_WEBCAM}
	 * @return the video standard
	 * @throws StateException if this 
	 * <code>FrameGrabber</code> has been already released, and therefore must
	 * not be used anymore.
	 */
	public int getStandard();

}