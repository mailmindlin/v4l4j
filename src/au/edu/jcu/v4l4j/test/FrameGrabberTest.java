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

import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ExecutionException;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import au.edu.jcu.v4l4j.CaptureCallback;
import au.edu.jcu.v4l4j.FrameGrabber;
import au.edu.jcu.v4l4j.VideoDevice;
import au.edu.jcu.v4l4j.VideoFrame;
import au.edu.jcu.v4l4j.exceptions.StateException;
import au.edu.jcu.v4l4j.exceptions.V4L4JException;

public class FrameGrabberTest implements CaptureCallback {
	private static VideoDevice vd;
	private FrameGrabber fg;
	int w, h, std, ch, repeats;
	String dev;
	volatile VideoFrame lastFrame;

	@Before
	public void setUp() throws Exception {
		dev = System.getProperty("test.device", "/dev/video0");
		vd = new VideoDevice(dev);
		w = Integer.getInteger("test.width", 320);
		h = Integer.getInteger("test.height", 240);
		std = Integer.getInteger("test.standard", 0);
		ch = Integer.getInteger("test.channel", 0);

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

	@Test
	public void testStartCaptureWithoutSetCallback() throws V4L4JException {
		try {
			fg.startCapture();
			fail("Error: we shouldn't be here");
			fg.stopCapture();
		} catch (V4L4JException e) {
			//Expected
		}
	}

	@Test
	public void testGetHeight() {
		assertTrue(fg.getHeight() > 0);
	}

	@Test
	public void testGetWidth() {
		assertTrue(fg.getWidth() > 0);
	}

	@Test
	public void testDoubleStartCapture() throws V4L4JException {
		fg.setCaptureCallback(this);
		try {
			fg.startCapture();
		} catch (V4L4JException e) {
			e.printStackTrace();
			fail("Error: Should be able to start the capture here");
		}
		
		try {
			fg.startCapture();
			fail("Error we shouldn't be here");
		} catch (StateException e) {
			//Expected
		}
	}
	
	@Test
	public void testDoubleStopCapture() throws StateException {
		fg.setCaptureCallback(this);
		try {
			fg.startCapture();
		} catch (V4L4JException e) {
			fail("Error: Should be able to start the capture here");
		}
		
		//Stop capture the first time
		fg.stopCapture();

		try {
			fg.stopCapture();
			fail("Error we shouldn't be here");
		} catch (StateException e) {
			//Expected
		}
	}

	@Test
	public void testAccessVideoFrameAfterStopCapture() throws Exception {
		fg.setCaptureCallback(this);
		try {
			fg.startCapture();
			Thread.sleep(2000);
			fg.stopCapture();
		} catch (V4L4JException e) {
			fail("Error: Should be able to start the capture here");
		}
		
		if (lastFrame == null)
			fail("Error: no frame was captured");
		
		try {
			lastFrame.getBytes(); // this should throw a StateException
			fail("We were able to access recycled frame data");
		} catch (StateException e) {
			//Exptected
		}
	}

	@Test
	public void testMultipleCapture() {
		fg.setCaptureCallback(this);
		try {
			fg.startCapture();
			fg.stopCapture();
			fg.startCapture();
			fg.stopCapture();
			fg.startCapture();
			fg.stopCapture();
		} catch (V4L4JException e) {
			fail("Error: Shouldn't be in exception handler here...");
		}
	}

	@Test
	public void testMultipleInitRelease() {
		try {
			int iteration = repeats;
			fg.setCaptureCallback(this);

			while (iteration-- > 0) {
				vd.releaseFrameGrabber();
				vd.release();

				vd = new VideoDevice(dev);
				fg = vd.getRawFrameGrabber(w, h, ch, std);
			}

		} catch (V4L4JException e) {
			fail("Error: Shouldn't be in exception handler here...");
		}

	}

	@Test
	public void testReleaseWithoutStopCapture() {
		try {
			fg.setCaptureCallback(this);
			fg.startCapture();
		} catch (V4L4JException e) {
			fail("Error: Shouldn't be in exception handler here...");
		}
	}
	
	@Test
	public void testDoubleFrameRecycle() throws V4L4JException, InterruptedException, ExecutionException {
		CompletableFuture<String> result = new CompletableFuture<>();
		
		fg.setCaptureCallback(frame->{
			frame.recycle();
			try {
				frame.recycle();
				result.complete("Error: second recycle() returned without throwing an exception");
			} catch (StateException e) {
				//Expected
				result.complete(null);
			}
		}, err -> {
			result.obtrudeException(err);
		});
		
		fg.startCapture();
		String failureMsg = result.get();
		fg.stopCapture();
		
		if (failureMsg != null)
			fail(failureMsg);
	}

	public void nextFrame(VideoFrame frame) {
		lastFrame = frame;
		frame.recycle();
	}

	public void exceptionReceived(V4L4JException e) {
		fail("Error we shouldn't be here");
	}
}
