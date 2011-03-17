package au.edu.jcu.v4l4j.examples;

import au.edu.jcu.v4l4j.FrameGrabber;
import au.edu.jcu.v4l4j.PushSourceCallback;
import au.edu.jcu.v4l4j.V4L4JConstants;
import au.edu.jcu.v4l4j.VideoDevice;
import au.edu.jcu.v4l4j.VideoFrame;
import au.edu.jcu.v4l4j.exceptions.V4L4JException;

/**
 * This class demonstrates how to perform a push-mode capture.
 * It simply starts a 10-second capture and prints the number of frames it
 * received during these 100 seconds.
 * @author gilles
 *
 */
public class PushModeCaptureApp  implements PushSourceCallback{
	private int				width, height;
	private String			device;
	private VideoDevice		videoDevice;
	private FrameGrabber	frameGrabber;
	private int				numFrames;


	/**
	 * Start a new PushModeCaptureApp
	 * @param d the path to the device file to use
	 * @param w the desired capture width
	 * @param h the desired capture height
	 * @throws V4L4JException if there is a problem capturing from the given device
	 */
	public PushModeCaptureApp(String d, int w, int h) throws V4L4JException {
		device = d;
		width = w;
		height = h;
		numFrames = 0;
		initFrameGrabber();
	}

	private void initFrameGrabber() throws V4L4JException{
		videoDevice = new VideoDevice(device);
		frameGrabber = videoDevice.getJPEGFrameGrabber(width, height, V4L4JConstants.INPUT_TYPE_CAMERA, V4L4JConstants.STANDARD_WEBCAM, 80);
		width = frameGrabber.getWidth();
		height = frameGrabber.getHeight();
		frameGrabber.setPushSourceMode(this);
		frameGrabber.startCapture();
	}

	@Override
	public void exceptionReceived(V4L4JException e) {
		// This method is called by v4l4j if an exception
		// has occurred while waiting for a new frame to be ready.
		// The exception is available through
		// e.getCause()
	}

	@Override
	public synchronized void nextFrame(VideoFrame frame) {
		// This method is called when a new frame is ready.
		// Don't forget to recycle it when done dealing with the frame.
		// Here we just count how many frames we have received.
		numFrames++;
		frame.recycle();
	}
	
	public void stopCapture() {
		frameGrabber.stopCapture();
	}
	
	public int getNumFrameReceived() {
		return numFrames;
	}

	public static void main(String args[]) throws V4L4JException, InterruptedException{
		String dev = (System.getProperty("test.device") != null) ? System.getProperty("test.device") : "/dev/video0"; 
		int w = (System.getProperty("test.width")!=null) ? Integer.parseInt(System.getProperty("test.width")) : 640;
		int h = (System.getProperty("test.height")!=null) ? Integer.parseInt(System.getProperty("test.height")) : 480;
		

		PushModeCaptureApp app = new PushModeCaptureApp(dev, w, h);
		Thread.sleep(10 * 1000);
		app.stopCapture();
		System.out.println("We have received "+app.getNumFrameReceived()+" frames.");
	}
}