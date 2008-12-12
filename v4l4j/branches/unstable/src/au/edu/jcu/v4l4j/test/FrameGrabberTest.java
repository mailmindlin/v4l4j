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

import java.util.List;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import au.edu.jcu.v4l4j.FrameGrabber;
import au.edu.jcu.v4l4j.Control;
import au.edu.jcu.v4l4j.exceptions.V4L4JException;

public class FrameGrabberTest {
	private FrameGrabber fg;
	private int initialQuality = 80;

	@Before
	public void setUp() throws Exception {
		try {
			String dev;
			int w,h, std, ch;
			dev = (System.getProperty("test.device")!=null) ? System.getProperty("test.device") : "/dev/video0"; 
			w = (System.getProperty("test.width")!=null) ? Integer.parseInt(System.getProperty("test.width")) : 320;
			h = (System.getProperty("test.height")!=null) ? Integer.parseInt(System.getProperty("test.height")) : 240;
			std = (System.getProperty("test.standard")!=null) ? Integer.parseInt(System.getProperty("test.standard")) : 0;
			ch = (System.getProperty("test.channel")!=null) ? Integer.parseInt(System.getProperty("test.channel")) : 0;
			initialQuality = (System.getProperty("test.qty")!=null) ? Integer.parseInt(System.getProperty("test.qty")) : 80; 
			fg = new FrameGrabber(dev, w, h, ch, std, initialQuality);
		} catch (V4L4JException e) {
			fail("Cant create a framegrabber, make sure a webcam is connected and the driver is loaded.\n"+e.getMessage());
			throw e;
		}
		
		try{
			fg.init();
		} catch (V4L4JException e) {
			fail("Cant create a framegrabber, make sure a webcam is connected and the driver is loaded.\n"+e.getMessage());
			throw e;
		}
	}

	@After
	public void tearDown() throws Exception {
		fg.remove();
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
		
		try {
			fg.stopCapture();
		} catch (V4L4JException e) {
			fail("failed to stop capture");
		}
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

	@Test
	public void testSetJPGQuality() {
		assertTrue(fg.getJPGQuality()==initialQuality);
		try {
			fg.setJPGQuality(50);
		} catch (V4L4JException e) {
			fail("Error: Shouldnt be in exception handler here...");
		}
		assertTrue(fg.getJPGQuality()==50);
	}

	@Test
	public void testSetControlValue(){
		List<Control> v = fg.getControls();
		int val = (v.get(0).getMax() - v.get(0).getMin())/2+v.get(0).getMin();
		try {
			v.get(0).setValue(val);
			assertTrue(v.get(0).getValue()==val);
		} catch (V4L4JException e) {
			fail("Error: Shouldnt be in exception handler here...");
		}
	}
	
	@Test(expected=V4L4JException.class)
	public void testDoubleStartCapture() throws V4L4JException{
		try {
			fg.startCapture();
		} catch (V4L4JException e) {
			fail("Error: Should be able to start the capture here");
		}
		fg.startCapture();
	}
	
	@Test(expected=V4L4JException.class)
	public void testDoubleStopCapture() throws V4L4JException{
		try {
			fg.startCapture();
		} catch (V4L4JException e) {
			fail("Error: Should be able to start the capture here");
		}
		try {
			fg.stopCapture();
		} catch (V4L4JException e) {
			fail("Error: Should be able to stop the capture here");
		}
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
	
	@Test(expected=V4L4JException.class)
	public void testGetFrameWithoutStartCapture() throws V4L4JException {
		fg.getFrame();
	}

	@Test
	public void testMultipleOpen(){
		try {
			fg.remove();
			fg.init();
			fg.startCapture();
			fg.remove();
			fg.init();
			fg.remove();
			fg.init();
			fg.remove();
			fg.init();
			fg.remove();
			fg.init();
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
