/*
* Copyright (C) 2011 Gilles Gigan (gilles.gigan@gmail.com)
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

import au.edu.jcu.v4l4j.FrameGrabber;
import au.edu.jcu.v4l4j.CaptureCallback;
import au.edu.jcu.v4l4j.VideoDevice;
import au.edu.jcu.v4l4j.VideoFrame;
import au.edu.jcu.v4l4j.exceptions.StateException;
import au.edu.jcu.v4l4j.exceptions.UnsupportedMethod;
import au.edu.jcu.v4l4j.exceptions.V4L4JException;

public class PushSourceTest implements CaptureCallback{
	private VideoDevice vd;
	private FrameGrabber fg;
	private int w,h, std, ch;
	private String dev;
	private int numCapturedFrames;
	private boolean hasSeenFrame0;

	@Before
	public void setUp() throws Exception {
		dev = (System.getProperty("test.device")!=null) ? System.getProperty("test.device") : "/dev/video0"; 
		vd = new VideoDevice(dev);

		w = (System.getProperty("test.width")!=null) ? Integer.parseInt(System.getProperty("test.width")) : 320;
		h = (System.getProperty("test.height")!=null) ? Integer.parseInt(System.getProperty("test.height")) : 240;
		std = (System.getProperty("test.standard")!=null) ? Integer.parseInt(System.getProperty("test.standard")) : 0;
		ch = (System.getProperty("test.channel")!=null) ? Integer.parseInt(System.getProperty("test.channel")) : 0;

		fg = vd.getRawFrameGrabber(w, h, ch, std);
		numCapturedFrames = 0;
		hasSeenFrame0 = false;
	}

	@After
	public void tearDown() throws Exception {
		vd.releaseFrameGrabber();
		vd.release();
	}

	public synchronized void Log(String s){
		System.err.println(Thread.currentThread().getName()+": "+ s);
		System.err.flush();
	}

	@Test
	public void testCapture() throws Exception{
		numCapturedFrames = 0;
		fg.setCaptureCallback(new CaptureCallback() {

			@Override
			public void nextFrame(VideoFrame frame) {
				if (frame.getSequenceNumber() == 0)
					hasSeenFrame0 = true;
				
				numCapturedFrames++;
				frame.recycle();

				synchronized(vd){
					vd.notifyAll();
				}
			}

			@Override
			public void exceptionReceived(V4L4JException e) {
				e.printStackTrace();
				fail("Received exception when we were not expecting one");
			}
		});

		synchronized (vd){
			fg.startCapture();
			vd.wait(5000);	//wait up to 5 seconds for a frame to arrive
		}

		assertTrue(hasSeenFrame0);
		assertTrue(numCapturedFrames > 0);
		fg.stopCapture();
	}

	@Test(expected=StateException.class)
	public void testStopCaptureFromCallbackThread() throws Exception{
		fg.setCaptureCallback(this);

		synchronized (vd){
			fg.startCapture();
			vd.wait(5000);	//wait up to 5 seconds for a frame to arrive
		}

		assertTrue(numCapturedFrames > 0);

		// This one should throw a StateException as the capture
		// has been stopped in the callback
		fg.stopCapture();
	}
	
	@Test
	public void testNumberOfRecycledFramesWhileInCapture() throws V4L4JException, InterruptedException{
		final int numberOfVideoFrames = fg.getNumberOfVideoFrames();
		numCapturedFrames = 0;
		
		fg.setCaptureCallback(new CaptureCallback() {

			@Override
			public void nextFrame(VideoFrame frame) {
				numCapturedFrames++;
				assertTrue(fg.getNumberOfRecycledVideoFrames() + numCapturedFrames == numberOfVideoFrames);

				if (fg.getNumberOfRecycledVideoFrames() == 0){
					synchronized(vd){
						vd.notifyAll();
					}
				}
			}

			@Override
			public void exceptionReceived(V4L4JException e) {
				e.printStackTrace();
				fail("Received exception when we were not expecting one");
			}
		});

		synchronized (vd){
			fg.startCapture();
			vd.wait(5000);	//wait up to 5 seconds for a frame to arrive
		}

		assertTrue(numCapturedFrames == fg.getNumberOfVideoFrames());

		fg.stopCapture();
	}
	
	@Test
	public void testMulitpleStartStop() throws V4L4JException, InterruptedException {
		fg.setCaptureCallback(new CaptureCallback() {
		
			@Override
			public void nextFrame(VideoFrame frame) {
				frame.recycle();
				numCapturedFrames++;
				// Capture 5 frames then interrupt main thread
				if (numCapturedFrames == 5 ) {
					synchronized(vd) {
						vd.notifyAll();
					}
				}
			}

			@Override
			public void exceptionReceived(V4L4JException e) {
				e.printStackTrace();
				fail("Received exception when we were not expecting one");
			}
		});


		numCapturedFrames = 0;
		synchronized(vd) {
			fg.startCapture();
			vd.wait(5000);
		}

		fg.stopCapture();

		numCapturedFrames = 0;
		synchronized(vd) {
			fg.startCapture();
			vd.wait(5000);
		}

		fg.stopCapture();

		numCapturedFrames = 0;
		synchronized(vd) {
			fg.startCapture();
			vd.wait(5000);
		}

		fg.stopCapture();

		numCapturedFrames = 0;
		synchronized(vd) {
			fg.startCapture();
			vd.wait(5000);
		}

		fg.stopCapture();

	}
	
	@Override
	public void nextFrame(VideoFrame frame) {
		numCapturedFrames++;
		frame.recycle();
		fg.stopCapture();

		synchronized(vd){
			vd.notifyAll();
		}
	}

	@Override
	public void exceptionReceived(V4L4JException e) {
		e.printStackTrace();
		fail("Received exception when we were not expecting one");
	}

}
