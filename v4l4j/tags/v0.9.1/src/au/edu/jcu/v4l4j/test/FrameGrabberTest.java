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

import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

import java.util.Vector;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import au.edu.jcu.v4l4j.FrameGrabber;
import au.edu.jcu.v4l4j.VideoDevice;
import au.edu.jcu.v4l4j.VideoFrame;
import au.edu.jcu.v4l4j.CaptureCallback;
import au.edu.jcu.v4l4j.exceptions.StateException;
import au.edu.jcu.v4l4j.exceptions.V4L4JException;

public class FrameGrabberTest implements CaptureCallback {
	private static VideoDevice vd;
	private FrameGrabber fg;
	int w,h, std, ch, repeats;
	String dev;
	VideoFrame lastFrame;
	
	
	@Before
	public void setUp() throws Exception {
		dev = (System.getProperty("test.device")!=null) ? System.getProperty("test.device") : "/dev/video0"; 
		vd = new VideoDevice(dev);
		w = (System.getProperty("test.width")!=null) ? Integer.parseInt(System.getProperty("test.width")) : 320;
		h = (System.getProperty("test.height")!=null) ? Integer.parseInt(System.getProperty("test.height")) : 240;
		std = (System.getProperty("test.standard")!=null) ? Integer.parseInt(System.getProperty("test.standard")) : 0;
		ch = (System.getProperty("test.channel")!=null) ? Integer.parseInt(System.getProperty("test.channel")) : 0;
 
		repeats = 10;
		
		fg = vd.getRawFrameGrabber(w, h, ch, std);
	}

	@After
	public void tearDown() throws Exception {
		vd.releaseFrameGrabber();
		vd.release();
	}
	
	@Test
	public void testDoNothing() {
	}

	@Test(expected=V4L4JException.class)
	public void testStartCaptureWithoutSetCallback() throws V4L4JException{
		fg.startCapture();
		fail("Error: we shouldnt be here");	
		fg.stopCapture();
	}

	@Test
	public void testGetHeight() {
		assertTrue(fg.getHeight()>0);
	}

	@Test
	public void testGetWidth() {
		assertTrue(fg.getWidth()>0);
	}
	
	@Test(expected=StateException.class)
	public void testDoubleStartCapture() throws V4L4JException{
		fg.setCaptureCallback(this);
		try {
			fg.startCapture();
		} catch (V4L4JException e) {
			e.printStackTrace();
			fail("Error: Should be able to start the capture here");
		}
		fg.startCapture();
		fail("Error we shouldnt be here");
	}
	
	@Test(expected=StateException.class)
	public void testDoubleStopCapture() throws StateException{
		fg.setCaptureCallback(this);
		try {
			fg.startCapture();
		} catch (V4L4JException e) {
			fail("Error: Should be able to start the capture here");
		}
		fg.stopCapture();

		fg.stopCapture();
		fail("Error we shouldnt be here");
	}
		
	@Test(expected=StateException.class)
	public void testAccessVideoFrameAfterStopCapture() throws Exception{
		fg.setCaptureCallback(this);
		try {
			fg.startCapture();
			Thread.sleep(2000);
			fg.stopCapture();
		} catch (V4L4JException e) {
			fail("Error: Should be able to start the capture here");
		}
		lastFrame.getBytes();	// this should throw a StateException
	}
	
	@Test
	public void testMultipleCapture(){
		fg.setCaptureCallback(this);
		try {
			fg.startCapture();
			fg.stopCapture();
			fg.startCapture();
			fg.stopCapture();
			fg.startCapture();
			fg.stopCapture();
		} catch (V4L4JException e) {
			fail("Error: Shouldnt be in exception handler here...");
		}
	}
	
	@Test
	public void testMultipleInitRelease(){
		try {
			int iteration = repeats;
			fg.setCaptureCallback(this);

			while (iteration-- > 0){
				vd.releaseFrameGrabber();
				vd.release();
			
				vd = new VideoDevice(dev);
				fg = vd.getRawFrameGrabber(w, h, ch, std);
			}
			
		} catch (V4L4JException e) {
			fail("Error: Shouldnt be in exception handler here...");
		}

	}
	
	@Test
	public void testReleaseWithoutStopCapture() {
		try {
			fg.setCaptureCallback(this);
			fg.startCapture();
		} catch (V4L4JException e) {
			fail("Error: Shouldnt be in exception handler here...");
		}
	}

	public void nextFrame(VideoFrame frame) {
		lastFrame = frame;
		frame.recycle();
	}

        public void exceptionReceived(V4L4JException e) {
		fail("Error we shouldnt be here");
	}
}

