package au.edu.jcu.v4l4j;

import java.util.List;
import java.util.Vector;

import au.edu.jcu.v4l4j.exceptions.V4L4JException;


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
	 * @return the name
	 */
	public String getName() {
		return name;
	}

	/**
	 * @return the deviceFile
	 */
	public String getDeviceFile() {
		return deviceFile;
	}


	/**
	 * @return the inputs
	 */
	public List<InputInfo> getInputs() {
		return new Vector<InputInfo>(inputs);
	}


	/**
	 * @return the formats
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
