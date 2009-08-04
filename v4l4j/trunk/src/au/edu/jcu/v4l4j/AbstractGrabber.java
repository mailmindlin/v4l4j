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
import au.edu.jcu.v4l4j.exceptions.CaptureChannelException;
import au.edu.jcu.v4l4j.exceptions.ImageFormatException;
import au.edu.jcu.v4l4j.exceptions.InitialisationException;
import au.edu.jcu.v4l4j.exceptions.InvalidValue;
import au.edu.jcu.v4l4j.exceptions.NoTunerException;
import au.edu.jcu.v4l4j.exceptions.StateException;
import au.edu.jcu.v4l4j.exceptions.V4L4JException;
import au.edu.jcu.v4l4j.exceptions.VideoStandardException;


/**
 * This abstract class implements the core functionalities found in all frame 
 * grabbers, intialistation, starting the capture, capturing frames, stopping
 * the capture and releasing resources. It must be subclassed.<br>
 * Subclasses must implement {@link FrameGrabber#getImageFormat()} to return the
 * correct image format used for capture. {@link #init()} may also be overriden
 * if required.
 * @author gilles 
 */

abstract class AbstractGrabber implements FrameGrabber {
	protected final static int RAW_GRABBER = 0;
	protected final static int JPEG_GRABBER = 1;
	protected final static int RGB24_GRABBER = 2;
	protected final static int BGR24_GRABBER = 3;
	protected final static int YUV_GRABBER = 4;
	protected final static int YVU_GRABBER = 5;
	
	protected DeviceInfo dInfo;
	private int width;
	private int height;
	private int channel;
	private int standard;
	private int nbV4LBuffers = 4;
	private ByteBuffer[] bufs;
	protected State state;
	protected int format;
	private Tuner tuner;
	private int type;
	
	/*
	 * JNI returns a long (which is really a pointer) when a device is allocated
	 * for use. This field is read-only (!!!) 
	 */
	protected long object;
	
	static {
		try {
			System.loadLibrary("v4l4j");
		} catch (UnsatisfiedLinkError e) {
			System.err.println("Cant load v4l4j JNI library");
			throw e;
		}
	}
	

	private native ByteBuffer[] doInit(long o, int w, int h, int ch, int std,
			int nbBuf, int requestedFormat, int output)
		throws V4L4JException;
	private native void start(long o) throws V4L4JException;
	/**
	 * This method sets a new value for the JPEG quality
	 * @param o the struct v4l4_device
	 * @param i the new value
	 * @throws V4L4JException if the JPEG quality is disabled because of the
	 * type of this frame grabber (not {@link #JPEG_GRABBER}).
	 */
	protected native void setQuality(long o, int i);
	private native int getBuffer(long o) throws V4L4JException;
	private native int getBufferLength(long o);
	private native void stop(long o);
	private native void doRelease(long o);
	private native void doSetFrameIntv(long o, int n, int d) throws InvalidValue;
	private native int doGetFrameIntv(long o, int what);

	
	/**
	 * This constructor builds a FrameGrabber object used to capture frames from
	 * a video source.
	 * @param di the DeviceInfo object associated with the video device who 
	 * created this grabber
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
	 * @param ty the output image format, ie the type of this frame grabber:
	 * {@link #RAW_GRABBER}, {@link #JPEG_GRABBER}, {@link #RGB24_GRABBER}, 
	 * {@link #BGR24_GRABBER}, {@link #YUV_GRABBER}, {@link #YVU_GRABBER}
	 * @throw {@link ImageFormatException} if the image format is null and 
	 * type = {@link #RAW_GRABBER}
	 */
	protected AbstractGrabber(DeviceInfo di, long o, int w, int h, int ch, int std
			, Tuner t,ImageFormat imf, int ty) throws ImageFormatException{
		if(imf==null)
			throw new ImageFormatException("The image format can not be null");
		state= new State();
		dInfo = di;
		object = o;
		width = w;
		height = h;
		channel = ch;
		standard= std;
		format = imf.getIndex();
		tuner = t;
		type = ty;
	}
	
	
	/**
	 * This method initialises the capture, and apply the capture parameters.
	 * V4L may either adjust the height and width parameters to the closest 
	 * valid values or reject them altogether. If the values were adjusted, 
	 * they can be retrieved after calling {@link #init()} using 
	 * {@link #getWidth()} and {@link #getHeight()}.
	 * @throws VideoStandardException if the chosen video standard is not 
	 * supported
	 * @throws ImageFormatException for a raw frame grabber, this exception is 
	 * thrown if the chosen Image format is unsupported.
	 * @throws CaptureChannelException if the given channel number value is not 
	 * valid
	 * @throws ImageDimensionException if the given image dimensions are not 
	 * supported
	 * @throws InitialisationException if the video device file can not be 
	 * initialised 
	 * @throws StateException if the frame grabber is already initialised
	 * @throws V4L4JException if there is an error applying capture parameters
	 */
	void init() throws V4L4JException{
		state.init();
		bufs = doInit(object, width, height, channel, standard, nbV4LBuffers,
				format, type);
		state.commit();
	}
	
	/* (non-Javadoc)
	 * @see au.edu.jcu.v4l4j.FrameGrabber#setFrameInterval()
	 */
	@Override
	public void setFrameInterval(int num, int denom) throws InvalidValue{
		synchronized(state){
			if(!state.isStarted())
				throw new StateException("Invalid method call");
			doSetFrameIntv(object, num, denom);
		}
	}
	
	/* (non-Javadoc)
	 * @see au.edu.jcu.v4l4j.FrameGrabber#getFrameInterval()
	 */
	@Override
	public DiscreteInterval getFrameInterval() {
		synchronized(state){
			//TODO: not sure if the following if statement is required
			//ie, it might be possible to get the current frame intv
			//while capturing... to be tested
			if(!state.isStarted())
				throw new StateException("Invalid method call: cannot get the"
						+" frame interval while capturing.");
			return new DiscreteInterval(
					doGetFrameIntv(object, 0), doGetFrameIntv(object, 1)
			);
		}
	}
	
	/* (non-Javadoc)
	 * @see au.edu.jcu.v4l4j.FrameGrabber#getTuner()
	 */
	@Override
	public final Tuner getTuner() throws NoTunerException{
		if(tuner==null)
			throw new NoTunerException("This input does not have a tuner");
		
		state.checkReleased();
		return tuner;
	}
	
	/* (non-Javadoc)
	 * @see au.edu.jcu.v4l4j.FrameGrabber#startCapture()
	 */
	@Override
	public final void startCapture() throws V4L4JException {
		state.start();
		start(object);
		state.commit();
	}
	
	/* (non-Javadoc)
	 * @see au.edu.jcu.v4l4j.FrameGrabber#getFrame()
	 */
	@Override
	public final ByteBuffer getFrame() throws V4L4JException {
		//we need the synchronized statement to serialise calls to getBuffer
		//since libvideo is not reentrant. Also we dont want the frame grabber
		//to be released in the middle of a getBuffer() call.
		synchronized(state) {
			if(!state.isStarted())
				throw new StateException("Invalid method call");
			ByteBuffer b = bufs[getBuffer(object)];
			b.limit(getBufferLength(object)).position(0);
			return b;
		}
	}
	
	/* (non-Javadoc)
	 * @see au.edu.jcu.v4l4j.FrameGrabber#stopCapture()
	 */
	@Override
	public final void stopCapture(){
		state.stop();		
		stop(object);
		state.commit();
	}
	
	/**
	 * This method releases resources used by the FrameCapture object.
	 * @throws StateException if if this 
	 * <code>FrameGrabber</code> has been already released, and therefore must 
	 * not be used anymore.
	 */
	final void release(){
		try {stopCapture();}
		catch (StateException se) {
			//capture already stopped 
		}
		
		state.remove();		
		doRelease(object);
		state.commit();
	}
	
	/* (non-Javadoc)
	 * @see au.edu.jcu.v4l4j.FrameGrabber#getHeight()
	 */
	@Override
	public final int getHeight(){
		state.checkReleased();
		return height;
	}
	
	/* (non-Javadoc)
	 * @see au.edu.jcu.v4l4j.FrameGrabber#getWidth()
	 */
	@Override
	public final int getWidth(){
		state.checkReleased();
		return width;
	}
	
	/* (non-Javadoc)
	 * @see au.edu.jcu.v4l4j.FrameGrabber#getChannel()
	 */
	@Override
	public final int getChannel(){
		state.checkReleased();
		return channel;
	}
	
	/* (non-Javadoc)
	 * @see au.edu.jcu.v4l4j.FrameGrabber#getStandard()
	 */
	@Override
	public final int getStandard(){
		state.checkReleased();
		return standard;
	}
	
	protected static class State {

		private int state;
		private int temp;
		private int users;
		
		private static int UNINIT=0;
		private static int INIT=1;
		private static int STARTED=2;
		private static int STOPPED=3;
		private static int RELEASED=4;

		public State() {
			state = UNINIT;
			temp = UNINIT;
			users = 0;
		}
		
		public synchronized void init(){
			if(state==UNINIT && temp!=INIT) {
				temp=INIT;
			} else
				throw new StateException("This FrameGrabber can not be "
						+"initialised again");
		}
		
		public synchronized void start(){
			if(state==INIT || state==STOPPED && temp!=STARTED) {
				temp=STARTED;
			} else
				throw new StateException("This FrameGrabber is not initialised"
						+" or stopped and can not be started");
		}
		
		/**
		 * Must be called with state object lock held.
		 * @throws StateException if released
		 * @return
		 */
		public boolean isStarted(){
			checkReleased();
			return state==STARTED && temp!=STOPPED;
		}
		
		/**
		 * Must be called with state object lock held
		 * @return
		 */
		public void checkReleased(){
			if(state==RELEASED || temp==RELEASED)
				throw new StateException("This FrameGrabber has been released");
		}

		public synchronized void get(){
			if(state==INIT || state==STARTED && temp!=STOPPED) {
				users++;
			} else
				throw new StateException("This FrameGrabber is neither "
						+"initialised nor started and can not be used");
		}
		
		public synchronized void put(){
			if(state==INIT || state==STARTED) {
				if(--users==0  && temp==STOPPED)
					notify();
			} else
				throw new StateException("This FrameGrabber is neither "
						+"initialised nor started and can not be used");
		}
		
		
		public synchronized void stop(){
			if(state==STARTED && temp!=STOPPED) {
				temp=STOPPED;
				while(users!=0)
					try {
						wait();
					} catch (InterruptedException e) {
						System.err.println("Interrupted while waiting for "
								+"FrameGrabber users to complete");
						e.printStackTrace();
						throw new StateException("There are remaining users of "
								+"this FrameGrabber and it can not be stopped");
					}
			} else
				throw new StateException("This FrameGrabber is not started and "
						+"can not be stopped");
		}
		
		public synchronized void remove(){
			if(state==INIT || state==STOPPED && temp!=RELEASED) {
				temp=RELEASED;
			} else
				throw new StateException("This FrameGrabber is neither "
						+"initialised nor stopped and can not be released");
		}
		
		public synchronized void commit(){
			state=temp;
		}
	}
}

