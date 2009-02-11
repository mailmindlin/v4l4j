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
 * 
 * @author gilles 
 */

import java.nio.ByteBuffer;

import au.edu.jcu.v4l4j.exceptions.CaptureChannelException;
import au.edu.jcu.v4l4j.exceptions.ImageFormatException;
import au.edu.jcu.v4l4j.exceptions.InitialisationException;
import au.edu.jcu.v4l4j.exceptions.NoTunerException;
import au.edu.jcu.v4l4j.exceptions.StateException;
import au.edu.jcu.v4l4j.exceptions.V4L4JException;
import au.edu.jcu.v4l4j.exceptions.VideoStandardException;


/**
 * This class provides methods to capture raw frames from a {@link VideoDevice}.
 * Raw means that the image format will be left untouched and passed on straight
 * away to the caller. <code>FrameGrabber</code> objects are not instantiated
 * directly. Instead, the 
 * {@link VideoDevice#getRawFrameGrabber(int, int, int, int) getRawFrameGrabber()}
 * method must be called on the associated {@link VideoDevice}. Requested height
 * and width may be adjusted to the closest supported values. The adjusted width 
 * and height can be retrieved by calling {@link #getWidth()} and 
 * {@link #getHeight()}.<br>
 * A typical <code>FrameGrabber</code> use is as follows:<br><br>
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
 * Once the frame grabber is released with 
 * {@link VideoDevice#releaseFrameGrabber()}, it can be re-initialised again 
 * with one of the <code>getXXFrameGrabber()</code> method again. Similarly,
 * when the capture is stopped with {@link #stopCapture()}, it can be started 
 * again with {@link #stopCapture()} without having to create a new 
 * <code>FrameGrabber</code>.
 * 
 * @author gilles
 *
 */
public class FrameGrabber {
	private int width;
	private int height;
	private int channel;
	protected int quality;
	private int standard;
	private int nbV4LBuffers = 4;
	private ByteBuffer[] bufs;
	private State state;
	private ImageFormat format;
	private Tuner tuner;
	
	/*
	 * JNI returns a long (which is really a pointer) when a device is allocated for use
	 * This field is read-only (!!!) 
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
	

	private native ByteBuffer[] doInit(long o, int w, int h, int ch, int std, int nbBuf, int q, int requestedFormat)
		throws V4L4JException;
	private native void start(long o) throws V4L4JException;
	/**
	 * This method sets a new value for the JPEG quality
	 * @param o the struct v4l4_device
	 * @param i the new value
	 * @throws V4L4JException if the JPEG quality is disabled because if the image format
	 */
	protected native void setQuality(long o, int i);
	private native int getBuffer(long o) throws V4L4JException;
	private native int getBufferLength(long o);
	private native void stop(long o);
	private native void doRelease(long o) throws V4L4JException;
	private native int hasTuner(long o);

	
	/**
	 * This constructor builds a FrameGrabber object used to capture JPEG frames from a video source
	 * @param w the requested frame width 
	 * @param h the requested frame height
	 * @param ch the input index, as returned by <code>InputInfo.getIndex()</code>
	 * @param std the video standard, as returned by <code>InputInfo.getSupportedStandards()</code>
	 * (see V4L4JConstants)
	 * @param q the JPEG image quality (the higher, the better the quality)
	 */
	protected FrameGrabber(long o, int w, int h, int ch, int std, int q){
		quality = q;			

		state= new State();
		
		object = o;
		width = w;
		height = h;
		channel = ch;
		standard= std;
		format = null;
		tuner = null;
	}
	
	/**
	 * This constructor builds a FrameGrabber object used to capture raw frames from a video source.
	 * The image formats to be tried are given through the argument <code>fmts</code>. The first 
	 * successful format will be used.
	 * @param w the requested frame width 
	 * @param h the requested frame height
	 * @param ch the input index, as returned by <code>InputInfo.getIndex()</code>
	 * @param std the video standard, as returned by <code>InputInfo.getSupportedStandards()</code>
	 * (see V4L4JConstants)
	 * @param fmts an array of image format indexes
	 */
	protected  FrameGrabber(long o, int w, int h, int ch, int std, ImageFormat fmt){
		state= new State();
		
		object = o;
		width = w;
		height = h;
		channel = ch;
		standard= std;
		quality = 0;
		format = fmt;
		tuner = null;
	}
	
	/**
	 * This method initialises the capture, and apply the capture parameters.
	 * V4L may either adjust the height and width parameters to the closest valid values
	 * or reject them altogether. If the values were adjusted, they can be retrieved 
	 * after calling init() using getWidth() and getHeight()
	 * @throws VideoStandardException if the chosen video standard is not supported
	 * @throws ImageFormatException if the selected video device uses an unsupported image format (let the author know, see README file)
	 * @throws CaptureChannelException if the given channel number value is not valid
	 * @throws ImageDimensionException if the given image dimensions are not supported
	 * @throws InitialisationException if the video device file cant be initialised 
	 * @throws StateException if the framegrabber is already initialised
	 * @throws V4L4JException if there is an error applying capture parameters
	 */
	void init() throws V4L4JException{
		state.init();		
		if(format!=null)
			bufs = doInit(object, width, height, channel, standard, nbV4LBuffers, -1, format.getIndex());
		else 
			bufs = doInit(object, width, height, channel, standard, nbV4LBuffers, quality, -1);
		state.commit();
	}
	
	/**
	 * This method returns a {@link Tuner} object associated with this 
	 * <code>FrameGrabber</code>, or throws a {@link NoTunerException} if there
	 * is none. The {@link Tuner} object must be released with 
	 * {@link #releaseTuner()} when finished, otherwise this <code>FrameGrabber</code>
	 * can not be released.
	 * @return a {@link Tuner} object
	 * @throws NoTunerException if the selected input does not have a tuner
	 * @throws StateException if this <code>FrameGrabber</code> has been already
	 * released, and therefore must not be used anymore
	 */
	public Tuner getTuner() throws NoTunerException{
		if(hasTuner(object)==1){
			synchronized(this){
				if(tuner==null){
					state.get();
					tuner = new Tuner(object, channel);
				}
				return tuner;
			}
		} else
			throw new NoTunerException("This input does not have a tuner");
	}
	
	/**
	 * This method releases the {@link Tuner} object associated with this
	 * <code>FrameGrabber</code>. It does nothing if this 
	 * <code>FrameGrabber</code> does not have a tuner, or if a reference to it
	 * has not been retrieved before (with a call to {@link #getTuner()}).
	 * @throws StateException if this <code>FrameGrabber</code> has been already
	 * released, and therefore must not be used anymore
	 */
	public void releaseTuner(){
		if(hasTuner(object)==1){
			synchronized(this){
				if(tuner!=null){
					tuner.release();
					tuner=null;
					state.put();
				}
			}
		}
	}
	
	/**
	 * This method starts the capture. After this call, frames can be retrieved
	 * with {@link #getFrame()}.
	 * @throws V4L4JException if the capture cant be started
	 * @throws StateException if this <code>FrameGrabber</code> has been already
	 * released, and therefore must not be used anymore
	 */
	public void startCapture() throws V4L4JException {
		state.start();
		start(object);
		state.commit();
	}
	
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
	public ByteBuffer getFrame() throws V4L4JException {
		//we need the synchronized statement to serialise calls to getBuffer
		//since libv4l is not reentrant
		synchronized(state) {
			if(!state.isStarted())
				throw new StateException("Invalid method call");
			ByteBuffer b = bufs[getBuffer(object)];
			b.limit(getBufferLength(object)).position(0);
			return b;
		}
	}
	
	/**
	 * This method stops the capture.
	 * @throws StateException if the capture has not been started or if this 
	 * <code>FrameGrabber</code> has been already released, and therefore must
	 * not be used anymore.
	 */
	public void stopCapture(){
		state.stop();		
		stop(object);
		state.commit();
	}
	
	/**
	 * This method releases resources used by the FrameCapture object.
	 * @throws V4L4JException if there is a problem freeing resources
	 */
	void release() throws V4L4JException {
		releaseTuner();
		try {stopCapture();}
		catch (StateException se) {}
		
		state.remove();		
		doRelease(object);
		state.commit();
	}
	
	/**
	 * This method returns the actual height of captured frames.
	 * @return the height
	 */
	public int getHeight(){
		return height;
	}
	
	/**
	 * This method returns the actual width of captured frames.
	 * @return the width
	 */
	public int getWidth(){
		return width;
	}
	
	private static class State {

		private int state;
		private int temp;
		private int users;
		
		private int UNINIT=0;
		private int INIT=1;
		private int STARTED=2;
		private int STOPPED=3;
		private int REMOVED=4;

		public State() {
			state = UNINIT;
			temp = UNINIT;
			users = 0;
		}
		
		public synchronized void init(){
			if(state==UNINIT && temp!=INIT) {
				temp=INIT;
			} else
				throw new StateException("This FrameGrabber can not be initialised again");
		}
		
		public synchronized void start(){
			if(state==INIT || state==STOPPED && temp!=STARTED) {
				temp=STARTED;
			} else
				throw new StateException("This FrameGrabber is not initialised or stopped and can not be started");
		}
		
		/**
		 * Must be called with state object lock held
		 * @return
		 */
		public boolean isStarted(){
			return state==STARTED && temp!=STOPPED;
		}

		public synchronized void get(){
			if(state==INIT || state==STARTED && temp!=STOPPED) {
				users++;
			} else
				throw new StateException("This FrameGrabber is neither initialised nor started and can not be used");
		}
		
		public synchronized void put(){
			if(state==INIT || state==STARTED) {
				if(--users==0  && temp==STOPPED)
					notify();
			} else
				throw new StateException("This FrameGrabber is neither initialised nor started and can not be used");
		}
		
		
		public synchronized void stop(){
			if(state==STARTED && temp!=STOPPED) {
				temp=STOPPED;
				while(users!=0)
					try {
						wait();
					} catch (InterruptedException e) {
						System.err.println("Interrupted while waiting for FrameGrabber users to complete");
						e.printStackTrace();
						throw new StateException("There are remaining users of this FrameGrabber and it can not be stopped");
					}
			} else
				throw new StateException("This FrameGrabber is not started and can not be stopped");
		}
		
		public synchronized void remove(){
			if(state==INIT || state==STOPPED && temp!=REMOVED) {
				temp=REMOVED;
			} else
				throw new StateException("This FrameGrabber is neither initialised nor stopped and can not be released");
		}
		
		public synchronized void commit(){
			state=temp;
		}
	}
}

