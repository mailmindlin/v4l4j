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


import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import au.edu.jcu.v4l4j.FrameGrabber;
import au.edu.jcu.v4l4j.VideoDevice;
import au.edu.jcu.v4l4j.exceptions.V4L4JException;

public class FrameGrabberThreadTest {
	public class TestThread implements Runnable{
		int type;
		
		public TestThread(int t){
			type = t;
		}
		
		public void run(){
			if(type==1)
				try {
					run1();
				} catch (Exception e) {
					e.printStackTrace();
				}
			else if (type==2)
				try {
					run2();
				} catch (Exception e) {
					e.printStackTrace();
				}
			else if (type==3)
				try {
					run3();
				} catch (Exception e) {
					e.printStackTrace();
				}
			else if (type==4)
				try {
					run4();
				} catch (Exception e) {
					e.printStackTrace();
				}
		}
		

		public void run1() throws V4L4JException{
			long t = System.currentTimeMillis();
			fg.startCapture();
			System.out.println("RUN1: Starting capture");
			while(System.currentTimeMillis() - t < 10000) {
				fg.getVideoFrame().recycle();
			}
			System.out.println("RUN1: Done capturing - Stopping capture");
			fg.stopCapture();
			//we shouldnt get to here - stopCapture should fail
			System.out.println("RUN1: ************WE SHOULDNT BE HERE*********- exiting");
		}
		public void run2() throws InterruptedException{
			Thread.sleep(5000);
			System.out.println("RUN2: Running RELEASEFG");
			vd.releaseFrameGrabber();
			System.out.println("RUN2: exiting");
		}
		

		public void run3() throws V4L4JException{
			long t = System.currentTimeMillis();
			fg.startCapture();
			System.out.println("RUN3: Starting capture");
			while(System.currentTimeMillis() - t < 10000) {
				fg.getVideoFrame().recycle();
			}
			System.out.println("RUN3: Done capturing - stopping capture");
			fg.stopCapture();
			System.out.println("RUN3: RELEASEFG");
			vd.releaseFrameGrabber();
			System.out.println("RUN3: exiting");
		}
		public void run4() throws InterruptedException, V4L4JException{
			Thread.sleep(5000);
			System.out.println("RUN4: calling RELEASE");
			vd.release();
			System.out.println("RUN4: exiting");
		}
	}
	
	public VideoDevice vd;
	public FrameGrabber fg;
	
	public Thread t1, t2;
	public TestThread tt1, tt2;
	

	@Before
	public void setUp() throws Exception {
		int w,h, std, ch;
		String dev;
		dev = (System.getProperty("test.device")!=null) ? System.getProperty("test.device") : "/dev/video0"; 
		vd = new VideoDevice(dev);

		w = (System.getProperty("test.width")!=null) ? Integer.parseInt(System.getProperty("test.width")) : 320;
		h = (System.getProperty("test.height")!=null) ? Integer.parseInt(System.getProperty("test.height")) : 240;
		std = (System.getProperty("test.standard")!=null) ? Integer.parseInt(System.getProperty("test.standard")) : 0;
		ch = (System.getProperty("test.channel")!=null) ? Integer.parseInt(System.getProperty("test.channel")) : 0;
 
		fg = vd.getRawFrameGrabber(w, h, ch, std);
	}

	@After
	public void tearDown() throws Exception {
		vd.release();
	}
	
	@Test
	public void testConcurrentCapture() throws InterruptedException {
		tt1 = new TestThread(1);
		tt2 = new TestThread(2);
		t1 = new Thread(tt1, "tt1");
		t2 = new Thread(tt2, "tt2");
		t1.start();
		t2.start();
		t1.join();
		t2.join();
	}
	
	@Test
	public void testConcurrentCaptureNRelease() throws InterruptedException {
		tt1 = new TestThread(3);
		tt2 = new TestThread(4);
		t1 = new Thread(tt1, "tt1");
		t2 = new Thread(tt2, "tt2");
		t1.start();
		t2.start();
		t1.join();
		t2.join();
		
	}

}
