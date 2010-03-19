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

package au.edu.jcu.v4l4j.examples;

import java.io.IOException;

import au.edu.jcu.v4l4j.DeviceList;
import au.edu.jcu.v4l4j.FrameGrabber;
import au.edu.jcu.v4l4j.ImageFormatList;
import au.edu.jcu.v4l4j.V4L4JConstants;
import au.edu.jcu.v4l4j.VideoDevice;
import au.edu.jcu.v4l4j.exceptions.V4L4JException;

/**
 * Objects of this class compute the maximum achievable frame rate for a video
 * device, and print it.
 * @author gilles
 *
 */
public class GetFrameRate {
	public static final int captureLength = 10;
	private int inFmt, outFmt, width, height, channel, std, intv;
	private FrameGrabber fg;
	private VideoDevice vd;
	private ImageFormatList imfList;
	
	/**
	 * This method builds a new object to test the maximum FPS for a video 
	 * device
	 * @param dev the full path to the device file
	 * @param ifmt the input format (-1, or a device-supported format)
	 * @param ofmt the output format (0: raw - 1: JPEG - 2: RGB - 3: BGR - 
	 * 4:YUV - 5:YVU)
	 * @param w the desired capture width
	 * @param h the desired capture height
	 * @param c the input index
	 * @param s the video standard (0: webcam - 1: PAL - 2:SECAM - 3:NTSC)
	 * @param iv the frame rate (how many frame per second)
	 * @throws V4L4JException if there is an error initialising the video device
	 */
	public GetFrameRate(String dev, int ifmt, int ofmt, int w, int h, int c, 
			int s, int iv) throws V4L4JException{
		inFmt = ifmt;
		outFmt = ofmt;
		width = w;
		height = h;
		channel = c;
		std = s;
		intv = iv;
		try {
			vd = DeviceList.getVideoDeviceFromDeviceFile(dev);
			imfList = vd.getDeviceInfo().getFormatList();
			if(outFmt==0)
				getRawFg();
			else if(outFmt==1 && vd.supportJPEGConversion())
				getJPEGfg();
			else if(outFmt==2 && vd.supportRGBConversion())
				getRGBfg();
			else if(outFmt==3 && vd.supportBGRConversion())
				getBGRfg();
			else if(outFmt==4 && vd.supportYUVConversion())
				getYUVfg();
			else if(outFmt==5 && vd.supportYVUConversion())
				getYVUfg();
			else {
				System.out.println("Unknown output format: "+outFmt);
				throw new V4L4JException("unknown output format");
			}
			System.out.println("Input image format: "+fg.getImageFormat().getName());
		} catch (V4L4JException e) {
			e.printStackTrace();
			System.out.println("Failed to instanciate the FrameGrabber ("+dev+")");
			vd.release();
			throw e;
		}
		width = fg.getWidth();
		height = fg.getHeight();
		std = fg.getStandard();
		channel = fg.getChannel();
	}
	
	private void getJPEGfg() throws V4L4JException{
		if(inFmt==-1 ||	imfList.getJPEGEncodableFormat(inFmt)==null){
			System.out.println("Invalid format / no capture format " +
					"specified, let v4l4j find a suitable one");
			fg= vd.getJPEGFrameGrabber(width, height, channel, std, 80);
		} else{ 
			System.out.println("Trying input format "+
					imfList.getJPEGEncodableFormat(inFmt).getName());
			fg= vd.getJPEGFrameGrabber(width, height, channel, std, 80, 
					imfList.getJPEGEncodableFormat(inFmt));
		}				
		System.out.println("Output image format: JPEG");		
	}
	
	private void getRGBfg() throws V4L4JException{
		if(inFmt==-1 || imfList.getRGBEncodableFormat(inFmt)==null){
			System.out.println("Invalid format / no capture format " +
					"specified, let v4l4j find a suitable one");
			fg= vd.getRGBFrameGrabber(width, height, channel, std);
		} else { 
			System.out.println("Trying input format "+
					imfList.getRGBEncodableFormat(inFmt).getName());
			fg= vd.getRGBFrameGrabber(width, height, channel, std,
					imfList.getRGBEncodableFormat(inFmt));
		}
		System.out.println("Output image format: RGB");		
	}
	
	private void getBGRfg() throws V4L4JException{
		if(inFmt==-1 || imfList.getBGREncodableFormat(inFmt)==null){
			System.out.println("Invalid format / no capture format " +
					"specified, let v4l4j find a suitable one");
			fg= vd.getBGRFrameGrabber(width, height, channel, std);
		} else { 
			System.out.println("Trying input format "+
					imfList.getBGREncodableFormat(inFmt).getName());
			fg= vd.getBGRFrameGrabber(width, height, channel, std,
					imfList.getBGREncodableFormat(inFmt));
		}
		System.out.println("Output image format: BGR");		
	}
	
	private void getYUVfg() throws V4L4JException{
		if(inFmt==-1 || imfList.getYUVEncodableFormat(inFmt)==null){
			System.out.println("Invalid format / no capture format " +
					"specified, let v4l4j find a suitable one");
			fg= vd.getYUVFrameGrabber(width, height, channel, std);
		} else { 
			System.out.println("Trying input format "+
					imfList.getYUVEncodableFormat(inFmt).getName());
			fg= vd.getYUVFrameGrabber(width, height, channel, std,
					imfList.getYUVEncodableFormat(inFmt));
		}
		System.out.println("Output image format: YUV");		
	}
	
	private void getYVUfg() throws V4L4JException{
		if(inFmt==-1 || imfList.getYVUEncodableFormat(inFmt)==null){
			System.out.println("Invalid format / no capture format " +
					"specified, let v4l4j find a suitable one");
			fg= vd.getYVUFrameGrabber(width, height, channel, std);
		} else { 
			System.out.println("Trying input format "+
					imfList.getYVUEncodableFormat(inFmt).getName());
			fg= vd.getYVUFrameGrabber(width, height, channel, std,
					imfList.getYVUEncodableFormat(inFmt));
		}
		System.out.println("Output image format: YVU");		
	}
	
	private void getRawFg() throws V4L4JException{
		if(inFmt==-1 || imfList.getNativeFormat(inFmt)==null){
			System.out.println("Invalid format / no capture format " +
					"specified, v4l4j will pick the first one");
			fg= vd.getRawFrameGrabber(width, height, channel, std);
		} else {
			System.out.println("Trying input format "+
					imfList.getNativeFormat(inFmt).getName());
			fg= vd.getRawFrameGrabber(width, height, channel, std,
					imfList.getNativeFormat(inFmt));					
		}
		System.out.println("Output image format: RAW (same as input)");
	}
	
	private void startCapture() throws V4L4JException{
		if(intv!=-1){
			//try setting the frame rate
			try {
				System.out.println("setting frame rate to "+intv);
				fg.setFrameInterval(1, intv);
			} catch (Exception e){
				System.out.println("Couldnt set the frame interval");
			}
		}
		try {
			fg.startCapture();
		} catch (V4L4JException e) {
			e.printStackTrace();
			System.out.println("Failed to start capture");
			vd.releaseFrameGrabber();
			vd.release();
			throw e;
		}
	}
	
	/**
	 * This method starts the frame rate test. It will run for 10 seconds, and
	 * then print the achieved FPS
	 * @throws V4L4JException if there is an error capturing frames
	 */
	public void startTest() throws V4L4JException{
		long start=0, now=0;
		int n=0;
		startCapture();

		try {
			//discard the first frame to make sure device has settled
			fg.getFrame();
			
			System.out.println("Starting test capture at "+
					width+"x"+height+" for "+captureLength+" seconds");
			now=start=System.currentTimeMillis();
			while(now<start+(captureLength*1000)){
				fg.getFrame();
				//Uncomment the following to dump the captured frame to a jpeg file
				//also import java.io.FileOutputStream 
				//new FileOutputStream("file"+n+".raw").getChannel().write(f.getFrame());
				n++;
				now=System.currentTimeMillis();
			}
			
			System.out.println(" =====  TEST RESULTS  =====");
			System.out.println("\tFrames captured :"+n);
			System.out.println("\tFPS: "+((float) n/(now/1000-start/1000)));
			System.out.println(" =====  END  RESULTS  =====");
			
		} catch (V4L4JException e) {
			e.printStackTrace();
			System.out.println("Failed to perform test capture");
			throw e;
		} finally {
			fg.stopCapture();
			vd.releaseFrameGrabber();
			vd.release();
		}		
	}
	
	public static void main(String[] args) throws V4L4JException, IOException {
		String dev;
		int w, h, std, channel, inFmt, outFmt, intv;
		//Check if we have the required args
		//otherwise put sensible values in
		try {
			dev = args[0];
		} catch (Exception e){
			dev = "/dev/video0";
		}
		try {
			w = Integer.parseInt(args[1]);
		} catch (Exception e){
			w = V4L4JConstants.MAX_WIDTH;
		}
		try{			
			h = Integer.parseInt(args[2]);
		} catch  (Exception e) {
			h = V4L4JConstants.MAX_HEIGHT;
		}
		try {
			std = Integer.parseInt(args[3]);
		} catch (Exception e) {
			std = 0;
		}
		try {
			channel = Integer.parseInt(args[4]);
		} catch (Exception e){
			channel = 0;
		}
		try {
			inFmt = Integer.parseInt(args[5]);
		} catch (Exception e){
			inFmt = -1;
		}
		
		//RAW: 0 , JPEG:1, RGB:2 , bgr=3, yuv=4, yvu=5
		try {
			outFmt = Integer.parseInt(args[6]);
		} catch (Exception e){
			outFmt = 0;
		}
		
		try {
			intv = Integer.parseInt(args[7]);
		} catch (Exception e){
			intv=-1;
		}

		System.out.println("This program will open "+dev+", capture frames for "
					+ captureLength+ " seconds and print the FPS");
		
		new GetFrameRate(dev, inFmt, outFmt, w, h, channel, std, intv).startTest();
	}
}
