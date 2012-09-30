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

import au.edu.jcu.v4l4j.FrameGrabber;
import au.edu.jcu.v4l4j.ImageFormatList;
import au.edu.jcu.v4l4j.CaptureCallback;
import au.edu.jcu.v4l4j.V4L4JConstants;
import au.edu.jcu.v4l4j.VideoDevice;
import au.edu.jcu.v4l4j.VideoFrame;
import au.edu.jcu.v4l4j.exceptions.V4L4JException;

/**
 * Objects of this class compute the maximum achievable frame rate for a video
 * device, and print it.
 * @author gilles
 *
 */
public class GetFrameRate implements CaptureCallback{
	public static final int 	captureLength = 10;	// seconds
	private static String 		dev;
	private static int 			inFmt, outFmt, width, height, channel, std, intv;

	private FrameGrabber 	fg;
	private VideoDevice 	vd;
	private ImageFormatList imfList;
	private int 			numFrames;
	private long			startTime, currentTime;
	private boolean			seenFirstFrame;


	public static void main(String[] args) throws Exception {
		dev = (System.getProperty("test.device") != null) ? System.getProperty("test.device") : "/dev/video0"; 
		width = (System.getProperty("test.width")!=null) ? Integer.parseInt(System.getProperty("test.width")) : 640;
		height = (System.getProperty("test.height")!=null) ? Integer.parseInt(System.getProperty("test.height")) : 480;
		std = (System.getProperty("test.standard")!=null) ? Integer.parseInt(System.getProperty("test.standard")) : V4L4JConstants.STANDARD_WEBCAM;
		channel = (System.getProperty("test.channel")!=null) ? Integer.parseInt(System.getProperty("test.channel")) : 0;
		inFmt = (System.getProperty("test.inFormat")!=null) ? Integer.parseInt(System.getProperty("test.inFormat")) : -1;
		//outformat: RAW: 0 , JPEG:1, RGB:2 , bgr=3, yuv=4, yvu=5
		outFmt = (System.getProperty("test.outFormat")!=null) ? Integer.parseInt(System.getProperty("test.outFormat")) : 0;
		intv = (System.getProperty("test.fps")!=null) ? Integer.parseInt(System.getProperty("test.fps")) : -1;

		System.out.println("This program will open "+dev+", capture frames for "
				+ captureLength+ " seconds and print the FPS");

		new GetFrameRate().startTest();
	}

	/**
	 * This method builds a new object to test the maximum FPS for a video 
	 * device
	 * @throws V4L4JException if there is an error initialising the video device
	 */
	public GetFrameRate() throws V4L4JException{
		try {
			vd = new VideoDevice(dev);
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
				System.out.println("Unknown / unsupported output format: "+outFmt);
				throw new V4L4JException("unknown / unsupported output format");
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
		//set the frame rate
		if(intv!=-1){
			try {
				System.out.println("setting frame rate to "+intv);
				fg.setFrameInterval(1, intv);
			} catch (Exception e){
				System.out.println("Couldnt set the frame interval");
			}
		}

		// enable push mode
		fg.setCaptureCallback(this);

		// start capture
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
	 * @throws InterruptedException 
	 */
	public void startTest() throws V4L4JException, InterruptedException{
		startTime = 0;
		currentTime = 0;
		numFrames = 0;
		seenFirstFrame = false;

		System.out.println("Starting test capture at "+
				width+"x"+height+" for "+captureLength+" seconds");

		startCapture();

		// block until 'captureLength' seconds have passed
		synchronized(this){
			wait();
		}
		fg.stopCapture();
		
		System.out.println(" =====  TEST RESULTS  =====");
		System.out.println("\tFrames captured :"+numFrames);
		System.out.println("\tFPS: "+((float) numFrames/(currentTime/1000-startTime/1000)));
		System.out.println(" =====  END  RESULTS  =====");

		vd.releaseFrameGrabber();
		vd.release();	
	}

	@Override
	public void nextFrame(VideoFrame frame) {

		// Some drivers take a long time to start the capture and we dont
		// want to include the time it takes to start the capture when we 
		// calculate the FPS. So, we start counting only once we have 
		// received the first frame
		if (! seenFirstFrame) {
			// This is the first frame we receive, note the start time
			startTime = System.currentTimeMillis();
		} else {
			// This is not the first frame we receive, increment frame count
			numFrames++;
			
			// Check how much time has passed since the start time
			currentTime = System.currentTimeMillis();
			if (currentTime >= startTime + (captureLength * 1000)) {
				// 'captureLength' seconds have passed, wake up main thread 
				// which will stop the capture
				synchronized(this) {
					notify();
				}
			}			
		}
		
		seenFirstFrame = true;
		frame.recycle();
	}

	@Override
	public void exceptionReceived(V4L4JException e) {
		e.printStackTrace();
		System.out.println("Failed to perform test capture");
		
		// wake up main thread
		synchronized(this) {
			notify();
		}
	}
}
