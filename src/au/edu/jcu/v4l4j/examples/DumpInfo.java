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
import java.util.Map;

import au.edu.jcu.v4l4j.Control;
import au.edu.jcu.v4l4j.DeviceInfo;
import au.edu.jcu.v4l4j.FrameInterval;
import au.edu.jcu.v4l4j.FrameInterval.DiscreteInterval;
import au.edu.jcu.v4l4j.FrameInterval.StepwiseInterval;
import au.edu.jcu.v4l4j.ImageFormat;
import au.edu.jcu.v4l4j.ImageFormatList;
import au.edu.jcu.v4l4j.InputInfo;
import au.edu.jcu.v4l4j.ResolutionInfo;
import au.edu.jcu.v4l4j.ResolutionInfo.DiscreteResolution;
import au.edu.jcu.v4l4j.ResolutionInfo.StepwiseResolution;
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

	public static void main(String args[]) throws V4L4JException {
		String device = System.getProperty("test.device", "/dev/video0");
		new DumpInfo(device);
	}

	/**
	 * This method prints information for the given video device file.
	 * 
	 * @param deviceFile
	 *            the device file for which we need information
	 * @throws V4L4JException
	 *             if there is an error getting the device information
	 * @throws InterruptedException 
	 */
	public DumpInfo(String deviceFile) throws V4L4JException {
		this.videoDevice = new VideoDevice(deviceFile);
		this.deviceInfo = videoDevice.getDeviceInfo();
		dumpInfo();
		videoDevice.releaseControlList();
		videoDevice.release();
	}

	/**
	 * This method prints the contents of the {@link DeviceInfo} object member
	 * 'deviceInfo'
	 * @throws InterruptedException 
	 */
	private void dumpInfo() {

		System.out.flush();
		System.out.println("Camera name:  " + deviceInfo.getName());
		System.out.println("Device file: " + deviceInfo.getDeviceFile());
		System.out.println("Supported formats:");
		ImageFormatList formats = deviceInfo.getFormatList();
		
		List<ImageFormat> nativeFormats = formats.getNativeFormats();
		System.out.flush();
		System.out.println("\t- Native Formats (" + nativeFormats.size() + "): ");
		for (ImageFormat f : nativeFormats)
			dumpFormatInfo(f);
		
		System.out.println("\tFormats that can be RGB24-converted (" + formats.getRGBEncodableFormats().size() + ") :");
		if (videoDevice.supportRGBConversion())
			for (ImageFormat f : deviceInfo.getFormatList().getRGBEncodableFormats())
				dumpFormatInfo(f);
		
		System.out.println("\tFormats that can be BGR24-converted (" + formats.getBGREncodableFormats().size() + ") :");
		if (videoDevice.supportBGRConversion())
			for (ImageFormat f : deviceInfo.getFormatList().getBGREncodableFormats())
				dumpFormatInfo(f);

		System.out.println("\tFormats that can be YUV420-converted (" + formats.getYUVEncodableFormats().size() + ") :");
		if (videoDevice.supportYUVConversion())
			for (ImageFormat f : deviceInfo.getFormatList().getYUVEncodableFormats())
				dumpFormatInfo(f);

		System.out.println("\tFormats that can be YVU420-converted (" + formats.getYVUEncodableFormats().size() + ") :");
		if (videoDevice.supportYVUConversion())
			for (ImageFormat f : deviceInfo.getFormatList().getYVUEncodableFormats())
				dumpFormatInfo(f);

		System.out.println("\tFormats that can be JPEG-encoded (" + formats.getJPEGEncodableFormats().size() + ") :");
		if (videoDevice.supportJPEGConversion())
			for (ImageFormat f : deviceInfo.getFormatList().getJPEGEncodableFormats())
				dumpFormatInfo(f);

		System.out.println("Inputs:");
		for (InputInfo i : deviceInfo.getInputs()) {
			System.out.println("\tIndex: " + i.getIndex());
			System.out.println("\tName: " + i.getName());
			System.out.println("\tType: " + i.getType() + "("
					+ (i.getType() == V4L4JConstants.INPUT_TYPE_CAMERA ? "Camera" : "Tuner") + ")");
			System.out.println("\tSupported standards:");
			for (Integer s : i.getSupportedStandards()) {
				System.out.print("\t\t" + s + " ");
				if (s == V4L4JConstants.STANDARD_PAL)
					System.out.println("(PAL)");
				else if (s == V4L4JConstants.STANDARD_NTSC)
					System.out.println("(NTSC)");
				else if (s == V4L4JConstants.STANDARD_SECAM)
					System.out.println("(SECAM)");
				else
					System.out.println("(None/Webcam)");
			}
			if (i.getType() == V4L4JConstants.INPUT_TYPE_TUNER) {
				TunerInfo t;
				try {
					t = i.getTunerInfo();
					System.out.println("\tTuner");
					System.out.println("\t\tname: " + t.getName());
					System.out.println("\t\tIndex: " + t.getIndex());
					System.out.println("\t\tRange high: " + t.getRangeHigh());
					System.out.println("\t\tRange low: " + t.getRangeLow());
					System.out.println("\t\tUnit: " + t.getUnit() + "("
							+ (t.getUnit() == V4L4JConstants.FREQ_MHZ ? "MHz" : "kHz") + ")");
					System.out.println("\t\tType: " + t.getType() + "("
							+ (t.getType() == V4L4JConstants.TUNER_TYPE_RADIO ? "Radio" : "TV") + ")");
				} catch (NoTunerException e) {
					// no tuner
				}
			}
		}
		
		List<Control> ctrls = videoDevice.getControlList().getList();
		for (Control c : ctrls) {
			if (c.getType() == V4L4JConstants.CTRL_TYPE_STRING) {
				System.out.print("String control: " + c.getName() + " - min: " + c.getMinValue() + " - max: "
						+ c.getMaxValue() + " - step: " + c.getStepValue() + " - value: ");
				try {
					System.out.println(c.getStringValue());
				} catch (V4L4JException ve) {
					System.out.println(" ERROR");
					ve.printStackTrace();
				}
			} else if (c.getType() == V4L4JConstants.CTRL_TYPE_LONG) {
				System.out.print("Long control: " + c.getName() + " - value: ");
				try {
					System.out.println(c.getLongValue());
				} catch (V4L4JException ve) {
					System.out.println(" ERROR");
				}
			} else if (c.getType() == V4L4JConstants.CTRL_TYPE_DISCRETE) {
				Map<String, Integer> valueMap = c.getDiscreteValuesMap();
				System.out.print("Menu control: " + c.getName() + " - value: ");
				try {
					int value = c.getValue();
					System.out.print(value);
					try {
						System.out.println(" (" + c.getDiscreteValueName(c.getDiscreteValues().indexOf(value)) + ")");
					} catch (Exception e) {
						System.out.println(" (unknown)");
					}
				} catch (V4L4JException ve) {
					System.out.println(" ERROR");
				}
				System.out.println("\tMenu entries:");
				for (String s : valueMap.keySet())
					System.out.println("\t\t" + valueMap.get(s) + " - " + s);
			} else {
				System.out.print("Control: " + c.getName() + " - min: " + c.getMinValue() + " - max: " + c.getMaxValue()
						+ " - step: " + c.getStepValue() + " - value: ");
				try {
					System.out.println(c.getValue());
				} catch (V4L4JException ve) {
					System.out.println(" ERROR");
				}
			}
		}
	}
	private void dumpFormatInfo(ImageFormat format) {
		StringBuffer sb = new StringBuffer();
		sb.append("\t\t").append(format.toString()).append('\n');
		
		ResolutionInfo resolutions = format.getResolutionInfo();
		sb.append("\t\t\tType: ").append(resolutions.getType()).append('\n');
		if (resolutions.getType() == ResolutionInfo.Type.DISCRETE) {
			for (DiscreteResolution resolution : resolutions.getDiscreteResolutions()) {
				sb.append("\t\t\t").append(resolution.getWidth()).append('x').append(resolution.getHeight()).append('\n');
				dumpFrameIntervalInfo(resolution.getFrameInterval(), sb);
			}
		} else if (resolutions.getType() == ResolutionInfo.Type.STEPWISE) {
			StepwiseResolution resolution = resolutions.getStepwiseResolution();
			sb.append("\t\t\tFrom ").append(resolution.getMinWidth()).append('x').append(resolution.getMinHeight()).append('\n');
			dumpFrameIntervalInfo(resolution.getMinResFrameInterval(), sb);
			sb.append("\t\t\tStep ").append(resolution.getWidthStep()).append('x').append(resolution.getHeightStep()).append('\n');
			sb.append("\t\t\tTo   ").append(resolution.getMaxWidth()).append('x').append(resolution.getMaxHeight()).append('\n');
			dumpFrameIntervalInfo(resolution.getMaxResFrameInterval(), sb);
		} else {
			sb.append("\t\t\tUnknown resolution type: ").append(resolutions.getType()).append('\n');
		}
		System.out.println(sb.toString());
	}
	
	private void dumpFrameIntervalInfo(FrameInterval intervals, StringBuffer sb) {
		if (intervals.getType() == FrameInterval.Type.DISCRETE) {
			for (DiscreteInterval interval : intervals.getDiscreteIntervals())
				sb.append("\t\t\t\t").append(getIntervalPrettyString(interval)).append('\n');
		} else if (intervals.getType() == FrameInterval.Type.STEPWISE) {
			StepwiseInterval interval = intervals.getStepwiseInterval();
			sb.append("\t\t\t\tFrom ").append(getIntervalPrettyString(interval.getMinInterval())).append('\n');
			sb.append("\t\t\t\tStep ").append(getIntervalPrettyString(interval.getStepInterval())).append('\n');
			sb.append("\t\t\t\tTo   ").append(getIntervalPrettyString(interval.getMaxInterval())).append('\n');
		} else {
			sb.append("\t\t\t\tUnknown interval type ").append(intervals.getType()).append('\n');
		}
	}
	
	private String getIntervalPrettyString(DiscreteInterval interval) {
		double fps = .1 * Math.round(10.0 * interval.getDenominator() / interval.getNumerator());
		return interval.getNumerator() + "/" + interval.getDenominator() + " (" + fps + " FPS)";
	}
}
