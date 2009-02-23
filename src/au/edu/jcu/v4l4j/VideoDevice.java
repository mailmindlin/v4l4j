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

import java.io.File;
import java.util.List;
import java.util.Vector;

import au.edu.jcu.v4l4j.exceptions.CaptureChannelException;
import au.edu.jcu.v4l4j.exceptions.ImageDimensionsException;
import au.edu.jcu.v4l4j.exceptions.ImageFormatException;
import au.edu.jcu.v4l4j.exceptions.InitialisationException;
import au.edu.jcu.v4l4j.exceptions.JNIException;
import au.edu.jcu.v4l4j.exceptions.NoTunerException;
import au.edu.jcu.v4l4j.exceptions.ReleaseException;
import au.edu.jcu.v4l4j.exceptions.StateException;
import au.edu.jcu.v4l4j.exceptions.V4L4JException;
import au.edu.jcu.v4l4j.exceptions.VideoStandardException;

/**
 * An instance of a <code>VideoDevice</code> object represents an existing V4L video device.
 * It is the starting point to use functionalities provided by v4l4j, which are divided in 3 categories:
 * <ul>
 * <li>Information gathering about the video device,</li>
 * <li>Capturing frames from the video device,</li>
 * <li>and Adjusting values of available video controls.</li>
 * </ul>
 * Each of these categories is detailed in the following sections.
 * To use features provided in any of these category, a <code>VideoDevice</code> object must first be instantiated.
 * This is done simply by calling the constructor and giving it the full path to the associated device file:
 * <br><code>
 * VideoDevice vd = new VideoDevice("/dev/video0");
 * </code><br>
 * <b>It is important that, once the <code>VideoDevice</code> is no longer used, its resources are freed. This is
 * done by calling the <code>release()</code> method:</b>
 * <br><code>vd.release();</code>
 * <br>Once the <code>VideoDevice</code> is released, neither itself nor any object references obtained through
 * it must be used.
 * <h2>Information gathering</h2>
 * A <code>VideoDevice</code> object offers a single method (<code>getDeviceInfo()</code>) to return information
 * about itself, encapsulated in a <code>DeviceInfo</code> object. DeviceInfo objects contain
 * get-type methods only, and return details such as:
 * <ul>
 * <li>List of all video inputs, their types & their supported standards</li>
 * <li>Details about available tuners (type, frequency range & unit, ...) </li>
 * <li>List of image formats supported by the device</li>
 * </ul>
 * See the {@link DeviceInfo} class for an example on how to use it.
 * <h2>Capturing frames</h2>
 * In order to capture frames from a video device, an instance of a <code>FrameGrabber</code> object must be
 * obtained. The <code>VideoDevice</code> object provides 2 methods for that, depending on the image format required:
 * <ul>
 * <li>{@link #getRawFrameGrabber(int, int, int, int) getRawFrameGrabber()} returns a <code>FrameGrabber</code> object capable of capturing frames
 * in one of the supported image format as reported by the {@link DeviceInfo#getFormatList() getFormat()} method of 
 * {@link DeviceInfo} objects. Captured frames are handed out straight away to the caller, without any other form of processing.</li>
 * <li>{@link #getJPEGFrameGrabber(int, int, int, int, int) getJPEGFrameGrabber} creates a <code>JPEGFrameGrabber</code> object capable of capturing frames
 * and JPEG-encoding them before handing them out. This frame grabber cannot be used with all video sources. It requires
 * images from the video source to be in some pre-defined formats (namely JPEG, MJPEG, YUV420, YUYV, YVYU and RGB24) in order 
 * to be encoded in JPEG format. If the video source is not capable of handing out images in either of these
 * formats, then no <code>JPEGFrameGrabber</code> can be created.</li>
 * </ul>
 * To check whether JPEG-encoding is supported by a <code>VideoSource</code> object, call its {@link #canJPEGEncode()} method.
 * <b>Similarly to <code>VideoDevice</code> objects, once the frame grabber is no longer used, its resources must be released.</b>
 * This is achieved by calling the {@link #releaseFrameGrabber()} method on the <code>VideoDevice</code> to which 
 * the frame grabber belongs. See the {@link FrameGrabber} class for more information on how to capture frames.
 * 
 * <h2>Video controls</h2>
 * A list of available video controls can be obtained by calling the {@link #getControlList()} method. Each control
 * contains detailed information about its type and acceptable values. Controls have a pair of get & set methods
 * to adjust their values. Once again, when no longer used, resources taken up by video controls must be released
 * by calling {@link #releaseControlList()}. See the {@link ControlList} and {@link Control} classes for more information
 * on how to use them.
 * 
 * 
 * 
 * @author gilles
 *
 */
public class VideoDevice {
	
	static {
		try {
			System.loadLibrary("v4l4j");
		} catch (UnsatisfiedLinkError e) {
			System.err.println("Cant load v4l4j JNI library");
			throw e;
		}
	}
	
	/**
	 * This JNI method initialises the libv4's struct video_device
	 * @param device the name of the device file
	 * @throws V4L4JException if there is an error
	 */
	private native long doInit(String device) throws V4L4JException;
	
	/**
	 * This JNI method releases resources used by libv4l's struct video_device,
	 * as allocated by <code>doInit()</code>
	 * @param o A C pointer to a struct vl4j_device
	 * @throws ReleaseException if the device is still in use.
	 */
	private native void doRelease(long o);
	
	/**
	 * This JNI method initialises the control interface and 
	 * returns an array of <code>Control</code>s.
	 * @param o A C pointer to a struct v4l4j_device
	 * @return an array of <code>Control</code>s.
	 * @throws JNIException if there is an error in the JNI code
	 */
	private native Control[] doGetControlList(long o);

	/**
	 * This JNI method releases the control interface
	 * @param o A C pointer to a struct v4l4j_device
	 * @throws ReleaseException if this device is sill in use, and has not been released.
	 */
	private native void doReleaseControlList(long o);
	
	/**
	 * This JNI method gets the tuner interface 
	 * @param o A C pointer to a struct v4l4j_device
	 * @throws JNIException if there is an error in the JNI code
	 */
	private native void doGetTunerActions(long o);

	/**
	 * This JNI method releases the tuner interface
	 * @param o A C pointer to a struct v4l4j_device
	 * @throws ReleaseException if this device is sill in use, and has not been released.
	 */
	private native void doReleaseTunerActions(long o);
	
	/**
	 * The FrameGrabber interface associated with this video device
	 */
	private FrameGrabber fg;
	
	/**
	 * The DeviceInfo object associated with this video device
	 */
	private DeviceInfo deviceInfo;
	
	/**
	 * The control list associated with this video device
	 */
	private ControlList controls;
	
	/**
	 * The tuner list associated with this video device
	 */
	private TunerList tuners;
	
	/**
	 * The name of the device file for this video device
	 */
	private String deviceFile;
	
	/**
	 * The state of our VideoDevice (used for synchronisation)
	 */
	private State state;
	
	/**
	 * Whether or not frames captured from this video device can be JPEG-encoded 
	 */
	private boolean supportJPEG;
	
	/**
	 * JNI returns a long (which is really a pointer) when a device is allocated for use
	 * This field is read-only (!!!) 
	 */
	private long v4l4jObject;
	
	/**
	 * This constructor builds a <code>VideoDevice</code> using the full path to its device file.
	 * When finished, resources must be released by calling the {@link #release()} method. 
	 * @param dev the path to the device file
	 * @throws V4L4JException if the device file is not accessible
	 */
	public VideoDevice(String dev) throws V4L4JException{
		if(!(new File(dev).canRead()))
			throw new V4L4JException("The device file is not readable");

		state = new State();		
		deviceFile = dev;
		
		init();
	}
	
	/**
	 * This method initialises this VideoDevice. This method must be called before any other methods.
	 * @throws V4L4JException if the device can not be initialised
	 */
	private void init() throws V4L4JException{		
		v4l4jObject = doInit(deviceFile);
		//initialise deviceInfo
		deviceInfo = new DeviceInfo(v4l4jObject, deviceFile);
		
		supportJPEG = deviceInfo.getFormatList().getJPEGEncodableFormats().size()==0?false:true;
		
		//initialise TunerList
		Vector<Tuner> v= new Vector<Tuner>();
		doGetTunerActions(v4l4jObject);
		for(InputInfo i:deviceInfo.getInputs()){
			try {
				v.add(new Tuner(v4l4jObject,i.getTuner().getIndex()));
			} catch (NoTunerException e) {	//no tuner for this input
			}
		}

		if(v.size()!=0)
			tuners = new TunerList(v);
	}
	
	/**
	 * This method release resources used by this VideoDevice. This method WILL <code>wait()</code> if a
	 * <code>FrameGrabber()</code> or/and a <code>ControlList</code> is in use, until 
	 * {@link #releaseFrameGrabber()} or/and {@link #releaseControlList()} have been called. 
	 * @throws StateException if a call to <code>release()</code> is already in progress.
	 */
	public void release(){
		state.release();
		doReleaseTunerActions(v4l4jObject);
		doRelease(v4l4jObject);
		state.commit();
	}	
	
	/**
	 * This method release resources used by this VideoDevice. If the argument <code>wait</code> is true,
	 * this method will <code>wait()</code> if a <code>FrameGrabber()</code> or/and a
	 * <code>ControlList</code> are in use, until {@link #releaseFrameGrabber()} or/and
	 * {@link #releaseControlList()} have been called. It is in effect identical to 
	 * {@link #release()}. If argument <code>wait</code> is false,
	 * this method will throw an <code>InUseException</code> if a <code>FrameGrabber</code> or/and a
	 * <code>ControlList</code> are in use. Otherwise, it will just proceed with the release of 
	 * resources
	 * @param wait whether or not this method should block and wait until {@link #releaseFrameGrabber()}
	 * or/and {@link #releaseControlList()} have been called 
	 * @throws StateException if this video device has already been released, or is being released.
	 * @throws ReleaseException if there either the {@link #releaseFrameGrabber()} or the
	 * {@link #releaseControlList()} have not been called, and we asked to not wait 
	 * (argument <code>wait</code> is false)
	 */
	public void release(boolean wait){
		state.release(wait);
		doRelease(v4l4jObject);
		state.commit();
	}	
	
	/**
	 * This method creates a <code>DeviceInfo</code> object which contains information about
	 * this video device. This method (as well as {@link #getTunerList()} does not have
	 * an equivalent release method. In other word, the returned {@link DeviceInfo}
	 * object does not need to be released before releasing the {@link VideoDevice}.
	 * @return a <code>DeviceInfo</code> object describing this video device.
	 * @see DeviceInfo
	 */
	public DeviceInfo getDeviceInfo(){
		return deviceInfo;		
	}
	
	/**
	 * This method returns a list of {@link Control}s associated with this video device.
	 * The {@link ControlList} must be released when no longer used by calling
	 * {@link VideoDevice#releaseControlList()}.
	 * @return a list of available {@link Control}s 
	 * @throws StateException if the <code>VideoDevice</code> has been released. 
	 */
	public ControlList getControlList(){
		synchronized(this){
			if(controls==null) {
				state.get();
				controls = new ControlList(doGetControlList(v4l4jObject));				
			}
			return controls;
		}
	}
	
	/**
	 * This method releases the list of {@link Control}s returned by {@link #getControlList()}.
	 * This method must be called when the list of {@link Control}s is no longer used, so low-level 
	 * resources can be freed. This method does nothing if a list of {@link Control}s has never been
	 * allocated in the first place.
	 */
	public void releaseControlList(){
		synchronized(this){
			if(controls!=null){
				controls.release();
				doReleaseControlList(v4l4jObject);
				controls = null;
				state.put();
			}
		}	
	}
	
	/**
	 * This method specifies whether frames captured from this video device can be JPEG-encoded before
	 * being handed out. If this video device can capture frames in a native format that can be encoded
	 * in JPEG, then this method returns true, and calls to
	 * {@link #getJPEGFrameGrabber(int, int, int, int, int)} and 
	 * {@link #getJPEGFrameGrabber(int, int, int, int, int, ImageFormat))} will succeed.
	 * If this method returns false,no <code>JPEGFrameGrabber</code>s can be instantiated and  
	 * the only alternative is to use a raw frame grabber, 
	 * returned by {@link #getRawFrameGrabber(int, int, int, int)} or
	 * {@link #getRawFrameGrabber(int, int, int, int, ImageFormat)}.<br>
	 * <b>If JPEGFrameGrabbers cannot be created for your video device, please let the
	 * author know about it so JPEG-encoding can be added. See the README file
	 * on how to submit reports.</b>  
	 * @return whether or not frames captured by this video device can be JPEG-encoded.
	 */
	public boolean canJPEGEncode(){
		return supportJPEG;
	}

	/**
	 * This method returns a <code>FrameGrabber</code> associated with this video device.
	 * Captured frames will be JPEG-encoded before being handed out. The video device 
	 * must support an appropriate image format that v4l4j can convert to JPEG. If it does 
	 * not, this method will throw an {@link ImageFormatException}. To check if 
	 * JPEG-encoding is possible, call {@link #canJPEGEncode()}. The returned 
	 * {@link JPEGFrameGrabber} must be released when no longer used by calling
	 * {@link #releaseFrameGrabber()}.
	 * @param w the desired frame width. This value may be adjusted to the closest
	 * supported by hardware. 
	 * @param h the desired frame height. This value may be adjusted to the closest
	 * supported by hardware. 
	 * @param input the input index, as returned by {@link InputInfo#getIndex()}
	 * @param std the video standard, as returned by {@link InputInfo#getSupportedStandards()}
	 * (see {@link V4L4JConstants})
	 * @param q the JPEG image quality (the higher, the better the quality), within the range
	 * {@link V4L4JConstants#MIN_JPEG_QUALITY}, {@link V4L4JConstants#MAX_JPEG_QUALITY}.
	 * @param imf the {@link ImageFormat} the frames should be captured in before
	 * being JPEG-encoded. This image format must be one that v4l4j knows how to convert
	 * to JPEG, ie it must be in the list returned by this video device's 
	 * {@link ImageFormatList#getJPEGEncodableFormats()}. You can get this video
	 * device's {@link ImageFormatList} by calling <code>getDeviceInfo().getFormatList()</code>.
	 * Also, {@link ImageFormatList#getKnownJPEGEncodableFormats()} returns
	 * a list of all formats that can be JPEG-encoded by v4l4j. If this argument is 
	 * <code>null</code>, v4l4j will pick the first image format it know how to JPEG-encode. 
	 * @return a {@link JPEGFrameGrabber} associated with this video device, if supported.
	 * @throws VideoStandardException if the chosen video standard is not supported
	 * @throws ImageFormatException if the video device uses an unsupported image format
	 * which can not be JPEG-encoded. If you encounter such device, please let the 
	 * author know about it. See README file in v4l4j/ on how to report this issue. 
	 * @throws CaptureChannelException if the given channel number value is not valid
	 * @throws ImageDimensionException if the given image dimensions are not supported
	 * @throws InitialisationException if the video device file can not be initialised 
	 * @throws V4L4JException if there is an error applying capture parameters
	 * @throws StateException if a {@link FrameGrabber} already exists and must be released 
	 * before a JPEGFrameGrabber can be allocated, or if the <code>VideoDevice</code>
	 * has been released.
	 * <b>If JPEGFrameGrabbers cannot be created for your video device, please let the
	 * author know about it so JPEG-encoding can be added. See the README file
	 * on how to submit reports.</b>  
	 */
	public JPEGFrameGrabber getJPEGFrameGrabber(int w, int h, int input, int std, int q, ImageFormat imf) throws V4L4JException{
		if(!supportJPEG)
			throw new ImageFormatException("This video device does not support JPEG-encoding of its frames.");
		
		if(imf!=null && !deviceInfo.getFormatList().getJPEGEncodableFormats().contains(imf)){
			String msg = "The image format "+imf.getName()+" cannot be JPEG encoded.\n"
						+ "Please let the author know about this, so that support\n"
						+ "for this image format can be added to v4l4j. See \nREADME file "
						+ "on how to submit v4l4j reports.";
			System.err.println(msg);
			throw new ImageFormatException(msg);
		}
		
		synchronized(this){
			if(fg==null) {
				state.get();
				fg = new JPEGFrameGrabber(v4l4jObject, w, h, input, std, q, findTuner(input), imf);
				try {
					fg.init();
				} catch (V4L4JException ve){
					fg = null;
					state.put();
					throw ve;
				}  catch (StateException se){
					fg = null;
					state.put();
					throw se;
				}  catch (Throwable t){
					fg = null;
					state.put();
					throw new V4L4JException("Error", t);
				}
				return (JPEGFrameGrabber) fg;
			} else {
				if(fg.getClass().isInstance(JPEGFrameGrabber.class))
					return (JPEGFrameGrabber) fg;
				else {
					state.put();
					throw new StateException("A RawFrameGrabber object already exists");
				}
			}
		}
	}
	
	/**
	 * This method returns a <code>FrameGrabber</code> associated with this video device.
	 * Captured frames will be JPEG-encoded before being handed out. The video device 
	 * must support an appropriate image format that v4l4j can convert to JPEG. If it does 
	 * not, this method will throw an {@link ImageFormatException}. To check if 
	 * JPEG-encoding is possible, call {@link #canJPEGEncode()}. Among all the image
	 * formats the video device supports, v4l4j will choose the first one that can be
	 * JPEG encoded. If you prefer to specify which image format is to be used, call
	 * {@link #getJPEGFrameGrabber(int, int, int, int, int, ImageFormat)} instead. This is 
	 * sometimes required because some video device have a lower frame rate with some
	 * image formats, and a higher one with others. So far, testing is the only way to 
	 * find out.
	 * The returned 
	 * {@link JPEGFrameGrabber} must be released when no longer used by calling
	 * {@link #releaseFrameGrabber()}.
	 * @param w the desired frame width. This value may be adjusted to the closest
	 * supported by hardware. 
	 * @param h the desired frame height. This value may be adjusted to the closest
	 * supported by hardware. 
	 * @param input the input index, as returned by {@link InputInfo#getIndex()}
	 * @param std the video standard, as returned by {@link InputInfo#getSupportedStandards()}
	 * (see {@link V4L4JConstants})
	 * @param q the JPEG image quality (the higher, the better the quality), within the range
	 * {@link V4L4JConstants#MIN_JPEG_QUALITY}, {@link V4L4JConstants#MAX_JPEG_QUALITY}.
	 * @return a {@link JPEGFrameGrabber} associated with this video device, if supported.
	 * @throws VideoStandardException if the chosen video standard is not supported
	 * @throws ImageFormatException if the video device uses an unsupported image format
	 * which can not be JPEG-encoded. If you encounter such device, please let the 
	 * author know about it. See README file in v4l4j/ on how to report this issue. 
	 * @throws CaptureChannelException if the given channel number value is not valid
	 * @throws ImageDimensionException if the given image dimensions are not supported
	 * @throws InitialisationException if the video device file can not be initialised 
	 * @throws V4L4JException if there is an error applying capture parameters
	 * @throws StateException if a {@link FrameGrabber} already exists and must be released 
	 * before a JPEGFrameGrabber can be allocated, or if the <code>VideoDevice</code>
	 * has been released.
	 * <b>If JPEGFrameGrabbers cannot be created for your video device, please let the
	 * author know about it so JPEG-encoding can be added. See the README file
	 * on how to submit reports.</b>  
	 */
	public JPEGFrameGrabber getJPEGFrameGrabber(int w, int h, int input, int std, int q) throws V4L4JException{
		return getJPEGFrameGrabber(w, h, input, std, q, null);
	}
	
	/**
	 * This method returns a <code>FrameGrabber</code> associated with this video device.
	 * Captured frames will be handed out in the same format as received from the driver 
	 * and can be chosen amongst the ones supported by this video device. To enumerated 
	 * the supported {@link ImageFormat}s, check the {@link ImageFormatList} returned by
	 *  {@link DeviceInfo#getFormatList()}.
	 * The returned {@link FrameGrabber} must be released when no longer used by calling
	 * {@link #releaseFrameGrabber()}.
	 * @param w the desired frame width. This value may be adjusted to the closest
	 * supported by hardware. 
	 * @param h the desired frame height. This value may be adjusted to the closest
	 * supported by hardware. 
	 * @param input the input index, as returned by {@link InputInfo#getIndex()}.
	 * @param std the video standard, as returned by {@link InputInfo#getSupportedStandards()}.
	 * (see {@link V4L4JConstants})
	 * @param format the desired image format. A list of supported {@link ImageFormat}s 
	 * can be obtained by calling <code>getDeviceInfo().getFormats()</code>. If this argument
	 * is <code>null</code>, an @{link {@link ImageFormatException} is thrown. 
	 * @return the <code>FrameGrabber</code> associated with this video device
	 * @throws VideoStandardException if the chosen video standard is not supported
	 * @throws ImageFormatException if the selected video device uses an unsupported 
	 * image format (let the author know, see README file).
	 * @throws CaptureChannelException if the given channel number value is not valid
	 * @throws ImageDimensionsException if the given image dimensions are not supported
	 * @throws InitialisationException if the video device file can not be initialised 
	 * @throws V4L4JException if there is an error applying capture parameters
	 * @throws StateException if a <code>FrameGrabber</code> already exists or if the 
	 * <code>VideoDevice</code> has been released.
	 */
	public FrameGrabber getRawFrameGrabber(int w, int h, int input, int std, ImageFormat format) throws V4L4JException{
		if(format==null)
			throw new ImageFormatException("The image format can not be null");
		
		synchronized(this){
			if(fg==null) {
				state.get();
				fg = new FrameGrabber(v4l4jObject, w, h, input, std, findTuner(input), format);
				try {
					fg.init();
				} catch (V4L4JException ve){
					fg = null;
					state.put();
					throw ve;
				}  catch (StateException se){
					fg = null;
					state.put();
					throw se;
				} catch (Throwable t){
					fg = null;
					state.put();
					throw new V4L4JException("Error", t);
				}
				return fg;
			} else {
				if(fg.getClass().isInstance(FrameGrabber.class))
					return fg;
				else {
					state.put();
					throw new StateException("A JPEGFrameGrabber object already exists");
				}
			}
		}
	}
	
	/**
	 * This method returns a <code>FrameGrabber</code> associated with this video device.
	 * Captured frames will be handed out in the same format as received from the driver. 
	 * The chosen format is the one returned by
	 * <code>getDeviceInfo().getFormatList().get(0)</code>. The {@link FrameGrabber} 
	 * must be released when no longer used by calling
	 * {@link #releaseFrameGrabber()}.
	 * @param w the desired frame width. This value may be adjusted to the closest
	 * supported by hardware. 
	 * @param h the desired frame height. This value may be adjusted to the closest
	 * supported by hardware. 
	 * @param input the input index, as returned by {@link InputInfo#getIndex()}.
	 * @param std the video standard, as returned by {@link InputInfo#getSupportedStandards()}
	 * (see {@link V4L4JConstants})
	 * @return the <code>FrameGrabber</code> associated with this video device.
	 * @throws VideoStandardException if the chosen video standard is not supported.
	 * @throws ImageFormatException if the selected video device uses an unsupported 
	 * image format (let the author know, see README file).
	 * @throws CaptureChannelException if the given channel number value is not valid.
	 * @throws ImageDimensionException if the given image dimensions are not supported.
	 * @throws InitialisationException if the video device file can not be initialised .
	 * @throws V4L4JException if there is an error applying capture parameters
	 * @throws StateException if a <code>FrameGrabber</code> already exists or if 
	 * the <code>VideoDevice</code> has been released.
	 */
	public FrameGrabber getRawFrameGrabber(int w, int h, int input, int std) throws V4L4JException{
		return getRawFrameGrabber(w, h, input, std, deviceInfo.getFormatList().getList().get(0));
	}
	
	/**
	 * This method releases the <code>FrameGrabber</code> object allocated previously with
	 * <code>getJPEGFrameGrabber()</code> or <code>getRawFrameGrabber()</code>.
	 * This method must be called when the <code>FrameGrabber</code> object is no longer
	 * used, so low-level resources can be freed. This method does nothing if no 
	 * <code>FrameGrabber</code> object has been allocated in the first place.
	 */
	public void releaseFrameGrabber() {
		synchronized(this){
			if(fg!=null){
				try {fg.release();}
				catch (Throwable t){
					t.printStackTrace();
					throw new StateException("Cant release resources used by framegrabber", t);
				}
				fg = null;
				state.put();

			}
		}	
	}

	/**
	 * This method returns the {@link TunerList} of {@link Tuner}s available on this
	 * {@link VideoDevice}. {@link Tuner}s in this list can be used to control 
	 * individual physical tuners on the video device and get signal information form them.
	 * Check the documentation of the {@link Tuner} class for more information.
	 * This method (as well as {@link #getDeviceInfo()} does not have
	 * an equivalent release method. In other word, the returned {@link TunerList}
	 * object does not need to be released before releasing the {@link VideoDevice}.
	 * @return a {@link TunerList} 
	 * @throws NoTunerException if this video device does not have any tuners.
	 */
	public TunerList getTunerList() throws NoTunerException{
		if(tuners==null)
			throw new NoTunerException("This video device does not have any tuners");
		return tuners;
	}
	
	
	private Tuner findTuner(int input){
		for(InputInfo i: deviceInfo.getInputs())
			if(i.getIndex() == input && i.hasTuner())
				try {
					return tuners.getTuner(i.getTuner().getIndex());
				} catch (NoTunerException e) {
					//weird, shoudlnt be here
				}
		
		return null;

	}
	
	private static class State {

		private int state;
		private int temp;
		private int users;
		
		private int INIT=1;
		private int RELEASED=2;

		public State() {
			state = INIT;
			temp = INIT;
			users = 0;
		}

		public synchronized void get(){
			if(state==INIT && temp!=RELEASED) {
				users++;
			} else
				throw new StateException("This VideoDevice has not been initialised or is about to be removed, and can not be used");
		}
		
		public synchronized void put(){
			if(state==INIT) {
				if(--users==0  && temp==RELEASED)
					notify();
			} else
				throw new StateException("This VideoDevice has not been initialised and can not be used");
		}
		
		/**
		 * This method switched to the released state. This method will wait
		 * until all users have finished.
		 * @return whether we can switch to the released state or not
		 */
		public synchronized void release(){
			release(true);
		}
		
		/**
		 * This method switched to the released state
		 * @param wait whether we want to wait for potential users or not. If we do, calls to this method
		 * will block until all users are finished. If we dont (wait = false), this method will throw a
		 * V4L4JException if there are users
		 * @return whether we can switch to the released state or not
		 * @throws ReleaseException if there are still some users and we have chosen not to wait
		 */
		public synchronized void release(boolean wait){
			int t = temp;
			if(state==INIT && temp!=RELEASED) {
				temp=RELEASED;
				while(users!=0)
					try {
						if(wait==true)
							wait();
						else {
							temp = t;
							throw new ReleaseException("Still in use");	
						}
					} catch (InterruptedException e) {
						temp = t;
						System.err.println("Interrupted while waiting for VideoDevice users to complete");
						e.printStackTrace();
						throw new StateException("There are remaining users of this VideoDevice and it can not be released");
					}
				return;
			}
			throw new StateException("This VideoDevice has not been initialised and can not be used");
		}
		
		public synchronized void commit(){
			state=temp;
		}
	}

	public static void main(String args[]) throws V4L4JException{
		VideoDevice vd = new VideoDevice(args[0]);
		DeviceInfo d = vd.getDeviceInfo(); 
		System.out.println("name: "+d.getName());
		System.out.println("Device file: "+d.getDeviceFile());
		System.out.println("Supported formats:");
		for(ImageFormat f : d.getFormatList().getList())
			System.out.println("\t"+f.getName()+" - "+f.getIndex());
		
		System.out.println("Some formats can be JPEG-encoded? "
				+(vd.canJPEGEncode()?"Yes":"No"));
		if(vd.canJPEGEncode()){
			System.out.println("List of Formats that can be JPEG-encoded:");
			for(ImageFormat f: d.getFormatList().getJPEGEncodableFormats())
				System.out.println("\t"+f.getName()+" - "+f.getIndex());
		}
		
		System.out.println("Inputs:");
		for(InputInfo i: d.getInputs()){
			System.out.println("\tName: "+i.getName());
			System.out.println("\tType: "+i.getType()+"("+(i.getType() == V4L4JConstants.CAMERA ? "Camera" : "Tuner")+")");
			System.out.println("\tIndex: "+i.getIndex());
			System.out.println("\tSupported standards:");
			for(Integer s: i.getSupportedStandards()){
				System.out.print("\t\t"+s);
				if(s==V4L4JConstants.PAL)
					System.out.println("(PAL)");
				else if(s==V4L4JConstants.NTSC)
					System.out.println("(NTSC)");
				else if(s==V4L4JConstants.SECAM)
					System.out.println("(SECAM)");
				else
					System.out.println("(None/Webcam)");
			}
			if(i.getType() == V4L4JConstants.TUNER) {
				TunerInfo t = i.getTuner();
				System.out.println("\tTuner");
				System.out.println("\t\tname: "+t.getName());
				System.out.println("\t\tIndex: "+t.getIndex());
				System.out.println("\t\tRange high: "+t.getRangeHigh());
				System.out.println("\t\tRange low: "+t.getRangeLow());
				System.out.println("\t\tUnit: "+t.getUnit()+"("+(t.getUnit() == V4L4JConstants.MHZ ? "MHz" : "kHz")+")");
				System.out.println("\t\tType: "+t.getType()+"("+(t.getType() == V4L4JConstants.RADIO ? "Radio" : "TV")+")");				
			}
		}
		
		List<Control> ctrls = vd.getControlList().getList();
		for(Control c: ctrls){
			System.out.print("Control: "+c.getName()+" - min: "+c.getMinValue()+" - max: "+c.getMaxValue()+" - step: "+c.getStepValue());
			try {
				System.out.println(" - value: "+c.getValue());
			} catch (V4L4JException ve){
				System.out.println();
			}
		}
		vd.releaseControlList();
		vd.release();
	}
}
