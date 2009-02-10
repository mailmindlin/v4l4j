package au.edu.jcu.v4l4j;

public class V4L4JConstants {

	/**
	 * Input from a tuner
	 */
	public static short TUNER = 1;
	/**
	 * Camera-type input
	 */
	public static short CAMERA = 2;
	/**
	 * Video standard value for webcams
	 */
	public static int WEBCAM=0;
	/**
	 * Video standard value for PAL sources
	 */
	public static int PAL=1;
	/**
	 * Video standard value for SECAM sources
	 */
	public static int SECAM=2;
	/**
	 * Video standard value for NTSC sources
	 */
	public static int NTSC=3;
	/**
	 * Setting the capture width to this value will set the actual width to the
	 * maximum width supported by the hardware  
	 */
	public static int MAX_WIDTH = 0;
	/**
	 * Setting the capture height to this value will set the actual height to the
	 * maximum height supported by the hardware  
	 */
	public static int MAX_HEIGHT = 0;
	/**
	 * This value represents the maximum value of the JPEG quality setting
	 */
	public static int MAX_JPEG_QUALITY = 100;
	/**
	 * This value represents the minimum value of the JPEG quality setting
	 */
	public static int MIN_JPEG_QUALITY = 0;
	/**
	 * If a control has a type equal to BUTTON, its value is always 0,
	 * and pressing it is done by setting any value using <code>setValue()</code> 
	 */
	public final static int BUTTON=0;
	/**
	 * If a control has a type equal to SLIDER, it accepts a range of value between a minimum (as returned by <code>Control.getMin()</code>) and
	 * a maximum (as returned by <code>Control.getMax()</code>) in increments (as returned by <code>Control.getStep()</code>)
	 */
	public final static int SLIDER=1;
	/**
	 * If a control has a type equal to SWITCH, it accepts two different values: 0 (as returned by <code>Control.getMin()</code>) and
	 *  1 (as returned by <code>getMax()</code>
	 */
	public final static int SWITCH=2;	
	/**
	 * If a control has a type equal to DISCRETE, it accepts discrete values as returned by <code>Control.getDiscreteValues()</code>.
	 * These discrete values may be have a string description (returned by <code>Control.getDescriptions()</code>)are in the range 
	 * <code>Control.getMin()</code> and	<code>Control.getMax()</code>. The step value as returned by <code>Control.getStep()</code> is not applicable.
	 */
	public final static int DISCRETE=3;
	/**
	 * Radio tuner
	 */
	public static int RADIO = 1;
	/**
	 * TV tuner
	 */
	public static int TV = 2;
	/**
	 * Frequencies in KHz
	 */
	public static int KHZ = 1;
	/**
	 * Frequencies in MHz
	 */
	public static int MHZ = 2;

}
