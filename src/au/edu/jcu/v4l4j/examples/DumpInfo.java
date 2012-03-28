/*
* Copyright (C) 2011 Gilles Gigan (gilles.gigan@gmail.com)
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
package au.edu.jcu.v4l4j.examples;

import java.util.List;

import au.edu.jcu.v4l4j.Control;
import au.edu.jcu.v4l4j.DeviceInfo;
import au.edu.jcu.v4l4j.ImageFormat;
import au.edu.jcu.v4l4j.InputInfo;
import au.edu.jcu.v4l4j.TunerInfo;
import au.edu.jcu.v4l4j.V4L4JConstants;
import au.edu.jcu.v4l4j.VideoDevice;
import au.edu.jcu.v4l4j.exceptions.NoTunerException;
import au.edu.jcu.v4l4j.exceptions.V4L4JException;

/**
 * This class simply creates a {@link VideoDevice}, gets its {@link DeviceInfo}
 * object, and prints its contents.
 *
 * @author gilles
 *
 */
public class DumpInfo {
	private VideoDevice videoDevice;
	private DeviceInfo deviceInfo;
	
	public static void main(String args[]) throws V4L4JException{
		String device = (System.getProperty("test.device") != null) ? System.getProperty("test.device") : "/dev/video0";
		new DumpInfo(device);
	}
	
	/**
	 * This method prints information for the given video device file.
	 * @param deviceFile the device file for which we need information 
	 * @throws V4L4JException if there is an error getting the device information
	 */
	public DumpInfo(String deviceFile) throws V4L4JException{
		videoDevice = new VideoDevice(deviceFile);
		deviceInfo = videoDevice.getDeviceInfo();
		dumpInfo();
		videoDevice.releaseControlList();
		videoDevice.release();
	}
	
	/**
	 * This method prints the contents of the {@link DeviceInfo} object member 
	 * 'deviceInfo' 
	 */
	private void dumpInfo() {
		 
		System.out.println("name: "+deviceInfo.getName());
		System.out.println("Device file: "+deviceInfo.getDeviceFile());
		System.out.println("Supported formats:");
		for(ImageFormat f : deviceInfo.getFormatList().getNativeFormats())
			System.out.println("\t"+f.toNiceString());
		
		System.out.println("\tFormats that can be RGB24-converted: "
				+(videoDevice.supportRGBConversion()?"":"None"));
		if(videoDevice.supportRGBConversion())
			for(ImageFormat f: deviceInfo.getFormatList().getRGBEncodableFormats())
				System.out.println("\t\t"+f.toNiceString());
		
		System.out.println("\tFormats that can be BGR24-converted: "
				+(videoDevice.supportBGRConversion()?"":"None"));
		if(videoDevice.supportBGRConversion())
			for(ImageFormat f: deviceInfo.getFormatList().getBGREncodableFormats())
				System.out.println("\t\t"+f.toNiceString());
		
		System.out.println("\tFormats that can be YUV420-converted: "
				+(videoDevice.supportYUVConversion()?"":"None"));
		if(videoDevice.supportYUVConversion())
			for(ImageFormat f: deviceInfo.getFormatList().getYUVEncodableFormats())
				System.out.println("\t\t"+f.toNiceString());
		
		System.out.println("\tFormats that can be YVU420-converted: "
				+(videoDevice.supportYVUConversion()?"":"None"));
		if(videoDevice.supportYVUConversion())
			for(ImageFormat f: deviceInfo.getFormatList().getYVUEncodableFormats())
				System.out.println("\t\t"+f.toNiceString());
		
		System.out.println("\tFormats that can be JPEG-encoded: "
				+(videoDevice.supportJPEGConversion()?"":"None"));
		if(videoDevice.supportJPEGConversion())
			for(ImageFormat f: deviceInfo.getFormatList().getJPEGEncodableFormats())
				System.out.println("\t\t"+f.toNiceString());
		
				
		System.out.println("Inputs:");
		for(InputInfo i: deviceInfo.getInputs()){
			System.out.println("\tName: "+i.getName());
			System.out.println("\tType: "+i.getType()+"("+(i.getType() == 
				V4L4JConstants.INPUT_TYPE_CAMERA ? "Camera" : "Tuner")+")");
			System.out.println("\tIndex: "+i.getIndex());
			System.out.println("\tSupported standards:");
			for(Integer s: i.getSupportedStandards()){
				System.out.print("\t\t"+s);
				if(s==V4L4JConstants.STANDARD_PAL)
					System.out.println("(PAL)");
				else if(s==V4L4JConstants.STANDARD_NTSC)
					System.out.println("(NTSC)");
				else if(s==V4L4JConstants.STANDARD_SECAM)
					System.out.println("(SECAM)");
				else
					System.out.println("(None/Webcam)");
			}
			if(i.getType() == V4L4JConstants.INPUT_TYPE_TUNER) {
				TunerInfo t;
				try {
					t = i.getTunerInfo();
					System.out.println("\tTuner");
					System.out.println("\t\tname: "+t.getName());
					System.out.println("\t\tIndex: "+t.getIndex());
					System.out.println("\t\tRange high: "+t.getRangeHigh());
					System.out.println("\t\tRange low: "+t.getRangeLow());
					System.out.println("\t\tUnit: "+t.getUnit()+"("+(t.getUnit() == 
						V4L4JConstants.FREQ_MHZ ? "MHz" : "kHz")+")");
					System.out.println("\t\tType: "+t.getType()+"("+(t.getType() == 
						V4L4JConstants.TUNER_TYPE_RADIO ? "Radio" : "TV")+")");
				} catch (NoTunerException e) {
					// no tuner
				}				
			}
		}
		
		List<Control> ctrls = videoDevice.getControlList().getList();
		for(Control c: ctrls){
			if (c.getType() == V4L4JConstants.CTRL_TYPE_STRING) {
				System.out.print("String control: "+c.getName()+" - min: "+c.getMinValue()+
						" - max: "+c.getMaxValue()+" - step: "+c.getStepValue()+
						" - value: ");
				try {
					System.out.println(c.getStringValue());
				} catch (V4L4JException ve){
					System.out.println(" ERROR");
					ve.printStackTrace();
				}
			} else if (c.getType() == V4L4JConstants.CTRL_TYPE_LONG) {
				System.out.print("Long control: "+c.getName() + " - value: ");
				try {
					System.out.println(c.getLongValue());
				} catch (V4L4JException ve){
					System.out.println(" ERROR");
				}
			} else {
				System.out.print("Control: "+c.getName()+" - min: "+c.getMinValue()+
						" - max: "+c.getMaxValue()+" - step: "+c.getStepValue()+
						" - value: ");
				try {
					System.out.println(c.getValue());
				} catch (V4L4JException ve){
					System.out.println(" ERROR");
				}
			}
		}		
	}
}
