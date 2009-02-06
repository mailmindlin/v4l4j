package au.edu.jcu.v4l4j;

import java.io.File;
import java.util.Hashtable;

import au.edu.jcu.v4l4j.exceptions.CaptureChannelException;
import au.edu.jcu.v4l4j.exceptions.ImageFormatException;
import au.edu.jcu.v4l4j.exceptions.InUseException;
import au.edu.jcu.v4l4j.exceptions.InitialistationException;
import au.edu.jcu.v4l4j.exceptions.JNIException;
import au.edu.jcu.v4l4j.exceptions.StateException;
import au.edu.jcu.v4l4j.exceptions.V4L4JException;
import au.edu.jcu.v4l4j.exceptions.VideoStandardException;

/**
 * Objects of this class represent a V4L video device. To create a <code>VideoDevice</code>,
 * you only need to pass the full path to its device file, and then call the <code>init()</code> method.
 * TODO: COMPLETE ME
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
	 * @throws V4L4JException if there is an error releasing the resources.
	 */
	private native void doRelease(long o) throws V4L4JException;
	
	/**
	 * This JNI method releases resources used by libv4l's struct video_device,
	 * as allocated by <code>doInit()</code>
	 * @param o A C pointer to a struct vl4j_device
	 * @param fmts an array of image formats supported by this <code>VideoDevice</code>
	 * @return 1 if JPEG-encoding is supported, 0 otherwise
	 * @throws JNIException if there is an error in the JNI code
	 */
	private native int doCheckJPEGSupport(long o, int[] fmts) throws JNIException;
	
	/**
	 * This JNI method initialises the control interface and 
	 * returns an array of <code>Control</code>s.
	 * @param o A C pointer to a struct v4l4j_device
	 * @return an array of <code>Control</code>s.
	 * @throws JNIException if there is an error in the JNI code
	 */
	private native Control[] doGetControlList(long o) throws JNIException;

	/**
	 * This JNI method releases the control interface
	 * @param o A C pointer to a struct v4l4j_device
	 */
	private native void doReleaseControlList(long o);
	
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
	private Hashtable<String,Control> controls;
	
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
	 * When finished, resources must be released by calling the <code>release()</code> method. 
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
		deviceInfo = new DeviceInfo(v4l4jObject, deviceFile);
		
		int[] fmts = new int[deviceInfo.getFormats().size()];
		int j=0;
		
		for(ImageFormat i: deviceInfo.getFormats())
			fmts[j++] = i.getIndex();
		
		supportJPEG = doCheckJPEGSupport(v4l4jObject, fmts) == 1 ? true:false;
		
		state.commit();
	}
	
	/**
	 * This method release resources used by this VideoDevice. This method WILL <code>wait()</code> if a
	 * <code>FrameGrabber()</code> or/and a <code>ControlList</code> is in use, until 
	 * <code>releaseFrameGrabber()</code> or/and <code>releaseControlList()</code> have been called. 
	 * @throws StateException if a call to <code>release()</code> is already in progress.
	 */
	public void release() throws V4L4JException{
		if(!state.release())
			throw new StateException("A call to this method is already being serviced");
		//TODO: check that capture is not in progress !
		doRelease(v4l4jObject);
		state.commit();
	}	
	
	/**
	 * This method release resources used by this VideoDevice. If argument <code>wait</code> is true,
	 * this method will <code>wait()</code> if a <code>FrameGrabber()</code> or/and a
	 * <code>ControlList</code> are in use, until <code>releaseFrameGrabber()</code> or/and
	 * <code>releaseControlList()</code> have been called. It is in effect identical to 
	 * <code>release(). If argument <code>wait</code> is false,
	 * this method will throw an <code>InUseException</code> if a <code>FrameGrabber()</code> or/and a
	 * <code>ControlList</code> are in use. Otherwise, it will just proceed with the release of 
	 * resources
	 * @param wait whether or not this method should block and wait until <code>releaseFrameGrabber()</code>
	 * or/and <code>releaseControlList()</code> have been called 
	 * @throws StateException if a call to <code>release()</code> is already in progress.
	 * @throws InUseException if there either the <code>releaseFrameGrabber()</code> or the
	 * <code>releaseControlList()</code> have not been called, and we asked to not wait 
	 * (argument <code>wait</code> is false)
	 */
	public void release(boolean wait) throws V4L4JException{
		if(!state.release(wait))
			throw new StateException("A call to this method is already being serviced");
		//TODO: check that capture is not in progress !
		doRelease(v4l4jObject);
		state.commit();
	}	
	
	/**
	 * This method creates a <code>DeviceInfo</code> object which contains information about
	 * this video device.
	 * @return a <code>DeviceInfo</code> object 
	 */
	public DeviceInfo getDeviceInfo(){
		return deviceInfo;		
	}
	
	/**
	 * This method returns a list of <code>Control</code>s associated with this video device.
	 * @return a list of <code>Control</code>s 
	 * @throws StateException if a list already exists or the <code>VideoDevice</code> is being released 
	 */
	public Hashtable<String,Control> getControlList(){
		if(!state.get())
			throw new StateException("this VideoDevice is being released");
		
		synchronized(this){
			if(controls==null) {
				Control[] c = doGetControlList(v4l4jObject);
				controls = new Hashtable<String, Control>();
				for(Control ctrl: c)
					controls.put(ctrl.getName(), ctrl);
				return controls;
			} else {
				state.put();
				throw new StateException("A list of Controls already exists");
			}
		}
	}
	
	/**
	 * This method releases the list of <code>Control</code>s returned by <code>getControlList()</code>.
	 * This method must be called when the list of <code>Control</code>s is no longer used, so low-level 
	 * resources can be freed. This method does nothing if a list of <code>Control</code>s has never been
	 * allocated in the first place.
	 */
	public void releaseControlList() throws StateException{
		synchronized(this){
			if(controls!=null){
				doReleaseControlList(v4l4jObject);
				controls = null;
				state.put();
			} else {
				//throw new V4L4JException("The list of controls was never allocated");
			}	
		}	
	}
	
	/**
	 * This method specifies whether frames captured from this video device can be JPEG-encoded before
	 * being handed out. If this video device can capture frames in a native format that can be encoded
	 * in JPEG, then this method returns true, and calls to <code>getJPEGFrameGrabber()</code> will succeed.
	 * If this method returns false, calls to <code>getJPEGFrameGrabber()</code> will definitely fail, and the only
	 * alternative is to call <code>getRawFrameGrabber()</code>. 
	 * @return
	 */
	public boolean canJPEGEncode(){
		return supportJPEG;
	}
	
	/**
	 * This method returns the <code>FrameGrabber</code> associated with this video device. Captured frames will be JPEG-encoded
	 * before being handed out. The video device must have an appropriate image format. If it doesnt, this method will throw 
	 * an <code>ImageFormatException</code>. To check if JPEG-encoding is supported by this <code>VideoDevice</code>, call
	 * <code>canJPEGEncode()</code>.
	 * @param w the desired frame width 
	 * @param h the desired frame height
	 * @param ch the input index, as returned by <code>InputInfo.getIndex()</code>
	 * @param std the video standard, as returned by <code>InputInfo.getSupportedStandards()</code>
	 * (see V4L4JConstants)
	 * @param q the JPEG image quality (the higher, the better the quality), within the range
	 * V4l4JConstants.MIN_JPEG_QUALITY, V4l4JConstants.MAX_JPEG_QUALITY
	 * @return a <code>JPEGFrameGrabber</code> associated with this video device, if supported.
	 * @throws VideoStandardException if the chosen video standard is not supported
	 * @throws ImageFormatException if the video device uses an unsupported image format which can not be JPEG-encoded. If you
	 * encounter such device, please let the author know about it. See README file in v4l4j/ on how to report this issue. 
	 * @throws CaptureChannelException if the given channel number value is not valid
	 * @throws ImageDimensionException if the given image dimensions are not supported
	 * @throws InitialistationException if the video device file cant be initialised 
	 * @throws V4L4JException if there is an error applying capture parameters
	 * @throws StateException if a <code>RawFrameGrabber</code> already exists and must be released before a JPEGFrameGrabber
	 * can be allocated, or the <code>VideoDevice</code> is being released.
	 */
	public JPEGFrameGrabber getJPEGFrameGrabber(int w, int h, int input, int std, int q) throws V4L4JException{
		if(!supportJPEG)
			throw new ImageFormatException("This video device does not support JPEG-encoding of its frames.");
		
		if(!state.get())
			throw new StateException("this VideoDevice is being released");
		
		synchronized(this){
			if(fg==null) {
				fg = new JPEGFrameGrabber(v4l4jObject, w, h, input, std, q);
				try {
					fg.init();
				} catch (V4L4JException ve){
					state.put();
					throw ve;
				}  catch (StateException se){
					state.put();
					throw se;
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
	 * This method returns the <code>FrameGrabber</code> associated with this video device. Captured frames will be handed out in the same
	 * format as received from the driver. The format (<code>ImageFormat</code>) can be chosen amongst the ones supported by, which can be 
	 * enumerated by calling <code>VideoDevice.getDeviceInfo().getFormats()</code>.
	 * @param w the desired frame width 
	 * @param h the desired frame height
	 * @param ch the input index, as returned by <code>InputInfo.getIndex()</code>
	 * @param std the video standard, as returned by <code>InputInfo.getSupportedStandards()</code>
	 * (see V4L4JConstants)
	 * @param format the desired image format. A list of supported <code>ImageFormat</code>s can be obtained by calling
	 * <code>VideoDevice.getDeviceInfo().getFormats()</code>.
	 * @return the <code>FrameGrabber</code> associated with this video device
	 * @throws VideoStandardException if the chosen video standard is not supported
	 * @throws ImageFormatException if the selected video device uses an unsupported image format (let the author know, see README file)
	 * @throws CaptureChannelException if the given channel number value is not valid
	 * @throws ImageDimensionException if the given image dimensions are not supported
	 * @throws InitialistationException if the video device file cant be initialised 
	 * @throws V4L4JException if there is an error applying capture parameters
	 * @throws StateException if a <code>FrameGrabber</code> already exists or the <code>VideoDevice</code> is being released
	 */
	public FrameGrabber getRawFrameGrabber(int w, int h, int input, int std, ImageFormat format) throws V4L4JException{
		if(!state.get())
			throw new StateException("this VideoDevice is being released");
		
		synchronized(this){
			if(fg==null) {
				fg = new FrameGrabber(v4l4jObject, w, h, input, std, format);
				try {
					fg.init();
				} catch (V4L4JException ve){
					state.put();
					throw ve;
				}  catch (StateException se){
					state.put();
					throw se;
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
	 * This method returns the <code>FrameGrabber</code> associated with this video device. Captured frames will be handed out in the same
	 * format as received from the driver. The chosen format is the first one in the list returned by
	 * <code>VideoDevice.getDeviceInfo().getFormats()</code>.
	 * @param w the desired frame width 
	 * @param h the desired frame height
	 * @param ch the input index, as returned by <code>InputInfo.getIndex()</code>
	 * @param std the video standard, as returned by <code>InputInfo.getSupportedStandards()</code>
	 * (see V4L4JConstants)
	 * @return the <code>FrameGrabber</code> associated with this video device
	 * @throws VideoStandardException if the chosen video standard is not supported
	 * @throws ImageFormatException if the selected video device uses an unsupported image format (let the author know, see README file)
	 * @throws CaptureChannelException if the given channel number value is not valid
	 * @throws ImageDimensionException if the given image dimensions are not supported
	 * @throws InitialistationException if the video device file cant be initialised 
	 * @throws V4L4JException if there is an error applying capture parameters
	 * @throws StateException if a <code>FrameGrabber</code> already exists or the <code>VideoDevice</code> is being released.
	 */
	public FrameGrabber getRawFrameGrabber(int w, int h, int input, int std) throws V4L4JException{
		return getRawFrameGrabber(w, h, input, std, deviceInfo.getFormats().get(0));
	}
	
	/**
	 * This method releases the <code>FrameGrabber</code> object returned by <code>getFrameGrabber()</code>.
	 * This method must be called when the <code>FrameGrabber</code> object is no longer used, so low-level 
	 * resources can be freed. This method does nothing if a <code>FrameGrabber</code> object has never been
	 * allocated in the first place.
	 * @throws StateException if the <code>VideoDevice</code> has not been initialised
	 */
	public void releaseFrameGrabber() throws StateException{
		synchronized(this){
			if(fg!=null){
				try {fg.release();}
				catch (Exception e){
					e.printStackTrace();
					throw new StateException("Cant release resources used by framegrabber", e);
				}
				fg = null;
				state.put();

			} else {
				//throw new V4L4JException("The FrameGrabber was never allocated");
			}	
		}	
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

		public synchronized boolean get(){
			if(state==INIT && temp!=RELEASED) {
				users++;
				return true;
			} else
				return false;
		}
		
		public synchronized boolean put(){
			if(state==INIT) {
				if(--users==0  && temp==RELEASED)
					notify();
				return true;
			} else
				return false;
		}
		
		/**
		 * This method switched to the released state. This method will wait
		 * until all users have finished.
		 * @return whether we can switch to the released state or not
		 */
		public synchronized boolean release(){
			return release(true);
		}
		
		/**
		 * This method switched to the released state
		 * @param wait whether we want to wait for potential users or not. If we do, calls to this method
		 * will block until all users are finished. If we dont (wait = false), this method will throw a
		 * V4L4JException if there are users
		 * @return whether we can switch to the released state or not
		 * @throws InUseException if there are still some users and we have chosen not to wait
		 */
		public synchronized boolean release(boolean wait) throws InUseException{
			int t = temp;
			if(state==INIT && temp!=RELEASED) {
				temp=RELEASED;
				while(users!=0)
					try {
						if(wait==true)
							wait();
						else {
							temp = t;
							throw new InUseException("Still in use");	
						}
					} catch (InterruptedException e) {
						temp = t;
						System.err.println("Interrupted while waiting for VideoDevice users to complete");
						e.printStackTrace();
						return false;
					}
				return true;
			}
			return false;
		}
		
		public synchronized void commit(){
			state=temp;
		}
	}

	public static void main(String args[]) throws V4L4JException{
		VideoDevice vd = new VideoDevice(args[0]);
		vd.init();
		DeviceInfo d = vd.getDeviceInfo(); 
		System.out.println("name: "+d.getName());
		System.out.println("Device file: "+d.getDeviceFile());
		System.out.println("Supported formats:");
		
		for(ImageFormat f : d.getFormats())
			System.out.println("\t"+f.getName()+" - "+f.getIndex());
		
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
				System.out.println("\t\tUnit: "+t.getUnit()+"("+(t.getUnit() == TunerInfo.MHZ ? "MHz" : "kHz")+")");
				System.out.println("\t\tType: "+t.getType()+"("+(t.getType() == TunerInfo.RADIO ? "Radio" : "TV")+")");				
			}
		}
		
		Hashtable<String,Control> ctrls = vd.getControlList();
		for(Control c: ctrls.values()){
			System.out.print("Control: "+c.getName()+" - min: "+c.getMin()+" - max: "+c.getMax()+" - step: "+c.getStep());
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
