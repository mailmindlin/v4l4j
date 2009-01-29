package au.edu.jcu.v4l4j;

import java.util.HashSet;
import java.util.Set;

import au.edu.jcu.v4l4j.exceptions.NoTunerException;

public class Input {
	/**
	 * The name of this input
	 */
	private String name;
	
	/**
	 * Input from a tuner
	 */
	public static short TUNER = 1;
	
	/**
	 * Camera-type input
	 */
	public static short CAMERA = 2;
	
	/**
	 * The type of this input
	 */
	private short type;
	
	/**
	 * A set of the supported standards (FrameGrabber.PAL, FrameGrabber.SECAM
	 * FrameGrabber.NTSC, FrameGrabber.WEBCAM)
	 */
	private Set<Integer> supportedStandards;
	
	/**
	 * The tuner associated with this input, if any
	 */
	private Tuner tuner;
	
	/**
	 * This constructor builds an Input object representing a physical input on the V4L 
	 * video device.
	 * @param n the name of the input
	 * @param stds the supported standards
	 * @param tun the tuner associated with this input if any
	 */
	private Input(String n, int[] stds, short t, Tuner tun){
		tuner = tun;
		supportedStandards = new HashSet<Integer>();
		for(int s: stds)
			supportedStandards.add(new Integer(s));
		type = t;
		name = n;
	}
	
	/**
	 * This constructor builds an Input object representing a TUNER input on the V4L 
	 * video device.
	 * @param n the name of the input
	 * @param stds the supported standards
	 * @param tun the tuner associated with this input
	 */
	public Input(String n, int[] stds, Tuner tun){
		this(n, stds, TUNER, tun);
	}
	
	/**
	 * This constructor builds an Input object representing a CAMERA input on the V4L 
	 * video device.
	 * @param n the name of the input
	 * @param stds the supported standards
	 */
	public Input(String n, int[] stds){
		this(n,stds,CAMERA ,null);
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
	public Tuner getTuner() throws NoTunerException{
		if(tuner==null)
			throw new NoTunerException("No tuner connected to this input");
		
		return tuner;
	}
}
