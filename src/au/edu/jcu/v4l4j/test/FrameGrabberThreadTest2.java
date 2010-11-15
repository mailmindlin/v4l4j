package au.edu.jcu.v4l4j.test;

import java.util.Random;
import java.util.Vector;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import au.edu.jcu.v4l4j.FrameGrabber;
import au.edu.jcu.v4l4j.VideoDevice;
import au.edu.jcu.v4l4j.VideoFrame;
import au.edu.jcu.v4l4j.exceptions.StateException;
import au.edu.jcu.v4l4j.exceptions.V4L4JException;

public class FrameGrabberThreadTest2 {	
	private VideoDevice vd;
	private FrameGrabber fg;
	private int w,h, std, ch;
	private String dev;
	private Vector<VideoFrame> frames;
	private int captureLength = 40 * 1000;
	

	@Before
	public void setUp() throws Exception {
		dev = (System.getProperty("test.device")!=null) ? System.getProperty("test.device") : "/dev/video0"; 
		vd = new VideoDevice(dev);

		w = (System.getProperty("test.width")!=null) ? Integer.parseInt(System.getProperty("test.width")) : 320;
		h = (System.getProperty("test.height")!=null) ? Integer.parseInt(System.getProperty("test.height")) : 240;
		std = (System.getProperty("test.standard")!=null) ? Integer.parseInt(System.getProperty("test.standard")) : 0;
		ch = (System.getProperty("test.channel")!=null) ? Integer.parseInt(System.getProperty("test.channel")) : 0;
 
		fg = vd.getRawFrameGrabber(w, h, ch, std);
		frames = new Vector<VideoFrame>();
	}

	@After
	public void tearDown() throws Exception {
		vd.releaseFrameGrabber();
		vd.release();
	}
	
	@Test
	public void CaptureNFreeFromDifferentThreads() throws Exception{
		Thread capture = new Thread(new TimedCapture(), "Capture thread");
		Thread release = new Thread(new FreeFrames(30), "Free thread   ");
		
		fg.startCapture();
		
		capture.start();
		release.start();
		
		capture.join();
		release.interrupt();
		release.join();
		
		fg.stopCapture();
	}
	
	@Test
	public void TwoCaptureThreadsN1FreeThread() throws Exception{
		Thread capture1 = new Thread(new TimedCapture(), "Capture thread1");
		Thread capture2 = new Thread(new TimedCapture(), "Capture thread2");
		Thread release = new Thread(new FreeFrames(10), "Free thread    ");
		
		fg.startCapture();
		
		capture1.start();
		capture2.start();
		release.start();
		
		capture1.join();
		capture2.join();
		release.interrupt();
		release.join();
		
		fg.stopCapture();
	}
	
	@Test
	public void OneCaptureThreadsN2FreeThread() throws Exception{
		Thread capture= new Thread(new TimedCapture(), "Capture thread");
		Thread release1 = new Thread(new FreeFrames(10), "Free thread1  ");
		Thread release2 = new Thread(new FreeFrames(10), "Free thread2  ");
		
		fg.startCapture();
		
		capture.start();
		release1.start();
		release2.start();
		
		capture.join();
		release1.interrupt();
		release2.interrupt();
		release1.join();
		release2.join();
		
		fg.stopCapture();
	}

	@Test(expected=StateException.class)
	public void test2CaptureThreads1Free1Release()throws Exception	{
		Thread capture1 = new Thread(new TimedCapture(), "Capture thread1");
		Thread capture2 = new Thread(new TimedCapture(), "Capture thread2");
		Thread free = new Thread(new FreeFrames(10), "Free thread    ");
		Thread release = new Thread(new ReleaseFG(5*1000), "Release thread ");
		
		fg.startCapture();
		
		capture1.start();
		capture2.start();
		free.start();
		release.start();
		
		capture1.join();
		capture2.join();
		free.interrupt();
		release.interrupt();
		free.join();
		release.join();
		
		fg.stopCapture();		
	}
	
	private synchronized void Log(String s) {
		System.out.println(Thread.currentThread().getName()+": "+s);
		System.out.flush();
	}
	
	
	private class TimedCapture implements Runnable {
			
		public void doTest() throws V4L4JException {
			VideoFrame frame;
			long t = System.currentTimeMillis();

			while(System.currentTimeMillis() - t < captureLength) {
				Log("Waiting for frame");
				frame = fg.getVideoFrame();
				synchronized(frames){
					frames.add(frame);
				}
				Log("Received frame");
			}
		}
		
		@Override
		public void run() {
			Log("Entering");
			try {
				doTest();
			} catch (V4L4JException e) {
				Log("Caught exception:");
				e.printStackTrace();
			}
			Log("Exiting");
		}		
	}
	
	
	private class FreeFrames implements Runnable {
		private int delay;
		private Random random;
		
		public FreeFrames(int delayMSBetweenFree) {
			delay = delayMSBetweenFree;
			random = new Random();
		}

		public void doTest() throws V4L4JException {
			try {
				while(! Thread.interrupted()){
					Thread.sleep(delay + random.nextInt(delay));
					
					synchronized(frames){
						if (frames.size() > 0) {
							Log("Freeing frame");
							frames.remove(0).recycle();
						}
					}
				}
			} catch (InterruptedException e) {}
		}
		
		@Override
		public void run() {
			Log("Entering");
			try {
				doTest();
			} catch (V4L4JException e) {
				Log("Caught exception:");
				e.printStackTrace();
			}
			Log("Exiting");
		}	
	}
	

	private class ReleaseFG implements Runnable {
		private int delay;
		
		public ReleaseFG(int delayBeforeRelease) {
			delay = delayBeforeRelease;
		}

		public void doTest() throws V4L4JException {
			try {
				Thread.sleep(delay);
				Log("Releasing frame grabber");
				vd.releaseFrameGrabber();
			} catch (InterruptedException e) {}
		}
		
		@Override
		public void run() {
			Log("Entering");
			try {
				doTest();
			} catch (V4L4JException e) {
				Log("Caught exception:");
				e.printStackTrace();
			}
			Log("Exiting");
		}
		
	}


}
