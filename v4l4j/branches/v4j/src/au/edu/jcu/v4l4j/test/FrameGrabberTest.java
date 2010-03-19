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

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import au.edu.jcu.v4l4j.DeviceList;
import au.edu.jcu.v4l4j.FrameGrabber;
import au.edu.jcu.v4l4j.VideoDevice;
import au.edu.jcu.v4l4j.exceptions.StateException;
import au.edu.jcu.v4l4j.exceptions.V4L4JException;

public class FrameGrabberTest {
	private static VideoDevice vd;
	private FrameGrabber fg;

	
	
	@Before
	public void setUp() throws Exception {
		int w,h, std, ch;
		String dev;
		dev = (System.getProperty("test.device")!=null) ? System.getProperty("test.device") : "/dev/video0"; 
		vd = DeviceList.getVideoDeviceFromDeviceFile(dev);

		w = (System.getProperty("test.width")!=null) ? Integer.parseInt(System.getProperty("test.width")) : 320;
		h = (System.getProperty("test.height")!=null) ? Integer.parseInt(System.getProperty("test.height")) : 240;
		std = (System.getProperty("test.standard")!=null) ? Integer.parseInt(System.getProperty("test.standard")) : 0;
		ch = (System.getProperty("test.channel")!=null) ? Integer.parseInt(System.getProperty("test.channel")) : 0;
 
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

	@Test
	public void testStartCapture() {
		try {
			fg.startCapture();
		} catch (V4L4JException e) {
			fail("Failed to start capture");
		}
		
		fg.stopCapture();
	}

	@Test
	public void testGetFrame() {
		try {
			fg.startCapture();
			fg.getFrame();
			fg.stopCapture();

		} catch (V4L4JException e) {
			fail("Error: Shouldnt be in exception handler here...");
		}
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
		try {
			fg.startCapture();
		} catch (V4L4JException e) {
			fail("Error: Should be able to start the capture here");
		}
		fg.startCapture();
	}
	
	@Test(expected=StateException.class)
	public void testDoubleStopCapture() throws StateException{
		try {
			fg.startCapture();
		} catch (V4L4JException e) {
			fail("Error: Should be able to start the capture here");
		}
		fg.stopCapture();

		fg.stopCapture();
	}
	
	@Test
	public void testMultipleCapture(){
		try {
			fg.startCapture();
			fg.getFrame();
			fg.stopCapture();
			fg.startCapture();
			fg.getFrame();
			fg.stopCapture();
			fg.startCapture();
			fg.getFrame();
			fg.stopCapture();
		} catch (V4L4JException e) {
			fail("Error: Shouldnt be in exception handler here...");
		}
	}
	
	@Test(expected=StateException.class)
	public void testGetFrameWithoutStartCapture() throws V4L4JException {
		fg.getFrame();
	}


	@Test
	public void testMultipleInitRelease(){
		int w, h, std, ch;
		try {
			vd.releaseFrameGrabber();
			vd.release();

			w = (System.getProperty("test.width")!=null) ? Integer.parseInt(System.getProperty("test.width")) : 320;
			h = (System.getProperty("test.height")!=null) ? Integer.parseInt(System.getProperty("test.height")) : 240;
			std = (System.getProperty("test.standard")!=null) ? Integer.parseInt(System.getProperty("test.standard")) : 0;
			ch = (System.getProperty("test.channel")!=null) ? Integer.parseInt(System.getProperty("test.channel")) : 0;
			fg = vd.getRawFrameGrabber(w, h, ch, std);
			vd.releaseFrameGrabber();


			fg = vd.getRawFrameGrabber(w, h, ch, std);
			vd.releaseFrameGrabber();


			fg = vd.getRawFrameGrabber(w, h, ch, std);
			vd.releaseFrameGrabber();

			fg = vd.getRawFrameGrabber(w, h, ch, std);
			fg.startCapture();
			fg.getFrame();
			fg.getFrame();
			fg.getFrame();
			fg.stopCapture();
			vd.releaseFrameGrabber();


			fg = vd.getRawFrameGrabber(w, h, ch, std);
			fg.startCapture();
			fg.getFrame();
			fg.getFrame();
			fg.getFrame();
			fg.stopCapture();
			vd.releaseFrameGrabber();
		} catch (V4L4JException e) {
			fail("Error: Shouldnt be in exception handler here...");
		}

	}
	
	@Test
	public void testRemoveWithoutStopCapture() {
		try {
			fg.startCapture();
		} catch (V4L4JException e) {
			fail("Error: Shouldnt be in exception handler here...");
		}
	}
}
