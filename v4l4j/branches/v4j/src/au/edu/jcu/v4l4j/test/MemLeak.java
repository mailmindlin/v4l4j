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
package au.edu.jcu.v4l4j.test;

import java.io.IOException;

import au.edu.jcu.v4l4j.DeviceList;
import au.edu.jcu.v4l4j.FrameGrabber;
import au.edu.jcu.v4l4j.VideoDevice;
import au.edu.jcu.v4l4j.exceptions.V4L4JException;

public class MemLeak implements Runnable{
	private boolean stop;
	private String dev;
	private VideoDevice vd;
	
	public MemLeak(String d) throws V4L4JException{
		stop = false;
		dev = d;
	}
	
	public void stop(){
		stop = true;
	}
	
	public void run(){
		FrameGrabber fg;
		int count = 0;
		
		while(!stop){
			count = 0;
			try {
				vd = DeviceList.getVideoDeviceFromDeviceFile(dev);
				vd.getControlList();
				fg = vd.getJPEGFrameGrabber(640, 480, 1, 1, 80);
				fg.startCapture();
				while(count++<200)
					fg.getFrame();
				fg.stopCapture();
				vd.releaseFrameGrabber();	
			} catch (V4L4JException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}

			try {
				count = 0;
				fg = vd.getRGBFrameGrabber(640, 480, 1, 1);
				fg.startCapture();
				while(count++<200)
					fg.getFrame();
				fg.stopCapture();
				vd.releaseFrameGrabber();	
			} catch (V4L4JException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}

			try {
				count = 0;
				fg = vd.getBGRFrameGrabber(640, 480, 1, 1);
				fg.startCapture();
				while(count++<200)
					fg.getFrame();
				fg.stopCapture();
				vd.releaseFrameGrabber();	
			} catch (V4L4JException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}

			try {
				count = 0;
				fg = vd.getYUVFrameGrabber(640, 480, 1, 1);
				fg.startCapture();
				while(count++<200)
					fg.getFrame();
				fg.stopCapture();
				vd.releaseFrameGrabber();	
			} catch (V4L4JException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}

			try {
				count = 0;
				fg = vd.getYVUFrameGrabber(640, 480, 1, 1);
				fg.startCapture();
				while(count++<200)
					fg.getFrame();
				fg.stopCapture();
				vd.releaseFrameGrabber();	
			} catch (V4L4JException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			vd.releaseControlList();
			vd.release();
		}
		
	}
	
	public void release(){
	}

	public static void main(String[] args) throws V4L4JException, IOException, InterruptedException{
		String dev = "/dev/video0";
		if(args.length==1)
			dev = args[0];
		MemLeak m = new MemLeak(dev);
		Thread t = new Thread(m,"CaptureThread");
		t.start();
		System.in.read();
		m.stop();
		t.join();
		m.release();
	}
	
}
