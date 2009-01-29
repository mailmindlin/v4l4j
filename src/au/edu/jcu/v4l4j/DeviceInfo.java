package au.edu.jcu.v4l4j;

import java.util.List;
import java.util.Vector;

import au.edu.jcu.v4l4j.exceptions.NoTunerException;


public class DeviceInfo {
	
	/**
	 * Native method interacting with libv4l to get info on device f
	 * @param f the full path to the V4L device file
	 */
	private native void getInfo(String f);
	
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
	private List<Input> inputs;
	
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
	public List<Input> getInputs() {
		return new Vector<Input>(inputs);
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
	 * @param f the full path to the V4L device file
	 */	
	public DeviceInfo(String f){
		deviceFile = f;
		inputs = new Vector<Input>();
		formats = new Vector<ImageFormat>();
		
		getInfo(deviceFile);
	}
	
	public static void main(String[] args) throws NoTunerException{
		DeviceInfo d = new DeviceInfo(args[0]);
		System.out.println("name: "+d.getName());
		System.out.println("Device file: "+d.getDeviceFile());
		System.out.println("Supported formats:");
		
		for(ImageFormat f : d.getFormats())
			System.out.println("\t"+f.getName()+" - "+f.getLibv4lID());
		
		System.out.println("Inputs:");
		for(Input i: d.getInputs()){
			System.out.println("\tName: "+i.getName());
			System.out.println("\tType: "+i.getType()+"("+(i.getType() == Input.CAMERA ? "Camera" : "Tuner")+")");
			System.out.println("\tSupported standards:");
			for(Integer s: i.getSupportedStandards())
				System.out.println("\t\t"+s);
			if(i.getType() == Input.TUNER) {
				Tuner t = i.getTuner();
				System.out.println("\tTuner");
				System.out.println("\t\tname: "+t.getName());
				System.out.println("\t\tRange high: "+t.getRangeHigh());
				System.out.println("\t\tRange low: "+t.getRangeLow());
				System.out.println("\t\tUnit: "+t.getUnit()+"("+(t.getUnit() == Tuner.MHZ ? "MHz" : "kHz")+")");
				System.out.println("\t\tType: "+t.getType()+"("+(t.getType() == Tuner.RADIO ? "Radio" : "TV")+")");				
			}
		}
	}

}
