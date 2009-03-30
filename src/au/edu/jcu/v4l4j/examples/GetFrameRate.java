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

import au.edu.jcu.v4l4j.FrameGrabber;
import au.edu.jcu.v4l4j.V4L4JConstants;
import au.edu.jcu.v4l4j.VideoDevice;
import au.edu.jcu.v4l4j.exceptions.V4L4JException;

public class GetFrameRate {
	public static void main(String[] args) throws V4L4JException, IOException {
		String dev;
		int w, h, std, channel, inFmt, outFmt, captureLength = 10;
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
		
		//RAW: 0 , JPEG:1, 2: RGB
		try {
			outFmt = Integer.parseInt(args[6]);
		} catch (Exception e){
			outFmt = 0;
		}
		

		long start=0, now=0;
		int n=0;
		FrameGrabber f = null;
		VideoDevice vd = null;

		System.out.println("This program will open "+dev+", capture frames for "
					+ captureLength+ " seconds and print the FPS");

		try {
			vd = new VideoDevice(dev);
			if(outFmt==1 && vd.supportJPEGConversion()) {
				if(vd.getDeviceInfo().getFormatList().getFormat(inFmt)==null)
					System.out.println("Capture format "+inFmt+" doesnt exist, trying to find a suitable one");
				else 
					System.out.println("Trying capture format "+vd.getDeviceInfo().getFormatList().getFormat(inFmt).getName());
				
				f= vd.getJPEGFrameGrabber(w, h, channel, std, 80,vd.getDeviceInfo().getFormatList().getFormat(inFmt));
				System.out.println("Output image format: JPEG");
			} else if(outFmt==2 && vd.supportRGBConversion()) {
				if(vd.getDeviceInfo().getFormatList().getFormat(inFmt)==null)
					System.out.println("Capture format "+inFmt+" doesnt exist, trying to find a suitable one");
				else 
					System.out.println("Trying capture format "+vd.getDeviceInfo().getFormatList().getFormat(inFmt).getName());
				
				f= vd.getRGBFrameGrabber(w, h, channel, std,vd.getDeviceInfo().getFormatList().getFormat(inFmt));
				System.out.println("Output image format: RGB");
			} else {
				if(inFmt==-1){
					System.out.println("No capture format specified, v4l4j will pick the first one");
					f= vd.getRawFrameGrabber(w, h, channel, std);
				} else {
					if(vd.getDeviceInfo().getFormatList().getFormat(inFmt)==null){
						System.out.println("The specified input format does not exist, or is not supported by the video device");
						throw new V4L4JException("Unsupported image format");
					}
					System.out.println("Trying capture format "+vd.getDeviceInfo().getFormatList().getFormat(inFmt).getName());
					f= vd.getRawFrameGrabber(w, h, channel, std,vd.getDeviceInfo().getFormatList().getFormat(inFmt));					
				}
				System.out.println("Output image format: RAW  (same as capture format)");
			}
			System.out.println("Capture image format: "+f.getImageFormat().getName());
		} catch (V4L4JException e) {
			e.printStackTrace();
			System.out.println("Failed to instanciate the FrameGrabber ("+dev+")");
			vd.release();
			throw e;
		}

		try {
			f.startCapture();
		} catch (V4L4JException e) {
			e.printStackTrace();
			System.out.println("Failed to start capture");
			vd.releaseFrameGrabber();
			vd.release();
			throw e;
		}

		try {
			System.out.println("Starting test capture at "+f.getWidth()+"x"+f.getHeight()+" for "+captureLength+" seconds");
			now=start=System.currentTimeMillis();
			while(now<start+(captureLength*1000)){
				f.getFrame();
				//Uncomment the following to dump the captured frame to a jpeg file
				//also import java.io.FileOutputStream 
				//new FileOutputStream("file"+n+".raw").getChannel().write(f.getFrame());
				n++;
				now=System.currentTimeMillis();
			}
		} catch (V4L4JException e) {
			e.printStackTrace();
			System.out.println("Failed to perform test capture");
			f.stopCapture();
			vd.releaseFrameGrabber();
			vd.release();
			throw e;
		}

		System.out.println(" =====  TEST RESULTS  =====");
		System.out.println("\tFrames captured :"+n);
		System.out.println("\tFPS: "+((float) n/(now/1000-start/1000)));
		System.out.println(" =====  END  RESULTS  =====");

		f.stopCapture();
		vd.releaseFrameGrabber();
		vd.release();
	}
}
