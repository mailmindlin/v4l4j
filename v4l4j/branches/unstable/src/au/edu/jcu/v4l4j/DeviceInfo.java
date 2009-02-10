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

import java.util.List;
import java.util.Vector;

import au.edu.jcu.v4l4j.exceptions.V4L4JException;

/**
 * Objects of this class encapsulate the following information about a video 
 * device:
 * <ul>
 * <li>its device file,</li>
 * <li>the name of the video device,</li>
 * <li>a list of {@link InputInfo} object providing information about each 
 * video input,</li>
 * <li>and a list of {@link ImageFormat}s used by the video device.
 * </ul>
 * To retrieve information about a video device, call {@link VideoDevice#getDeviceInfo()} on its 
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
 * </code>
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
	 * This method returns the device file associated with the video device.
	 * @return the device file
	 */
	public String getDeviceFile() {
		return deviceFile;
	}


	/**
	 * This method returns a list of {@link InputInfo} objects which provide
	 * information about each video input supported by the video device.
	 * @return a list of <code>InputInfo</code> objects
	 * @see InputInfo
	 */
	public List<InputInfo> getInputs() {
		return new Vector<InputInfo>(inputs);
	}


	/**
	 * This method returns a list of {@link ImageFormat} objects detailing
	 * the various image format the video device supports.
	 * @return a list of <code>ImageFormat</code> objects
	 * @see ImageFormat 
	 */
	public List<ImageFormat> getFormats() {
		return new Vector<ImageFormat>(formats);
	}


	/**
	 * This constructor build a DeviceInfo object containing information about 
	 * the given V4L device.
	 * @param object the JNI C pointer to struct v4l4j_device
	 * @throws V4L4JException if there is an error retrieving information from
	 * the video device.
	 */	
	DeviceInfo(long object, String dev) throws V4L4JException{
		inputs = new Vector<InputInfo>();
		formats = new Vector<ImageFormat>();
		deviceFile = dev;
	
		getInfo(object);
	}

}
