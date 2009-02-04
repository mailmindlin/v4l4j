package au.edu.jcu.v4l4j;

import java.util.HashSet;
import java.util.Set;

import au.edu.jcu.v4l4j.exceptions.NoTunerException;

/**
 * Objects of this class encapsulate information about a video device:<br>
 * <ul>
 * <li>The name of this input</li>
 * <li>The supported standards (<code>FrameGrabber.PAL</code>, <code>FrameGrabber.SECAM</code>,
 * <code>FrameGrabber.NTSC</code> and <code>FrameGrabber.WEBCAM</code>)</li>
 * <li>The input type (<code>InputInfo.TUNER</code> or <code>InputInfo.CAMERA</code></li>
 * <li>The <code>TunerInfo</code> object associated with this input if it is connected to a tuner.</li>
 * </ul>
 * You do not directly instantiate <code>InputInfo</code> objects. Instead, they are created by
 * the corresponding <code>VideoDevice</code> object through its <code>getDeviceInfo()</code> method. 
 * 
 * @author gilles
 *
 */
public class InputInfo {
	/**
	 * The name of this input
	 */
	private String name;
	
	/**
	 * The type of this input
	 */
	private short type;
	
	/**
	 * A set of the supported standards (<code>V4L4JConstants.PAL</code>, <code>V4L4JConstants.SECAM</code>
	 * <code>V4L4JConstants.NTSC</code>, <code>V4L4JConstants.WEBCAM</code>)
	 */
	private Set<Integer> supportedStandards;
	
	/**
	 * The tuner associated with this input, if any
	 */
	private TunerInfo tuner;
	
	/**
	 * The V4L id associated with this input
	 */
	private int index;
	
	/**
	 * This constructor builds an Input object representing a physical input on the V4L 
	 * video device.
	 * @param n the name of the input
	 * @param stds the supported standards
	 * @param tun the tuner associated with this input if any
	 */
	private InputInfo(String n, int[] stds, short t, TunerInfo tun, int id){
		tuner = tun;
		supportedStandards = new HashSet<Integer>();
		for(int s: stds)
			supportedStandards.add(new Integer(s));
		type = t;
		name = n;
		index = id;
	}
	
	/**
	 * This constructor builds an Input object representing a TUNER input on the V4L 
	 * video device.
	 * @param n the name of the input
	 * @param stds the supported standards
	 * @param tun the tuner associated with this input
	 */
	InputInfo(String n, int[] stds, TunerInfo tun, int id){
		this(n, stds, V4l4JConstants.TUNER, tun, id);
	}
	
	/**
	 * This constructor builds an Input object representing a CAMERA input on the V4L 
	 * video device.
	 * @param n the name of the input
	 * @param stds the supported standards
	 */
	InputInfo(String n, int[] stds, int id){
		this(n,stds,V4l4JConstants.CAMERA ,null, id);
	}

	/**
	 * This method returns the name of this input
	 * @return the name
	 */
	public String getName() {
		return name;
	}

	/**
	 * This method returns the type of this input (Input.TUNER or Input.CAMERA)
	 * @return the type
	 */
	public short getType() {
		return type;
	}

	/**
	 * This method returns the standards supported by this input
	 * @return the supportedStandards
	 */
	public Set<Integer> getSupportedStandards() {
		return supportedStandards;
	}

	/**
	 * This method returns the tuner associated with this input, if any.
	 * @return the tuner
	 * @throws NoTunerException if this input is of type Input.CAMERA, and is not
	 * a tuner
	 */
	public TunerInfo getTuner() throws NoTunerException{
		if(tuner==null)
			throw new NoTunerException("No tuner connected to this input");
		
		return tuner;
	}
	
	/**
	 * This method returns the (V4L) index of this input
	 * @return the (V4L) index of this input
	 */
	public int getIndex(){
		return index;
	}
}
