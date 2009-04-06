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
 * An instance of a <code>VideoDevice</code> object represents an existing V4L 
 * video device. It is the starting point to use functionalities provided by 
 * v4l4j, which are divided in 3 categories:
 * <ul>
 * <li>Information gathering about the video device,</li>
 * <li>Capturing frames from the video device,</li>
 * <li>and Adjusting values of available video controls.</li>
 * </ul>
 * Each of these categories is detailed in the following sections.
 * To use features provided in any of these category, a <code>VideoDevice</code>
 * object must first be instantiated. This is done simply by calling the 
 * constructor and giving it the full path to the associated device file:
 * <br><code>
 * VideoDevice vd = new VideoDevice("/dev/video0");
 * </code><br>
 * <b>It is important that, once the <code>VideoDevice</code> is no longer used,
 * its resources are freed. This is done by calling the {@link #release()}
 * method:</b>
 * <br><code>vd.release();</code>
 * <br>Once the <code>VideoDevice</code> is released, neither itself nor any
 * object references obtained through it must be used.
 * <h2>Information gathering</h2>
 * A <code>VideoDevice</code> object offers a single method 
 * ({@link #getDeviceInfo()}) to return information about itself, encapsulated 
 * in a {@link DeviceInfo} object. DeviceInfo objects contain
 * get-type methods only, and return details such as:
 * <ul>
 * <li>List of all video inputs, their types & their supported standards</li>
 * <li>Details about available tuners (type, frequency range & unit, ...) </li>
 * <li>List of image formats supported by the device</li>
 * </ul>
 * See the {@link DeviceInfo} class for an example on how to use it. Note that
 * most video device drivers do not allow multiple processes to open the device
 * at the same time. Depending on the driver, this restriction can be seen in
 * different ways. One of them is {@link #getDeviceInfo()} will throw a
 * {@link V4L4JException} if a process is already using the device.
 * <h2>Capturing frames</h2>
 * In order to capture frames from a video device, an instance of a 
 * <code>FrameGrabber</code> object must be obtained. The 
 * <code>VideoDevice</code> object provides 2 methods for that, depending on the
 * image format required:
 * <ul>
 *  
 * <li>The <code>getRGBFrameGrabber()</code> & <code>getBGRFrameGrabber()</code>
 * methods return either an {@link RGBFrameGrabber} or {@link BGRFrameGrabber} 
 * object capable of capturing frames and converting  them to RGB24 / BGR24
 * before handing them out. Here again, these frame grabbers cannot be used with
 * all video sources. They require images from the video source to 
 * be in some pre-defined formats in order to be converted to RGB24 or BGR24. If
 * the video source is not capable of handing out images in one of the supported
 * formats, then no <code>RGBFrameGrabber</code> / <code>BGRFrameGrabber</code>
 * can be created.</li>
 * 
 * <li>The <code>getYUVFrameGrabber()</code> & <code>getYVUFrameGrabber()</code>
 * methods return either an {@link YUVFrameGrabber} or {@link YVUFrameGrabber} 
 * object capable of capturing frames and converting  them to YUV420 / YVU420
 * before handing them out. Here again, these frame grabbers cannot be used with
 * all video sources. They require images from the video source to 
 * be in some pre-defined formats in order to be converted to YUV420 or YVU420.
 * If the video source is not capable of handing out images in one of the 
 * supported formats, then no <code>YUVFrameGrabber</code> / 
 * <code>YVUFrameGrabber</code> can be created.</li>
 * 
 * <li>The <code>getRawFrameGrabber()</code> methods return a 
 * {@link FrameGrabber} object capable of capturing frames in one of the 
 * supported image formats, as reported by the 
 * {@link DeviceInfo#getFormatList() getFormatList()} method of 
 * {@link DeviceInfo} objects. Captured frames are handed out straight away to 
 * the caller, without any other form of processing.</li>
 * <li>The <code>getJPEGFrameGrabber()</code> methods return a 
 * {@link JPEGFrameGrabber} object capable of capturing frames and JPEG-encoding
 * them before handing them out. This frame grabber cannot be used with all 
 * video sources. It requires images from the video source to be in some 
 * pre-defined formats (namely JPEG, MJPEG, YUV420, YUYV, RGB24, RGB32, BGR24, 
 * YVYU, UYVY and BGR32) in order to be JPEG-encoded. If the video source is not
 * capable of producing images in one of these formats, then no 
 * <code>JPEGFrameGrabber</code> can be created.</li>
 * 
 * </ul>
 * To check whether JPEG, RGB, BGR, YUV or YVU conversion is supported by a 
 * <code>VideoDevice</code> object, call its {@link #supportJPEGConversion()}, 
 * {@link #supportRGBConversion()}, {@link #supportBGRConversion()},
 * {@link #supportYUVConversion()} or {@link #supportYVUConversion()} method.
 * <b>Similarly to <code>VideoDevice</code> objects, once the frame grabber is 
 * no longer used, its resources must be released.</b> This is achieved by 
 * calling the {@link #releaseFrameGrabber()}. See the 
 * {@link FrameGrabber} class for more information on how to capture frames.
 * 
 * <h2>Video controls</h2>
 * A list of available video controls can be obtained by calling the 
 * {@link #getControlList()} method. Each control contains detailed information 
 * about its type and acceptable values. Controls have a pair of get & set 
 * methods to adjust their values. Once again, when no longer used, resources 
 * taken up by video controls must be released by calling 
 * {@link #releaseControlList()}. See the {@link ControlList} and 
 * {@link Control} classes for more information on how to use them.
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
	 * This JNI method releases resources used by libvideo's struct 
	 * video_device, as allocated by <code>doInit()</code>
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
	 * @throws ReleaseException if this device is sill in use, and has not been
	 * released.
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
	 * @throws ReleaseException if this device is sill in use, and has not been
	 * released.
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
	 * Whether or not frames captured from this video device can be 
	 * JPEG, RGB24, BGR24, YUV420 or YVU420-encoded 
	 */
	private boolean supportJPEG, supportRGB24, supportBGR24, 
					supportYUV420, supportYVU420;
	
	/**
	 * JNI returns a long (which is really a pointer) when a device is allocated
	 * for use. This field is read-only (!!!) 
	 */
	private long v4l4jObject;
	
	/**
	 * This constructor builds a <code>VideoDevice</code> using the full path to
	 * its device file. When finished, resources must be released by calling 
	 * {@link #release()}. 
	 * @param dev the path to the device file
	 * @throws V4L4JException if the device file is not accessible
	 */
	public VideoDevice(String dev) throws V4L4JException{
		if(!(new File(dev).canRead()))
			throw new V4L4JException("The device file is not readable");

		state = new State();		
		deviceFile = dev;
		v4l4jObject = doInit(deviceFile);
		
		try {
			initDeviceInfo();
		} catch (V4L4JException e){
			//error getting DeviceInfo
			//keep going but set things accordingly
			deviceInfo = null;
			
			supportJPEG = false;
			supportRGB24 = false;
			supportBGR24 = false;
			supportYUV420 = false;
			supportYVU420 = false;
		}
	}
	
	/**
	 * This method initialises this VideoDevice with the information obtained 
	 * from the {@link DeviceInfo}. This method must be called 
	 * before any other methods.
	 * @throws V4L4JException if the device can not be initialised
	 */
	private void initDeviceInfo() throws V4L4JException{		
		//initialise deviceInfo
		deviceInfo = new DeviceInfo(v4l4jObject, deviceFile);
		ImageFormatList l = deviceInfo.getFormatList();
		
		supportJPEG = l.getJPEGEncodableFormats().size()==0?false:true;
		supportRGB24 = l.getRGBEncodableFormats().size()==0?false:true;
		supportBGR24 = l.getBGREncodableFormats().size()==0?false:true;
		supportYUV420 = l.getYUVEncodableFormats().size()==0?false:true;
		supportYVU420 = l.getYVUEncodableFormats().size()==0?false:true;
		
		//initialise TunerList
		Vector<Tuner> v= new Vector<Tuner>();
		doGetTunerActions(v4l4jObject);
		for(InputInfo i:deviceInfo.getInputs()){
			try {
				v.add(new Tuner(v4l4jObject,i.getTunerInfo()));
			} catch (NoTunerException e) {}	//no tuner for this input
		}

		if(v.size()!=0)
			tuners = new TunerList(v);
	}
	
	/**
	 * This method releases resources used by this VideoDevice. This method WILL
	 * <code>wait()</code> if a <code>FrameGrabber</code> or/and a 
	 * <code>ControlList</code> is in use, until {@link #releaseFrameGrabber()} 
	 * or/and {@link #releaseControlList()} have been called. 
	 * @throws StateException if a call to <code>release()</code> is already in
	 * progress.
	 */
	public void release(){
		state.release();
		doReleaseTunerActions(v4l4jObject);
		doRelease(v4l4jObject);
		state.commit();
	}	
	
	/**
	 * This method releases resources used by this VideoDevice. If the argument 
	 * <code>wait</code> is true, this method will <code>wait()</code> if a 
	 * <code>FrameGrabber</code> or/and a <code>ControlList</code> are in use, 
	 * until {@link #releaseFrameGrabber()} or/and {@link #releaseControlList()}
	 * have been called. It is in effect identical to {@link #release()}. If 
	 * argument <code>wait</code> is false, this method will throw a 
	 * {@link ReleaseException} if a <code>FrameGrabber</code> or/and a
	 * <code>ControlList</code> are in use. Otherwise, it will just proceed with
	 * the release of resources.
	 * @param wait whether or not this method should block and wait until 
	 * {@link #releaseFrameGrabber()} or/and {@link #releaseControlList()} have 
	 * been called 
	 * @throws StateException if this video device has already been released, or
	 * is being released.
	 * @throws ReleaseException if there either the 
	 * {@link #releaseFrameGrabber()} or the {@link #releaseControlList()} have 
	 * not been called, and we asked to not wait (argument <code>wait</code> is 
	 * false)
	 */
	public void release(boolean wait){
		state.release(wait);
		doRelease(v4l4jObject);
		state.commit();
	}	
	
	/**
	 * This method creates a <code>DeviceInfo</code> object which contains 
	 * information about this video device. This method (as well as 
	 * {@link #getTunerList()} does not have an equivalent release method. 
	 * In other word, the returned {@link DeviceInfo} object does not need to be
	 * released before releasing the {@link VideoDevice}.
	 * @return a <code>DeviceInfo</code> object describing this video device.
	 * @throws V4L4JException if there was an error gathering information
	 * about the video device. This happens for example, when another 
	 * application is currently using the device. 
	 * @see DeviceInfo
	 */
	public DeviceInfo getDeviceInfo() throws V4L4JException{
		if(deviceInfo!=null)
			return deviceInfo;
		
		throw new V4L4JException("Error getting information about device");
	}
	
	/**
	 * This method returns the full path to the device file associated with this
	 * video device.
	 * @return the full path to the device file
	 */
	public String getDevicefile() {
		return deviceFile;
	}
	
	/**
	 * This method returns a list of {@link Control}s associated with this video
	 *  device.
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
	 * This method releases the list of {@link Control}s returned by 
	 * {@link #getControlList()}. This method must be called when the list of 
	 * {@link Control}s is no longer used, so low-level resources can be freed.
	 * This method does nothing if a list of {@link Control}s has never been
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
	 * This method specifies whether frames captured from this video device can
	 * be JPEG-encoded before being handed out. If this video device can capture
	 * frames in a native format that can be encoded in JPEG, then this method 
	 * returns true, and calls to
	 * {@link #getJPEGFrameGrabber(int, int, int, int, int)} and 
	 * {@link #getJPEGFrameGrabber(int, int, int, int, int, ImageFormat)} will 
	 * succeed. If this method returns false,no <code>JPEGFrameGrabber</code>s 
	 * can be instantiated. One alternative is to use a raw frame grabber, 
	 * returned by {@link #getRawFrameGrabber(int, int, int, int)} or
	 * {@link #getRawFrameGrabber(int, int, int, int, ImageFormat)}.<br>
	 * <b>If JPEGFrameGrabbers cannot be created for your video device, please 
	 * let the author know about it so JPEG-encoding can be added. See the 
	 * README file on how to submit reports.</b>  
	 * @return whether or not frames captured by this video device can be 
	 * JPEG-encoded.
	 */
	public boolean supportJPEGConversion(){
		return supportJPEG;
	}
	
	/**
	 * This method specifies whether frames captured from this video device can 
	 * be converted to RGB24 before being handed out. If this video device can 
	 * capture frames in a native format that can be converted to RGB24, then 
	 * this method returns true, and calls to
	 * {@link #getRGBFrameGrabber(int, int, int, int)} and 
	 * {@link #getRGBFrameGrabber(int, int, int, int, ImageFormat)} will succeed.
	 * If this method returns false,no <code>RGBFrameGrabber</code>s can be 
	 * instantiated. One alternative is to use a raw frame grabber, returned by 
	 * {@link #getRawFrameGrabber(int, int, int, int)} or
	 * {@link #getRawFrameGrabber(int, int, int, int, ImageFormat)}.<br>
	 * <b>If RGBFrameGrabbers cannot be created for your video device, please 
	 * let the author know about it so RGB24-encoding can be added. See the 
	 * README file on how to submit reports.</b>  
	 * @return whether or not frames captured by this video device can be 
	 * RGB24-encoded.
	 */
	public boolean supportRGBConversion(){
		return supportRGB24;
	}
	
	/**
	 * This method specifies whether frames captured from this video device can 
	 * be converted to BGR24 before being handed out. If this video device can 
	 * capture frames in a native format that can be converted to BGR24, then 
	 * this method returns true, and calls to
	 * {@link #getBGRFrameGrabber(int, int, int, int)} and 
	 * {@link #getBGRFrameGrabber(int, int, int, int, ImageFormat)} will succeed.
	 * If this method returns false,no <code>BGRFrameGrabber</code>s can be 
	 * instantiated.  
	 * One alternative is to use a raw frame grabber, 
	 * returned by {@link #getRawFrameGrabber(int, int, int, int)} or
	 * {@link #getRawFrameGrabber(int, int, int, int, ImageFormat)}.<br>
	 * <b>If BGRFrameGrabbers cannot be created for your video device, please
	 * let the author know about it so BGR24-encoding can be added. See the 
	 * README file on how to submit reports.</b>  
	 * @return whether or not frames captured by this video device can be 
	 * BGR24-encoded.
	 */
	public boolean supportBGRConversion(){
		return supportBGR24;
	}

	/**
	 * This method specifies whether frames captured from this video device can 
	 * be converted to YUV420 before being handed out. If this video device can 
	 * capture frames in a native format that can be converted to YUV420, then 
	 * this method returns true, and calls to
	 * {@link #getYUVFrameGrabber(int, int, int, int)} and 
	 * {@link #getYUVFrameGrabber(int, int, int, int, ImageFormat)} will succeed.
	 * If this method returns false, no <code>YUVFrameGrabber</code>s can be 
	 * instantiated. One alternative is to use a raw frame grabber, 
	 * returned by {@link #getRawFrameGrabber(int, int, int, int)} or
	 * {@link #getRawFrameGrabber(int, int, int, int, ImageFormat)}.<br>
	 * <b>If a YUVFrameGrabber cannot be created for your video device, please 
	 * let the author know about it so YUV-encoding can be added. 
	 * See the README file on how to submit reports.</b>  
	 * @return whether or not frames captured by this video device can be 
	 * YUV420-encoded.
	 */
	public boolean supportYUVConversion(){
		return supportYUV420;
	}
	
	/**
	 * This method specifies whether frames captured from this video device can 
	 * be converted to YVU420 before being handed out. If this video device can 
	 * capture frames in a native format that can be converted to YVU420, then 
	 * this method returns true, and calls to
	 * {@link #getYVUFrameGrabber(int, int, int, int)} and 
	 * {@link #getYVUFrameGrabber(int, int, int, int, ImageFormat)} will succeed.
	 * If this method returns false, no <code>YVUFrameGrabber</code>s can be 
	 * instantiated. One alternative is to use a raw frame grabber, 
	 * returned by {@link #getRawFrameGrabber(int, int, int, int)} or
	 * {@link #getRawFrameGrabber(int, int, int, int, ImageFormat)}.<br>
	 * <b>If a YVUFrameGrabber cannot be created for your video device, please 
	 * let the author know about it so YVU-encoding can be added. 
	 * See the README file on how to submit reports.</b>  
	 * @return whether or not frames captured by this video device can be 
	 * YVU420-encoded.
	 */
	public boolean supportYVUConversion(){
		return supportYVU420;
	}
	
	/**
	 * This method returns a <code>FrameGrabber</code> associated with this 
	 * video device. Captured frames will be JPEG-encoded before being handed 
	 * out. To obtain a JPEGFrameGrabber, this video device must support an 
	 * appropriate image format that v4l4j can convert to JPEG. If it does 
	 * not, this method will throw an {@link ImageFormatException}. To check if 
	 * JPEG-encoding is possible, call {@link #supportJPEGConversion()}. The 
	 * list of image formats supported by this video device that can be 
	 * JPEG-encoded is obtained by calling 
	 * {@link ImageFormatList#getJPEGEncodableFormats()}
	 * (<code>getDeviceInfo().getFormatList().getJPEGEncodableFormats()</code>).
	 * The returned {@link JPEGFrameGrabber} must be released when no longer 
	 * used by calling
	 * {@link #releaseFrameGrabber()}.<br><b>If JPEGFrameGrabbers cannot be 
	 * created for your video device, please let the author know so support for 
	 * it can be added. See the README file on how to submit reports.</b> 
	 * @param w the desired frame width. This value may be adjusted to the 
	 * closest supported by hardware. 
	 * @param h the desired frame height. This value may be adjusted to the 
	 * closest supported by hardware. 
	 * @param input the input index, as returned by {@link InputInfo#getIndex()}
	 * @param std the video standard, as returned by 
	 * {@link InputInfo#getSupportedStandards()} (see {@link V4L4JConstants})
	 * @param q the JPEG image quality (the higher, the better the quality), 
	 * within the range {@link V4L4JConstants#MIN_JPEG_QUALITY}, 
	 * {@link V4L4JConstants#MAX_JPEG_QUALITY}.
	 * @param imf the {@link ImageFormat} the frames should be captured in 
	 * before being JPEG-encoded. This image format must be one that v4l4j can
	 * convert to JPG, ie it must be in the list returned by this video device's 
	 * {@link ImageFormatList#getJPEGEncodableFormats()}. You can get this video
	 * device's {@link ImageFormatList} by calling 
	 * <code>getDeviceInfo().getFormatList()</code>. If this argument is 
	 * <code>null</code>, v4l4j will pick the first image format it knows how to
	 * JPEG-encode. 
	 * @return a {@link JPEGFrameGrabber} associated with this video device, 
	 * if supported.
	 * @throws VideoStandardException if the chosen video standard is not
	 * supported
	 * @throws ImageFormatException if the chosen image format can not be 
	 * JPEG-encoded. If no image format was specified (<code>null</code>), then 
	 * this video device does not have an image format that can be JPEG encoded.
	 * <b>If you encounter such device, please let the author know so support for 
	 * it can be added. See the README file on how to submit reports.</b> 
	 * @throws CaptureChannelException if the given channel number value is not 
	 * valid
	 * @throws ImageDimensionException if the given image dimensions are not
	 * supported
	 * @throws InitialisationException if the video device file can not be
	 * initialised 
	 * @throws V4L4JException if there is an error applying capture parameters
	 * @throws StateException if a {@link FrameGrabber} already exists and must 
	 * be released before a JPEGFrameGrabber can be allocated, or if the 
	 * <code>VideoDevice</code> has been released. 
	 */
	public JPEGFrameGrabber getJPEGFrameGrabber(int w, int h, int input, 
			int std, int q, ImageFormat imf) throws V4L4JException{
		if(!supportJPEG || deviceInfo==null)
			throw new ImageFormatException("This video device does not support "
					+"JPEG-encoding of its frames.");
		
		if(imf!=null){
			if(!deviceInfo.getFormatList().
					getJPEGEncodableFormats().contains(imf))
			
				throw new ImageFormatException(
						"The image format "+imf.getName()+" cannot be JPEG "
						+"encoded.");
		} else 
			//if imf is null, pick the first format that can be rgb encoded
			//the list returned by getJPEGEncodableFormats() is sorted by best 
			//format first, and if we re here, we know there is at least one 
			//format in there
			imf = deviceInfo.getFormatList().getJPEGEncodableFormats().get(0);
	

		synchronized(this){
			if(fg==null) {
				state.get();
				fg = new JPEGFrameGrabber(v4l4jObject, w, h, input, std, q, 
						findTuner(input), imf);
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
					throw new StateException("A FrameGrabber object already "
							+"exists");
				}
			}
		}
	}
	
	/**
	 * This method returns a <code>FrameGrabber</code> associated with this 
	 * video device. Captured frames will be JPEG-encoded before being handed 
	 * out. The video device must support an appropriate image format that v4l4j
	 * can convert to JPEG. If it does not, this method will throw an 
	 * {@link ImageFormatException}. To check if JPEG-encoding is possible, 
	 * call {@link #supportJPEGConversion()}. Among all the image formats the 
	 * video device supports, v4l4j will choose the first one that can be JPEG 
	 * encoded. If you prefer to specify which image format is to be used, call
	 * {@link #getJPEGFrameGrabber(int, int, int, int, int, ImageFormat)} 
	 * instead. This is sometimes required because some video devices may have a 
	 * lower frame rate with some image formats, and a higher one with others. 
	 * So far, testing is the only way to find out. The returned 
	 * {@link JPEGFrameGrabber} must be released when no longer used by calling
	 * {@link #releaseFrameGrabber()}.<br>
	 * <b>If a JPEGFrameGrabber cannot be created for your video device, please 
	 * let the author know so support for it can be added. See the 
	 * README file on how to submit reports.</b> 
	 * @param w the desired frame width. This value may be adjusted to the 
	 * closest supported by hardware. 
	 * @param h the desired frame height. This value may be adjusted to the 
	 * closest supported by hardware. 
	 * @param input the input index, as returned by {@link InputInfo#getIndex()}
	 * @param std the video standard, as returned by 
	 * {@link InputInfo#getSupportedStandards()} (see {@link V4L4JConstants})
	 * @param q the JPEG image quality (the higher, the better the quality), 
	 * within the range {@link V4L4JConstants#MIN_JPEG_QUALITY}, 
	 * {@link V4L4JConstants#MAX_JPEG_QUALITY}.
	 * @return a {@link JPEGFrameGrabber} associated with this video device, 
	 * if supported.
	 * @throws VideoStandardException if the chosen video standard is not 
	 * supported
	 * @throws ImageFormatException if the video device does not have any image 
	 * formats which can be JPEG-encoded.<b>If you encounter such device, please 
	 * let the author know so support for it can be added. See the 
	 * README file on how to submit reports.</b> 
	 * @throws CaptureChannelException if the given channel number value is not
	 * valid
	 * @throws ImageDimensionException if the given image dimensions are not 
	 * supported
	 * @throws InitialisationException if the video device file can not be 
	 * initialised 
	 * @throws V4L4JException if there is an error applying capture parameters
	 * @throws StateException if a {@link FrameGrabber} already exists and must
	 * be released before a JPEGFrameGrabber can be allocated, or if the 
	 * <code>VideoDevice</code> has been released. 
	 */
	public JPEGFrameGrabber getJPEGFrameGrabber(int w, int h, int input, 
			int std, int q) throws V4L4JException{
		return getJPEGFrameGrabber(w, h, input, std, q, null);
	}
	
	/**
	 * This method returns a <code>FrameGrabber</code> associated with this 
	 * video device. Captured frames will be converted to RGB24 before being 
	 * handed out. The video device must support an appropriate image format 
	 * that v4l4j can convert to RGB24. If it does not, this method will throw 
	 * an {@link ImageFormatException}. To check if RGB24 conversion is 
	 * possible, call {@link #supportRGBConversion()}. The returned 
	 * {@link RGBFrameGrabber} must be released when no longer used by calling
	 * {@link #releaseFrameGrabber()}.<br><b>If RGBFrameGrabbers cannot be 
	 * created for your video device, please let the author know about it so 
	 * RGB24-encoding can be added. See the README file on how to submit 
	 * reports.</b>  
	 * @param w the desired frame width. This value may be adjusted to the
	 * closest supported by hardware. 
	 * @param h the desired frame height. This value may be adjusted to the 
	 * closest supported by hardware. 
	 * @param input the input index, as returned by {@link InputInfo#getIndex()}
	 * @param std the video standard, as returned by 
	 * {@link InputInfo#getSupportedStandards()} (see {@link V4L4JConstants})
	 * @param imf the {@link ImageFormat} the frames should be captured in 
	 * before being converted to RGB24. This image format must be one that v4l4j
	 * can convert to RGB24, ie it must be in the list returned by this video 
	 * device's {@link ImageFormatList#getRGBEncodableFormats()}. You can get 
	 * this video device's {@link ImageFormatList} by calling 
	 * <code>getDeviceInfo().getFormatList()</code>. If this argument is 
	 * <code>null</code>, v4l4j will pick the first image format it can 
	 * RGB24-encode. 
	 * @return a {@link RGBFrameGrabber} associated with this video device, if 
	 * supported.
	 * @throws VideoStandardException if the chosen video standard is not 
	 * supported
	 * @throws ImageFormatException if the chosen image format can not be 
	 * RGB24-encoded. If no image format was specified (<code>null</code>), then 
	 * this video device does not have an image format that can be RGB24-encoded.
	 * <b>If you encounter such device, please let the author know so support 
	 * for it can be added. See the README file on how to submit reports.</b> 
	 * @throws CaptureChannelException if the given channel number value is not 
	 * valid
	 * @throws ImageDimensionException if the given image dimensions are not 
	 * supported
	 * @throws InitialisationException if the video device file can not be 
	 * initialised 
	 * @throws V4L4JException if there is an error applying capture parameters
	 * @throws StateException if a {@link FrameGrabber} already exists and must 
	 * be released  before another FrameGrabber can be allocated, or if the 
	 * <code>VideoDevice</code> has been released.
	 */
	public RGBFrameGrabber getRGBFrameGrabber(int w, int h, int input, int std,
			ImageFormat imf) throws V4L4JException{
		
		if(!supportRGB24 || deviceInfo==null)
			throw new ImageFormatException("This video device does not support "
					+"RGB-encoding of its frames.");
		
		if(imf!=null){
			if(!deviceInfo.getFormatList().
					getRGBEncodableFormats().contains(imf))
				throw new ImageFormatException(
						"The image format "+imf.getName()+
						" cannot be converted to RGB24");
		} else
			//if imf is null, pick the first format that can be rgb encoded
			//the list returned by getRGBEncodableFormats() is sorted by best 
			//format first, and if we re here, we know there is at least one 
			//format in there
			imf = deviceInfo.getFormatList().getRGBEncodableFormats().get(0);
	
		
		synchronized(this){
			if(fg==null) {
				state.get();
				fg = new RGBFrameGrabber(v4l4jObject, w, h, input, std,
						findTuner(input), imf);
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
				return (RGBFrameGrabber) fg;
			} else {
				if(fg.getClass().isInstance(RGBFrameGrabber.class))
					return (RGBFrameGrabber) fg;
				else {
					state.put();
					throw new StateException("A FrameGrabber object already "
							+"exists");
				}
			}
		}
	}
	
	/**
	 * This method returns a <code>FrameGrabber</code> associated with this 
	 * video device. Captured frames will be RGB24-encoded before being handed 
	 * out. The video device must support an appropriate image format that v4l4j
	 * can convert to RGB24. If it does not, this method will throw an 
	 * {@link ImageFormatException}. To check if RGB24-encoding is possible, 
	 * call {@link #supportRGBConversion()}. Among all the image formats the 
	 * video device supports, v4l4j will choose the first one that can be RGB24 
	 * encoded. If you prefer to specify which image format is to be used, call
	 * {@link #getRGBFrameGrabber(int, int, int, int, ImageFormat)} instead. 
	 * This is sometimes required because some video device have a lower frame 
	 * rate with some image formats, and a higher one with others. So far, 
	 * testing is the only way to find out. The returned {@link RGBFrameGrabber}
	 *  must be released when no longer used by calling
	 *  {@link #releaseFrameGrabber()}.<br><b>If RGBFrameGrabbers cannot be 
	 * created for your video device, please let the author know about it so 
	 * RGB24-encoding can be added. See the README file on how to submit 
	 * reports.</b>  
	 * @param w the desired frame width. This value may be adjusted to the 
	 * closest supported by hardware. 
	 * @param h the desired frame height. This value may be adjusted to the 
	 * closest supported by hardware. 
	 * @param input the input index, as returned by {@link InputInfo#getIndex()}
	 * @param std the video standard, as returned by 
	 * {@link InputInfo#getSupportedStandards()} (see {@link V4L4JConstants}).
	 * @return a {@link RGBFrameGrabber} associated with this video device, if 
	 * supported.
	 * @throws VideoStandardException if the chosen video standard is not 
	 * supported
	 * @throws ImageFormatException if this video device does not have an image 
	 * format that can be RGB24-encoded. <b>If you encounter such device, please
	 * let the author know so support for it can be added. See the README file 
	 * on how to submit reports.</b>
	 * @throws CaptureChannelException if the given channel number value is not 
	 * valid
	 * @throws ImageDimensionException if the given image dimensions are not 
	 * supported
	 * @throws InitialisationException if the video device file can not be 
	 * initialised 
	 * @throws V4L4JException if there is an error applying capture parameters
	 * @throws StateException if a {@link FrameGrabber} already exists and must
	 * be released before a RGBFrameGrabber can be allocated, or if the 
	 * <code>VideoDevice</code> has been released.  
	 */
	public RGBFrameGrabber getRGBFrameGrabber(int w, int h, int input, int std)
			throws V4L4JException{
		return getRGBFrameGrabber(w, h, input, std, null);
	}
	
	/**
	 * This method returns a <code>FrameGrabber</code> associated with this 
	 * video device. Captured frames will be converted to BGR24 before being 
	 * handed out. The video device must support an appropriate image format 
	 * that v4l4j can convert to BGR24. If it does not, this method will throw 
	 * an {@link ImageFormatException}. To check if BGR24 conversion is 
	 * possible, call {@link #supportBGRConversion()}. The returned 
	 * {@link BGRFrameGrabber} must be released when no longer used by calling
	 * {@link #releaseFrameGrabber()}.<br><b>If BGRFrameGrabbers cannot be 
	 * created for your video device, please let the author know about it so 
	 * BGR24-encoding can be added. See the README file on how to submit 
	 * reports.</b>  
	 * @param w the desired frame width. This value may be adjusted to the
	 * closest supported by hardware. 
	 * @param h the desired frame height. This value may be adjusted to the 
	 * closest supported by hardware. 
	 * @param input the input index, as returned by {@link InputInfo#getIndex()}
	 * @param std the video standard, as returned by 
	 * {@link InputInfo#getSupportedStandards()} (see {@link V4L4JConstants})
	 * @param imf the {@link ImageFormat} the frames should be captured in 
	 * before being converted to BGR24. This image format must be one that v4l4j
	 * can convert to BGR24, ie it must be in the list returned by this video 
	 * device's {@link ImageFormatList#getBGREncodableFormats()}. You can get 
	 * this video device's {@link ImageFormatList} by calling 
	 * <code>getDeviceInfo().getFormatList()</code>. If this argument is 
	 * <code>null</code>, v4l4j will pick the first image format it can 
	 * BGR24-encode. 
	 * @return a {@link BGRFrameGrabber} associated with this video device, if 
	 * supported.
	 * @throws VideoStandardException if the chosen video standard is not 
	 * supported
	 * @throws ImageFormatException if the chosen image format can not be 
	 * BGR24-encoded. If no image format was specified (<code>null</code>), then 
	 * this video device does not have an image format that can be BGR24-encoded.
	 * <b>If you encounter such device, please let the author know so support 
	 * for it can be added. See the README file on how to submit reports.</b> 
	 * @throws CaptureChannelException if the given channel number value is not 
	 * valid
	 * @throws ImageDimensionException if the given image dimensions are not 
	 * supported
	 * @throws InitialisationException if the video device file can not be 
	 * initialised 
	 * @throws V4L4JException if there is an error applying capture parameters
	 * @throws StateException if a {@link FrameGrabber} already exists and must 
	 * be released  before another FrameGrabber can be allocated, or if the 
	 * <code>VideoDevice</code> has been released.
	 */
	public BGRFrameGrabber getBGRFrameGrabber(int w, int h, int input, int std, 
			ImageFormat imf) throws V4L4JException{
		if(!supportBGR24 || deviceInfo==null)
			throw new ImageFormatException("This video device does not support "
					+"BGR-encoding of its frames.");
		
		if(imf!=null){
				if(!deviceInfo.getFormatList().
				getBGREncodableFormats().contains(imf))
			throw new ImageFormatException("The image format "+imf.getName()+
					" cannot be converted to BGR24");
		} else
			//if imf is null, pick the first format that can be rgb encoded
			//the list returned by getBGREncodableFormats() is sorted by best 
			//format first, and if we re here, we know there is at least one 
			//format in there
			imf = deviceInfo.getFormatList().getBGREncodableFormats().get(0);
		
		synchronized(this){
			if(fg==null) {
				state.get();
				fg = new BGRFrameGrabber(v4l4jObject, w, h, input, std,
						findTuner(input), imf);
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
				return (BGRFrameGrabber) fg;
			} else {
				if(fg.getClass().isInstance(BGRFrameGrabber.class))
					return (BGRFrameGrabber) fg;
				else {
					state.put();
					throw new StateException("A FrameGrabber object already "+
							"exists");
				}
			}
		}
	}
	
	/**
	 * This method returns a <code>FrameGrabber</code> associated with this 
	 * video device. Captured frames will be BGR24-encoded before being handed 
	 * out. The video device must support an appropriate image format that v4l4j
	 * can convert to BGR24. If it does not, this method will throw an 
	 * {@link ImageFormatException}. To check if BGR24-encoding is possible, 
	 * call {@link #supportBGRConversion()}. Among all the image formats the 
	 * video device supports, v4l4j will choose the first one that can be BGR24 
	 * encoded. If you prefer to specify which image format is to be used, call
	 * {@link #getBGRFrameGrabber(int, int, int, int, ImageFormat)} instead. 
	 * This is sometimes required because some video device have a lower frame 
	 * rate with some image formats, and a higher one with others. So far, 
	 * testing is the only way to find out. The returned {@link BGRFrameGrabber}
	 *  must be released when no longer used by calling
	 *  {@link #releaseFrameGrabber()}.<br><b>If BGRFrameGrabbers cannot be 
	 * created for your video device, please let the author know about it so 
	 * BGR24-encoding can be added. See the README file on how to submit 
	 * reports.</b>  
	 * @param w the desired frame width. This value may be adjusted to the 
	 * closest supported by hardware. 
	 * @param h the desired frame height. This value may be adjusted to the 
	 * closest supported by hardware. 
	 * @param input the input index, as returned by {@link InputInfo#getIndex()}
	 * @param std the video standard, as returned by 
	 * {@link InputInfo#getSupportedStandards()} (see {@link V4L4JConstants}).
	 * @return a {@link BGRFrameGrabber} associated with this video device, if 
	 * supported.
	 * @throws VideoStandardException if the chosen video standard is not 
	 * supported
	 * @throws ImageFormatException if this video device does not have an image 
	 * format that can be BGR24-encoded. <b>If you encounter such device, please
	 * let the author know so support for it can be added. See the README file 
	 * on how to submit reports.</b>
	 * @throws CaptureChannelException if the given channel number value is not 
	 * valid
	 * @throws ImageDimensionException if the given image dimensions are not 
	 * supported
	 * @throws InitialisationException if the video device file can not be 
	 * initialised 
	 * @throws V4L4JException if there is an error applying capture parameters
	 * @throws StateException if a {@link FrameGrabber} already exists and must
	 * be released before a BGRFrameGrabber can be allocated, or if the 
	 * <code>VideoDevice</code> has been released.  
	 */
	public BGRFrameGrabber getBGRFrameGrabber(int w, int h, int input, int std) 
		throws V4L4JException{
		return getBGRFrameGrabber(w, h, input, std, null);
	}
	
	/**
	 * This method returns a <code>FrameGrabber</code> associated with this 
	 * video device. Captured frames will be converted to YUV420 before being 
	 * handed out. The video device must support an appropriate image format 
	 * that v4l4j can convert to YUV420. If it does not, this method will throw 
	 * an {@link ImageFormatException}. To check if YUV420 conversion is 
	 * possible, call {@link #supportYUVConversion()}. The returned 
	 * {@link YUVFrameGrabber} must be released when no longer used by calling
	 * {@link #releaseFrameGrabber()}.<br><b>If YUVFrameGrabbers cannot be 
	 * created for your video device, please let the author know about it so 
	 * YUV420-encoding can be added. See the README file on how to submit 
	 * reports.</b>  
	 * @param w the desired frame width. This value may be adjusted to the
	 * closest supported by hardware. 
	 * @param h the desired frame height. This value may be adjusted to the 
	 * closest supported by hardware. 
	 * @param input the input index, as returned by {@link InputInfo#getIndex()}
	 * @param std the video standard, as returned by 
	 * {@link InputInfo#getSupportedStandards()} (see {@link V4L4JConstants})
	 * @param imf the {@link ImageFormat} the frames should be captured in 
	 * before being converted to YUV420. This image format must be one that 
	 * v4l4j can convert to YUV420, ie it must be in the list returned by this 
	 * video device's {@link ImageFormatList#getYUVEncodableFormats()}. You can 
	 * get this video device's {@link ImageFormatList} by calling 
	 * <code>getDeviceInfo().getFormatList()</code>. If this argument is 
	 * <code>null</code>, v4l4j will pick the first image format it can 
	 * YUV420-encode. 
	 * @return a {@link YUVFrameGrabber} associated with this video device, if 
	 * supported.
	 * @throws VideoStandardException if the chosen video standard is not 
	 * supported
	 * @throws ImageFormatException if the chosen image format can not be 
	 * YUV420-encoded. If no image format was specified (<code>null</code>), 
	 * then this video device does not have an image format that can be 
	 * YUV420-encoded. <b>If you encounter such device, please let the author 
	 * know so support for it can be added. See the README file on how to submit
	 * reports.</b> 
	 * @throws CaptureChannelException if the given channel number value is not 
	 * valid
	 * @throws ImageDimensionException if the given image dimensions are not 
	 * supported
	 * @throws InitialisationException if the video device file can not be 
	 * initialised 
	 * @throws V4L4JException if there is an error applying capture parameters
	 * @throws StateException if a {@link FrameGrabber} already exists and must 
	 * be released  before another FrameGrabber can be allocated, or if the 
	 * <code>VideoDevice</code> has been released.
	 */
	public YUVFrameGrabber getYUVFrameGrabber(int w, int h, int input, int std, 
			ImageFormat imf) throws V4L4JException{
		if(!supportYUV420 || deviceInfo==null)
			throw new ImageFormatException("This video device does not support "
					+"YUV-encoding of its frames.");
		
		if(imf!=null){ 
				if(!deviceInfo.getFormatList().
						getYUVEncodableFormats().contains(imf))
					throw new ImageFormatException(
							"The image format "+imf.getName()+
							" cannot be converted to YUV420");
		} else 
			//if imf is null, pick the first format that can be rgb encoded
			//the list returned by getYUVEncodableFormats() is sorted by best 
			//format first, and if we re here, we know there is at least one 
			//format in there
			imf = deviceInfo.getFormatList().getYUVEncodableFormats().get(0);
		
		synchronized(this){
			if(fg==null) {
				state.get();
				fg = new YUVFrameGrabber(v4l4jObject, w, h, input, std,
						findTuner(input), imf);
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
				return (YUVFrameGrabber) fg;
			} else {
				if(fg.getClass().isInstance(YUVFrameGrabber.class))
					return (YUVFrameGrabber) fg;
				else {
					state.put();
					throw new StateException("A FrameGrabber object already "+
							"exists");
				}
			}
		}
	}
	
	/**
	 * This method returns a <code>FrameGrabber</code> associated with this 
	 * video device. Captured frames will be YUV420-encoded before being handed 
	 * out. The video device must support an appropriate image format that v4l4j
	 * can convert to YUV420. If it does not, this method will throw an 
	 * {@link ImageFormatException}. To check if YUV420-encoding is possible, 
	 * call {@link #supportYUVConversion()}. Among all the image formats the 
	 * video device supports, v4l4j will choose the first one that can be YUV420 
	 * encoded. If you prefer to specify which image format is to be used, call
	 * {@link #getYUVFrameGrabber(int, int, int, int, ImageFormat)} instead. 
	 * This is sometimes required because some video device have a lower frame 
	 * rate with some image formats, and a higher one with others. So far, 
	 * testing is the only way to find out. The returned {@link YUVFrameGrabber}
	 *  must be released when no longer used by calling
	 *  {@link #releaseFrameGrabber()}.<br><b>If YUVFrameGrabbers cannot be 
	 * created for your video device, please let the author know about it so 
	 * YUV420-encoding can be added. See the README file on how to submit 
	 * reports.</b>  
	 * @param w the desired frame width. This value may be adjusted to the 
	 * closest supported by hardware. 
	 * @param h the desired frame height. This value may be adjusted to the 
	 * closest supported by hardware. 
	 * @param input the input index, as returned by {@link InputInfo#getIndex()}
	 * @param std the video standard, as returned by 
	 * {@link InputInfo#getSupportedStandards()} (see {@link V4L4JConstants}).
	 * @return a {@link YUVFrameGrabber} associated with this video device, if 
	 * supported.
	 * @throws VideoStandardException if the chosen video standard is not 
	 * supported
	 * @throws ImageFormatException if this video device does not have an image 
	 * format that can be YUV420-encoded. <b>If you encounter such device, please
	 * let the author know so support for it can be added. See the README file 
	 * on how to submit reports.</b>
	 * @throws CaptureChannelException if the given channel number value is not 
	 * valid
	 * @throws ImageDimensionException if the given image dimensions are not 
	 * supported
	 * @throws InitialisationException if the video device file can not be 
	 * initialised 
	 * @throws V4L4JException if there is an error applying capture parameters
	 * @throws StateException if a {@link FrameGrabber} already exists and must
	 * be released before a YUVFrameGrabber can be allocated, or if the 
	 * <code>VideoDevice</code> has been released.  
	 */
	public YUVFrameGrabber getYUVFrameGrabber(int w, int h, int input, int std) 
		throws V4L4JException{
		return getYUVFrameGrabber(w, h, input, std, null);
	}
	
	/**
	 * This method returns a <code>FrameGrabber</code> associated with this 
	 * video device. Captured frames will be converted to YVU420 before being 
	 * handed out. The video device must support an appropriate image format 
	 * that v4l4j can convert to YVU420. If it does not, this method will throw 
	 * an {@link ImageFormatException}. To check if YVU420 conversion is 
	 * possible, call {@link #supportYVUConversion()}. The returned 
	 * {@link YVUFrameGrabber} must be released when no longer used by calling
	 * {@link #releaseFrameGrabber()}.<br><b>If YVUFrameGrabbers cannot be 
	 * created for your video device, please let the author know about it so 
	 * YVU420-encoding can be added. See the README file on how to submit 
	 * reports.</b>  
	 * @param w the desired frame width. This value may be adjusted to the
	 * closest supported by hardware. 
	 * @param h the desired frame height. This value may be adjusted to the 
	 * closest supported by hardware. 
	 * @param input the input index, as returned by {@link InputInfo#getIndex()}
	 * @param std the video standard, as returned by 
	 * {@link InputInfo#getSupportedStandards()} (see {@link V4L4JConstants})
	 * @param imf the {@link ImageFormat} the frames should be captured in 
	 * before being converted to YVU420. This image format must be one that 
	 * v4l4j can convert to YVU420, ie it must be in the list returned by this 
	 * video device's {@link ImageFormatList#getYVUEncodableFormats()}. You can 
	 * get this video device's {@link ImageFormatList} by calling 
	 * <code>getDeviceInfo().getFormatList()</code>. If this argument is 
	 * <code>null</code>, v4l4j will pick the first image format it can 
	 * YVU420-encode. 
	 * @return a {@link YVUFrameGrabber} associated with this video device, if 
	 * supported.
	 * @throws VideoStandardException if the chosen video standard is not 
	 * supported
	 * @throws ImageFormatException if the chosen image format can not be 
	 * YVU420-encoded. If no image format was specified (<code>null</code>), 
	 * then this video device does not have an image format that can be 
	 * YVU420-encoded. <b>If you encounter such device, please let the author 
	 * know so support for it can be added. See the README file on how to submit
	 * reports.</b> 
	 * @throws CaptureChannelException if the given channel number value is not 
	 * valid
	 * @throws ImageDimensionException if the given image dimensions are not 
	 * supported
	 * @throws InitialisationException if the video device file can not be 
	 * initialised 
	 * @throws V4L4JException if there is an error applying capture parameters
	 * @throws StateException if a {@link FrameGrabber} already exists and must 
	 * be released  before another FrameGrabber can be allocated, or if the 
	 * <code>VideoDevice</code> has been released.
	 */
	public YVUFrameGrabber getYVUFrameGrabber(int w, int h, int input, int std, 
			ImageFormat imf) throws V4L4JException{
		if(!supportYVU420 || deviceInfo==null)
			throw new ImageFormatException("This video device does not support "
					+"YVU-encoding of its frames.");
		
		if(imf!=null){			
			if(!deviceInfo.getFormatList().
					getYVUEncodableFormats().contains(imf))
				throw new ImageFormatException("The image format "
						+imf.getName()+" cannot be converted to YVU420");
		} else {		
			//if imf is null, pick the first format that can be rgb encoded
			//the list returned by getYVUEncodableFormats() is sorted by best 
			//format first, and if we re here, we know there is at least one 
			//format in there
			imf = deviceInfo.getFormatList().getYVUEncodableFormats().get(0);
		}
		
		synchronized(this){
			if(fg==null) {
				state.get();
				fg = new YVUFrameGrabber(v4l4jObject, w, h, input, std,
						findTuner(input), imf);
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
				return (YVUFrameGrabber) fg;
			} else {
				if(fg.getClass().isInstance(YUVFrameGrabber.class))
					return (YVUFrameGrabber) fg;
				else {
					state.put();
					throw new StateException("A FrameGrabber object already "+
							"exists");
				}
			}
		}
	}
	
	/**
	 * This method returns a <code>FrameGrabber</code> associated with this 
	 * video device. Captured frames will be YVU420-encoded before being handed 
	 * out. The video device must support an appropriate image format that v4l4j
	 * can convert to YVU420. If it does not, this method will throw an 
	 * {@link ImageFormatException}. To check if YVU420-encoding is possible, 
	 * call {@link #supportYVUConversion()}. Among all the image formats the 
	 * video device supports, v4l4j will choose the first one that can be YVU420 
	 * encoded. If you prefer to specify which image format is to be used, call
	 * {@link #getYVUFrameGrabber(int, int, int, int, ImageFormat)} instead. 
	 * This is sometimes required because some video device have a lower frame 
	 * rate with some image formats, and a higher one with others. So far, 
	 * testing is the only way to find out. The returned {@link YVUFrameGrabber}
	 *  must be released when no longer used by calling
	 *  {@link #releaseFrameGrabber()}.<br><b>If YUVFrameGrabbers cannot be 
	 * created for your video device, please let the author know about it so 
	 * YVU420-encoding can be added. See the README file on how to submit 
	 * reports.</b>  
	 * @param w the desired frame width. This value may be adjusted to the 
	 * closest supported by hardware. 
	 * @param h the desired frame height. This value may be adjusted to the 
	 * closest supported by hardware. 
	 * @param input the input index, as returned by {@link InputInfo#getIndex()}
	 * @param std the video standard, as returned by 
	 * {@link InputInfo#getSupportedStandards()} (see {@link V4L4JConstants}).
	 * @return a {@link YVUFrameGrabber} associated with this video device, if 
	 * supported.
	 * @throws VideoStandardException if the chosen video standard is not 
	 * supported
	 * @throws ImageFormatException if this video device does not have an image 
	 * format that can be YVU420-encoded. <b>If you encounter such device, please
	 * let the author know so support for it can be added. See the README file 
	 * on how to submit reports.</b>
	 * @throws CaptureChannelException if the given channel number value is not 
	 * valid
	 * @throws ImageDimensionException if the given image dimensions are not 
	 * supported
	 * @throws InitialisationException if the video device file can not be 
	 * initialised 
	 * @throws V4L4JException if there is an error applying capture parameters
	 * @throws StateException if a {@link FrameGrabber} already exists and must
	 * be released before a YVUFrameGrabber can be allocated, or if the 
	 * <code>VideoDevice</code> has been released.  
	 */
	public YVUFrameGrabber getYVUFrameGrabber(int w, int h, int input, int std) 
		throws V4L4JException{
		return getYVUFrameGrabber(w, h, input, std, null);
	}
	
	/**
	 * This method returns a <code>FrameGrabber</code> associated with this 
	 * video device. Captured frames will be handed out in the same format as 
	 * received from the driver and can be chosen amongst the ones supported by
	 * this video device. To enumerate the supported {@link ImageFormat}s, check
	 * the {@link ImageFormatList} returned by
	 * {@link DeviceInfo#getFormatList()}.
	 * The returned {@link FrameGrabber} must be released when no longer used by
	 * calling {@link #releaseFrameGrabber()}.
	 * @param w the desired frame width. This value may be adjusted to the 
	 * closest supported by hardware. 
	 * @param h the desired frame height. This value may be adjusted to the 
	 * closest supported by hardware. 
	 * @param input the input index, as returned by 
	 * {@link InputInfo#getIndex()}.
	 * @param std the video standard, as returned by 
	 * {@link InputInfo#getSupportedStandards()} (see {@link V4L4JConstants})
	 * @param format the desired image format. A list of supported 
	 * {@link ImageFormat}s can be obtained by calling 
	 * <code>getDeviceInfo().getFormatList()</code>. If this argument is 
	 * <code>null</code>, an {@link ImageFormatException} is thrown. 
	 * @return the <code>FrameGrabber</code> associated with this video device
	 * @throws VideoStandardException if the chosen video standard is not 
	 * supported
	 * @throws ImageFormatException If the image format argument is 
	 * <code>null</code>
	 * @throws CaptureChannelException if the given channel number value is not 
	 * valid
	 * @throws ImageDimensionsException if the given image dimensions are not 
	 * supported
	 * @throws InitialisationException if the video device file can not be 
	 * initialised 
	 * @throws V4L4JException if there is an error applying capture parameters
	 * @throws StateException if a <code>FrameGrabber</code> already exists or
	 * if the <code>VideoDevice</code> has been released.
	 */
	public FrameGrabber getRawFrameGrabber(int w, int h, int input, int std,
			ImageFormat format) throws V4L4JException{
		if(format==null)
			throw new ImageFormatException("The image format can not be null");
		
		synchronized(this){
			if(fg==null) {
				state.get();
				fg = new FrameGrabber(v4l4jObject, w, h, input, std, 
						findTuner(input), format);
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
					throw new StateException("A JPEGFrameGrabber object already "
							+"exists");
				}
			}
		}
	}
	
	/**
	 * This method returns a <code>FrameGrabber</code> associated with this 
	 * video device. Captured frames will be handed out in the same format as 
	 * received from the driver. The chosen format is the one returned by
	 * <code>getDeviceInfo().getFormatList().getList.get(0)</code>. The 
	 * {@link FrameGrabber} must be released when no longer used by calling
	 * {@link #releaseFrameGrabber()}.
	 * @param w the desired frame width. This value may be adjusted to the 
	 * closest supported by hardware. 
	 * @param h the desired frame height. This value may be adjusted to the 
	 * closest supported by hardware. 
	 * @param input the input index, as returned by 
	 * {@link InputInfo#getIndex()}.
	 * @param std the video standard, as returned by 
	 * {@link InputInfo#getSupportedStandards()} (see {@link V4L4JConstants})
	 * @return the <code>FrameGrabber</code> associated with this video device.
	 * @throws VideoStandardException if the chosen video standard is not 
	 * supported.
	 * @throws CaptureChannelException if the given channel number value is not 
	 * valid.
	 * @throws ImageDimensionException if the given image dimensions are not 
	 * supported.
	 * @throws InitialisationException if the video device file can not be 
	 * initialised.
	 * @throws ImageFormatException if no image format could be found because
	 * the {@link DeviceInfo} object could not be obtained.
	 * @throws V4L4JException if there is an error applying capture parameters
	 * @throws StateException if a <code>FrameGrabber</code> already exists or 
	 * if the <code>VideoDevice</code> has been released.
	 */
	public FrameGrabber getRawFrameGrabber(int w, int h, int input, int std) 
			throws V4L4JException{
		if(deviceInfo==null)
			throw new ImageFormatException("No DeviceInfo could be obtained & "
					+"no image format could be found");
		return getRawFrameGrabber(w, h, input, std, 
				deviceInfo.getFormatList().getNativeFormats().get(0));
	}
	
	/**
	 * This method releases the <code>FrameGrabber</code> object allocated 
	 * previously with <code>getRGBFrameGrabber()</code>, 
	 * <code>getBGRFrameGrabber()</code>, <code>getYUVFrameGrabber()</code>,
	 * <code>getYVUFrameGrabber()</code>, <code>getJPEGFrameGrabber()</code> or
	 * <code>getRawFrameGrabber()</code>.
	 * This method must be called when the <code>FrameGrabber</code> object is 
	 * no longer used, so low-level resources can be freed. This method does 
	 * nothing if no <code>FrameGrabber</code> object has been allocated in the 
	 * first place.
	 */
	public void releaseFrameGrabber() {
		synchronized(this){
			if(fg!=null){
				try {fg.release();}
				catch (Throwable t){
					t.printStackTrace();
					throw new StateException("Cant release resources used by "
							+"framegrabber", t);
				}
				fg = null;
				state.put();

			}
		}	
	}

	/**
	 * This method returns the {@link TunerList} of {@link Tuner}s available on
	 * this {@link VideoDevice}. {@link Tuner}s in this list can be used to 
	 * control individual physical tuners on the video device and get signal 
	 * information form them. Check the documentation of the {@link Tuner} class 
	 * for more information. This method (as well as 
	 * {@link #getDeviceInfo()} does not have an equivalent release method. 
	 * In other word, the returned {@link TunerList} object does not need to be
	 * released before releasing the {@link VideoDevice}.
	 * @return a {@link TunerList} 
	 * @throws NoTunerException if this video device does not have any tuners.
	 */
	public TunerList getTunerList() throws NoTunerException{
		if(tuners==null)
			throw new NoTunerException("This video device does not have any "
					+"tuners");
		return tuners;
	}
	
	
	private Tuner findTuner(int input){
		if(deviceInfo!=null) {
			for(InputInfo i: deviceInfo.getInputs())
				if(i.getIndex() == input && i.hasTuner())
					try {
						return tuners.getTuner(i.getTunerInfo().getIndex());
					} catch (NoTunerException e) {
						//weird, shoudlnt be here
					}
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
				throw new StateException("This VideoDevice has not been "
						+"initialised or is about to be removed, and can not be"
						+" used");
		}
		
		public synchronized void put(){
			if(state==INIT) {
				if(--users==0  && temp==RELEASED)
					notify();
			} else
				throw new StateException("This VideoDevice has not been "
						+"initialised and can not be used");
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
		 * @param wait whether we want to wait for potential users or not. If we
		 * do, calls to this method will block until all users are finished. If 
		 * we dont (wait = false), this method will throw a ReleaseException if 
		 * there are users.
		 * @return whether we can switch to the released state or not
		 * @throws ReleaseException if there are still some users and we have 
		 * chosen not to wait
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
						System.err.println("Interrupted while waiting for "
								+"VideoDevice users to complete");
						e.printStackTrace();
						throw new StateException("There are remaining users of "
								+"this VideoDevice and it can not be released");
					}
				return;
			}
			throw new StateException("This VideoDevice has not been initialised"
					+" and can not be used");
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
		for(ImageFormat f : d.getFormatList().getNativeFormats())
			System.out.println("\t"+f.getName()+" - "+f.getIndex());
		
		System.out.print("Formats that can be RGB24-converted: "
				+(vd.supportRGBConversion()?"":"None"));
		if(vd.supportRGBConversion())
			for(ImageFormat f: d.getFormatList().getRGBEncodableFormats())
				System.out.print(f.getName()+" ("+f.getIndex()+") ");
		System.out.println("");
		
		System.out.print("Formats that can be BGR24-converted: "
				+(vd.supportBGRConversion()?"":"None"));
		if(vd.supportBGRConversion())
			for(ImageFormat f: d.getFormatList().getBGREncodableFormats())
				System.out.print(f.getName()+" ("+f.getIndex()+") ");
		System.out.println("");
		
		System.out.print("Formats that can be YUV420-converted: "
				+(vd.supportYUVConversion()?"":"None"));
		if(vd.supportYUVConversion())
			for(ImageFormat f: d.getFormatList().getYUVEncodableFormats())
				System.out.print(f.getName()+" ("+f.getIndex()+") ");
		System.out.println("");
		
		System.out.print("Formats that can be YVU420-converted: "
				+(vd.supportYVUConversion()?"":"None"));
		if(vd.supportYVUConversion())
			for(ImageFormat f: d.getFormatList().getYVUEncodableFormats())
				System.out.print(f.getName()+" ("+f.getIndex()+") ");
		System.out.println("");
		
		System.out.print("Formats that can be JPEG-encoded: "
				+(vd.supportJPEGConversion()?"":"None"));
		if(vd.supportJPEGConversion())
			for(ImageFormat f: d.getFormatList().getJPEGEncodableFormats())
				System.out.print(f.getName()+" ("+f.getIndex()+") ");
		System.out.println("");
		
		System.out.println("Inputs:");
		for(InputInfo i: d.getInputs()){
			System.out.println("\tName: "+i.getName());
			System.out.println("\tType: "+i.getType()+"("+(i.getType() == 
				V4L4JConstants.INPUT_TYPE_CAMERA ? "Camera" : "Tuner")+")");
			System.out.println("\tIndex: "+i.getIndex());
			System.out.println("\tSupported standards:");
			for(Integer s: i.getSupportedStandards()){
				System.out.print("\t\t"+s);
				if(s==V4L4JConstants.STANDARD_PAL)
					System.out.println("(PAL)");
				else if(s==V4L4JConstants.STANDARD_NTSC)
					System.out.println("(NTSC)");
				else if(s==V4L4JConstants.STANDARD_SECAM)
					System.out.println("(SECAM)");
				else
					System.out.println("(None/Webcam)");
			}
			if(i.getType() == V4L4JConstants.INPUT_TYPE_TUNER) {
				TunerInfo t = i.getTunerInfo();
				System.out.println("\tTuner");
				System.out.println("\t\tname: "+t.getName());
				System.out.println("\t\tIndex: "+t.getIndex());
				System.out.println("\t\tRange high: "+t.getRangeHigh());
				System.out.println("\t\tRange low: "+t.getRangeLow());
				System.out.println("\t\tUnit: "+t.getUnit()+"("+(t.getUnit() == 
					V4L4JConstants.FREQ_MHZ ? "MHz" : "kHz")+")");
				System.out.println("\t\tType: "+t.getType()+"("+(t.getType() == 
					V4L4JConstants.TUNER_TYPE_RADIO ? "Radio" : "TV")+")");				
			}
		}
		
		List<Control> ctrls = vd.getControlList().getList();
		for(Control c: ctrls){
			System.out.print("Control: "+c.getName()+" - min: "+c.getMinValue()+
					" - max: "+c.getMaxValue()+" - step: "+c.getStepValue()+
					" - value: ");
			try {
				System.out.println(c.getValue());
			} catch (V4L4JException ve){
				//ve.printStackTrace();
				System.out.println(" ERROR");
			}
		}
		vd.releaseControlList();
		vd.release();
	}
}
