package au.edu.jcu.v4l4j.test;

import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import au.edu.jcu.v4l4j.FrameGrabber;
import au.edu.jcu.v4l4j.PushSourceCallback;
import au.edu.jcu.v4l4j.VideoDevice;
import au.edu.jcu.v4l4j.VideoFrame;
import au.edu.jcu.v4l4j.exceptions.StateException;
import au.edu.jcu.v4l4j.exceptions.UnsupportedMethod;
import au.edu.jcu.v4l4j.exceptions.V4L4JException;

public class PushSourceTest implements PushSourceCallback{
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

	@Test(expected=StateException.class)
	public void testSetCallbackAfterStartCapture() throws V4L4JException{
		fg.startCapture();

		try {
			fg.setPushSourceMode(new PushSourceCallback() {

				@Override
				public void nextFrame(VideoFrame frame) {
					fail("we shouldnt be here");
					frame.recycle();
				}

				@Override
				public void exceptionReceived(V4L4JException e) {
					e.printStackTrace();
					fail("Received exception when we were not expecting one");
				}
			});
		} finally {
			fg.stopCapture();
		}
	}

	@Test(expected=UnsupportedMethod.class)
	public void testGetVideoFrameWhileInPushMode() throws V4L4JException{
		numCapturedFrames = 0;
		fg.setPushSourceMode(new PushSourceCallback() {

			@Override
			public void nextFrame(VideoFrame frame) {
				frame.recycle();
			}

			@Override
			public void exceptionReceived(V4L4JException e) {
				e.printStackTrace();
				fail("Received exception when we were not expecting one");
			}
		});

		fg.startCapture();

		try {
			fg.getVideoFrame().recycle();
			fail("We shouldnt be here");
		} finally {
			fg.stopCapture();
		}
	}
	
	public synchronized void Log(String s){
		System.err.println(Thread.currentThread().getName()+": "+ s);
		System.err.flush();
	}

	@Test
	public void testCapture() throws Exception{
		numCapturedFrames = 0;
		fg.setPushSourceMode(new PushSourceCallback() {

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
		fg.setPushSourceMode(this);

		synchronized (vd){
			fg.startCapture();
			vd.wait(5000);	//wait up to 5 seconds for a frame to arrive
		}

		assertTrue(numCapturedFrames > 0);

		// This one should throw a StateException as the capture
		// has been stopped in the callback
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
