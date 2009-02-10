package au.edu.jcu.v4l4j;

import au.edu.jcu.v4l4j.exceptions.StateException;

/**
 * Instances of this class are used to represent a tuner attached to a video input.
 * The provided methods control the frequency of the tuner and report the
 * received signal strength and the automatic frequency control (when its value is
 * negative, the frequency is too low and when positive, it is too high). The AFC may 
 * not be available on all hardware. If not, <code>getAFC()</code> returns a value of 0.
 * <code>Tuner</code> are not directly instantiated. Instead, to retrieve the <code>Tuner</code>,
 * call <code>getTuner()</code> on the associated <code>FrameGrabber</code> object.
 * 
 * @author gilles
 *
 */
public class Tuner {
	private long v4l4j_object;

	private native long getFreq(long o);
	private native void setFreq(long o, long f);
	private native int getAfc(long o);
	private native int getRssi(long o);
	
	private boolean released;
	
	Tuner(long o){
		v4l4j_object = o;
		released = false;
	}
	
	/**
	 * This method returns the current frequency of this tuner.
	 * @return the current frequency 
	 */
	public synchronized long getFrequency(){
		checkRelease();
		return getFreq(v4l4j_object);
	}
	
	/**
	 * This method sets the frequency of this tuner.
	 * @param f the new frequency
	 */
	public synchronized void setFrequency(long f){
		checkRelease();
		setFreq(v4l4j_object, f);
	}
	
	/**
	 * This method returns the current AFC value. When its value is
	 * negative, the frequency is too low and when positive, it is too high.
	 * @return the current AFC value
	 */
	public synchronized int getAFC() {
		checkRelease();
		return getAfc(v4l4j_object);
	}
	
	/**
	 * This method returns the current received signal strength.
	 * @return the current received signal strength
	 */
	public synchronized int getRSSI(){
		checkRelease();
		return getRssi(v4l4j_object);
	}
	
	/**
	 * This method releases this tuner.
	 */
	synchronized void release(){
		released = true;
	}
	
	private void checkRelease(){
		if(released)
			throw new StateException("This tuner object has been released already and must not be used anymore");
	}
}
