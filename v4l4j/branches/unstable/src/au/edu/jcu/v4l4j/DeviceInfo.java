package au.edu.jcu.v4l4j;

import java.util.List;
import java.util.Vector;

import au.edu.jcu.v4l4j.exceptions.V4L4JException;

/**
 * Objects of this class encapsulate the following information about a video device:
 * <ul>
 * <li>Its device file,</li>
 * <li>The name of the video device,</li>
 * <li>A list of <code>InputInfo</code> object providing information about each video input,</li>
 * <li>and a list of image formats used by the video device.
 * </ul>
 * To retrieve information about a video device, call <code>getDeviceInfo()</code> on its 
 * <code>VideoDevice</code> object, and retrieve parameters of interest.
 * Example:<br>
 * <code>
 * VideoDevice vd = new VideoDevice("/dev/video0");<br>
 * DeviceInfo d = vd.getDeviceInfo();<br>
 * System.out.println("name: "+d.getName());<br>
 * System.out.println("Device file: "+d.getDeviceFile());<br>
 * System.out.println("Supported formats:");<br>
 * for(ImageFormat f : d.getFormats())<br>
 * &nbsp;&nbsp;System.out.println("\t"+f.getName()+" - "+f.getIndex());<br>
 * System.out.println("Inputs:");<br>
 * for(InputInfo i: d.getInputs()){<br>
 * &nbsp;&nbsp;System.out.println("\tName: "+i.getName());<br>
 * &nbsp;&nbsp;System.out.println("\tType: "+i.getType()+"("+(i.getType() == V4L4JConstants.CAMERA ? "Camera" : "Tuner")+")");<br>
 * &nbsp;&nbsp;System.out.println("\tIndex: "+i.getIndex());<br>
 * &nbsp;&nbsp;System.out.println("\tSupported standards:");<br>
 * &nbsp;&nbsp;for(Integer s: i.getSupportedStandards()){<br>
 * &nbsp;&nbsp;&nbsp;&nbsp;System.out.print("\t\t"+s);<br>
 * &nbsp;&nbsp;&nbsp;&nbsp;if(s==V4L4JConstants.PAL)<br>
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;System.out.println("(PAL)");<br>
 * &nbsp;&nbsp;&nbsp;&nbsp;else if(s==V4L4JConstants.NTSC)<br>
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;System.out.println("(NTSC)");<br>
 * &nbsp;&nbsp;&nbsp;&nbsp;else if(s==V4L4JConstants.SECAM)<br>
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;System.out.println("(SECAM)");<br>
 * &nbsp;&nbsp;&nbsp;&nbsp;else<br
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;System.out.println("(None/Webcam)");<br>
 * &nbsp;&nbsp;}<br>
 * &nbsp;&nbsp;if(i.getType() == V4L4JConstants.TUNER) {<br>
 * &nbsp;&nbsp;&nbsp;&nbsp;TunerInfo t = i.getTuner();<br>
 * &nbsp;&nbsp;&nbsp;&nbsp;System.out.println("\tTuner");<br>
 * &nbsp;&nbsp;&nbsp;&nbsp;System.out.println("\t\tname: "+t.getName());<br>
 * &nbsp;&nbsp;&nbsp;&nbsp;System.out.println("\t\tIndex: "+t.getIndex());<br>
 * &nbsp;&nbsp;&nbsp;&nbsp;System.out.println("\t\tRange high: "+t.getRangeHigh());<br>
 * &nbsp;&nbsp;&nbsp;&nbsp;System.out.println("\t\tRange low: "+t.getRangeLow());<br>
 * &nbsp;&nbsp;&nbsp;&nbsp;System.out.println("\t\tUnit: "+t.getUnit()+"("+(t.getUnit() == TunerInfo.MHZ ? "MHz" : "kHz")+")");<br>
 * &nbsp;&nbsp;&nbsp;&nbsp;System.out.println("\t\tType: "+t.getType()+"("+(t.getType() == TunerInfo.RADIO ? "Radio" : "TV")+")");<br>
 * &nbsp;&nbsp;}<br>
 * }<br>
 * vd.release();<br>
 * 
 * @author gilles
 *
 */
public class DeviceInfo {
	
	/**
	 * Native method interacting with libv4l to get info on device f
	 * @param f the full path to the V4L device file
	 */
	private native void getInfo(long o);
	
	static {
		try {
			System.loadLibrary("v4l4j");
		} catch (UnsatisfiedLinkError e) {
			System.err.println("Cant load v4l4j JNI library");
			throw e;
		}
	}
	
	/**
	 * The name of the video device
	 */
	private String name;
	
	/**
	 * The path to the device file
	 */
	private String deviceFile;
	
	/**
	 * A list of inputs connected to this device
	 */
	private List<InputInfo> inputs;
	
	/**
	 * A list of supported image formats
	 */
	private List<ImageFormat> formats;	
	
	
	/**
	 * This method returns the name of the video device.
	 * @return the name of the video device.
	 */
	public String getName() {
		return name;
	}

	/**
	 * This method returns the device file associated with the video device
	 * @return the device file
	 */
	public String getDeviceFile() {
		return deviceFile;
	}


	/**
	 * This method returns a list of <code>InputInfo</code> objects which provide
	 * information about each video input supported by the video device.
	 * @return a list of <code>InputInfo</code> objects
	 */
	public List<InputInfo> getInputs() {
		return new Vector<InputInfo>(inputs);
	}


	/**
	 * This method returns a list of <code>ImageFormat</code> objects detailing
	 * the various image format the video device supports.
	 * @return a list of <code>ImageFormat</code> objects 
	 */
	public List<ImageFormat> getFormats() {
		return new Vector<ImageFormat>(formats);
	}


	/**
	 * This constructor build a DeviceInfo object containing information about the 
	 * given V4L device
	 * @param object the JNI C pointer to struct v4l4j_device
	 * @throws V4L4JException if there is an error retrieving information from the vide device
	 */	
	DeviceInfo(long object, String dev) throws V4L4JException{
		inputs = new Vector<InputInfo>();
		formats = new Vector<ImageFormat>();
		deviceFile = dev;
	
		getInfo(object);
	}

}
