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

import au.edu.jcu.v4l4j.FrameInterval.DiscreteInterval;
import au.edu.jcu.v4l4j.examples.PushModeCaptureApp;
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
 * Frame grabbers operate in two modes: push or pull. In push mode, v4l4j is given
 * an object implementing the {@link PushSourceCallback} interface which will
 * be given new captured frames as soon as they arrive. In pull mode,
 * frames are retrieved by repeated calls to {@link #getVideoFrame()}. By
 * default, a frame grabber operates in pull mode. Call 
 * {@link #setPushSourceMode(PushSourceCallback)} to select the desired mode <b>before</b>
 * starting the capture (with {@link #startCapture()}).<br>
 * A typical <code>FrameGrabber</code> use (in pull mode) is as follows:
 * <br>
 * In the main thread, create the video device and frame grabber:
 * <code><br><br>
 * //create a new video device<br>
 * VideoDevice vd = new VideoDevice("/dev/video0");<br>
 * <br>//Create an instance of FrameGrabber
 * <br>FrameGrabber f = vd.getRawFrameGrabber(320, 240, 0, 0, 80);
 * <br> //the framegrabber will use the first image format supported by the 
 * device, as returned by
 * <br> //vd.getDeviceInfo().getFormatList().getNativeFormats().get(0)
 * <br>
 * </code><br>
 * Create and run a capture thread which runs the following loop:<br>
 * <code> 
 * <br>VideoFrame frame;
 * <br>while (!stop) {
 * <br>&nbsp;&nbsp; frame = f.getVideoFrame(); // block until a frame is ready
 * <br>&nbsp;&nbsp; //do something useful with frame, then recycle it
 * <br>&nbsp;&nbsp; //when done with it, so v4l4j can re-use it later on
 * <br>&nbsp;&nbsp; frame.recycle();
 * <br>}<br>
 * </code>
 * Back in the main thread, start the capture:<br>
 * <code> 
 * <br>f.startCapture();
 * <br>//At this point, the capture thread will be unblocked each time a frame 
 * <br>//is ready.
 * <br>//
 * <br>//When done, stop the capture thread
 * <br>stop = true;
 * <br>//Stop the capture
 * <br>f.stopCapture();<br>
 * <br>//Free capture resources and release the FrameGrabber
 * <br>vd.releaseFrameGrabber();<br>
 * <br>//release VideoDevice
 * <br>vd.release();
 * </code><br><br>
 * In push mode, instead of calling {@link #getVideoFrame()}, frames are delivered
 * as soon they are captured by v4l4j to the provided {@link PushSourceCallback}
 * object via its {@link PushSourceCallback#nextFrame(VideoFrame) nextFrame()} method.
 * For an example of push mode capture, see {@link PushModeCaptureApp}.
 * <br>In both pull and push modes, video frames must be recycled when they are no
 * longer used.<br></br>
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
	 * FrameGrabber. The returned format specifies the image format in which
	 * frames are obtained from the device.
	 * @return the native image format used by this FrameGrabber.
	 * @throws StateException if this 
	 * <code>FrameGrabber</code> has been already released, and therefore must
	 * not be used anymore.
	 */
	public ImageFormat getImageFormat();
	
	/**
	 * This method returns the number of buffers v4l4j has negotiated with
	 * the driver. The driver store captured frames in these buffers and return
	 * them to v4l4j. This number is an indication as to how many video
	 * frames can be obtained from the driver through {@link #getVideoFrame()}
	 * before the capture stops until a buffer is returned to the driver 
	 * through {@link VideoFrame#recycle()}. Practically speaking, this number
	 * specifies how many times you can call {@link #getVideoFrame()} before 
	 * the method blocks, waiting for one of the previous frame to be recycled.
	 * @return the number of frame buffers used to retrieve frames from 
	 * the driver
	 */
	public int getNumberOfVideoFrames();
	
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
	 * <code>setPushSourceMode</code> tells this frame grabber to work either
	 * in push or pull mode. In push mode, the given {@link PushSourceCallback}
	 * object will be notified by this grabber as soon as a new frame becomes
	 * available. In pull mode, captured frames are obtained by calling 
	 * {@link #getVideoFrame()}. When this method is called with a valid
	 * <code>callback</code> argument (that is when, <code>callback != null</code>)
	 * the push mode is activated. When <code>callback != null</code>, 
	 * the pull mode is activated.<br>
	 * This method cannot be called while the capture is active, ie. in between
	 * a call to {@link #startCapture()} and {@link #stopCapture()}.
	 * @param callback if not null, tell the frame grabber to work in push mode.
	 * The given object implementing the {@link PushSourceCallback}
	 * interface gets notified when a new frame is available, or when 
	 * an exception is triggered. If null, tell the frame grabber to work in
	 * pull mode. Frames must be obtained by calling {@link #getVideoFrame()}.
	 * @throws StateException if this method is invoked while capture is active,
	 * ie. after a call to {@link #startCapture()} and prior a call 
	 * to {@link #stopCapture()}. 
	 * @return whether in push mode (true) or pull mode (false) 
	 */
	public boolean setPushSourceMode(PushSourceCallback callback);

	/**
	 * This method starts the capture. After this call, frames can be retrieved
	 * with {@link #getVideoFrame()}.
	 * @throws V4L4JException if the capture cant be started
	 * @throws StateException if this <code>FrameGrabber</code> has been already
	 * released, and therefore must not be used anymore
	 */
	public void startCapture() throws V4L4JException;

	/**
	 * This method retrieves one frame from the video source. At the start 
	 * of the capture, v4l4j creates a certain number of {@link VideoFrame}s
	 * and places them in an "available" queue. You can retrieve the exact 
	 * number of buffers by calling {@link #getNumberOfVideoFrames()}.
	 * Each time this method is called, it retrieves a VideoFrame from the "available" queue,
	 * captures an image from the device, places it in the VideoFrame object and
	 * return it. When you have finished processing the VideoFrame, you must 
	 * recycle it by calling {@link VideoFrame#recycle()}, so it returns to the
	 * "available" queue and can be reused. If there are no 
	 * VideoFrames in the available queue when this method is called, it will 
	 * block until one gets recycled.
	 * @return an {@link VideoFrame} containing the captured frame data.
	 * @throws V4L4JException if there is an error capturing from the source.
	 * @throws StateException if either the capture has not been started, if this 
	 * <code>FrameGrabber</code> has been already released, and therefore must 
	 * not be used anymore or if we were interrupted while waiting for a frame
	 * to be recycled.
	 */
	public VideoFrame getVideoFrame() throws V4L4JException;

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