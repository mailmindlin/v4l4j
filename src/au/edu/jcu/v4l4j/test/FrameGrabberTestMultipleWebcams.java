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

import static org.junit.Assert.fail;

import java.util.List;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import au.edu.jcu.v4l4j.FrameGrabber;
import au.edu.jcu.v4l4j.Control;
import au.edu.jcu.v4l4j.exceptions.V4L4JException;

public class FrameGrabberTestMultipleWebcams {
	private FrameGrabber fg1;
	private FrameGrabber fg2;
	private int initialQuality = 80;

	@Before
	public void setUp() throws Exception {
		try {
			fg1 = new FrameGrabber("/dev/video0", 320, 240, 0, 0, initialQuality);
			fg2 = new FrameGrabber("/dev/video1", 320, 240, 0, 0, initialQuality);
		} catch (V4L4JException e) {
			fail("Cant create a framegrabbers, make sure two webcams are connected and the drivers are loaded.");
			throw e;
		}
		
		try{
			fg1.init();
			fg2.init();
		} catch (V4L4JException e) {
			fail("Cant init the framegrabbers, make sure two webcams are connected and the drivers are loaded.");
			throw e;
		}
	}

	@After
	public void tearDown() throws Exception {
		fg1.remove();
		fg2.remove();
	}
	
	@Test
	public void testDoNothing() {
	}

	@Test
	public void testStartCapture() {
		try {
			fg1.startCapture();
		} catch (V4L4JException e) {
			fail("Failed to start capture on fg1");
		}
		
		try {
			fg2.startCapture();
		} catch (V4L4JException e) {
			fail("Failed to start capture");
		}
		

		fg1.stopCapture();

		fg2.stopCapture();

	}

	@Test
	public void testGetFrame(){
		try {
			fg1.startCapture();
			fg2.startCapture();
			fg1.getFrame();
			fg2.getFrame();
			fg2.getFrame();
			fg2.getFrame();
			fg1.getFrame();
			fg2.stopCapture();
			fg1.stopCapture();
		} catch (V4L4JException e) {
			fail();
		}
	}
	
	@Test
	public void testListControls(){
		List<Control> v1 = fg1.getControls(), v2 = fg2.getControls();
		System.out.println("Listing controls:");
		for(Control c: v1)
			System.out.println("Control "+c.getName());
		for(Control c: v2)
			System.out.println("Control "+c.getName());
	}
	
	@Test
	public void testMultipleOpen() throws V4L4JException {
		fg1.remove();
		fg1.init();
		fg1.startCapture();
		fg2.remove();
		fg1.remove();
		fg1.init();
		fg2.init();
		fg1.remove();
		fg1.init();
		fg2.remove();
		fg1.remove();
		fg1.init();
		fg2.init();
		fg1.remove();
		fg1.init();
		fg1.remove();
		fg1.init();
		fg1.startCapture();
		fg2.remove();
		fg1.remove();
		fg1.init();
		fg2.init();
		fg1.remove();
		fg1.init();
		fg2.remove();
		fg1.remove();
		fg1.init();
		fg2.init();
		fg1.remove();
		fg1.init();
	}

}
