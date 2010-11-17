package au.edu.jcu.v4l4j.test;

import au.edu.jcu.v4l4j.FrameGrabber;
import au.edu.jcu.v4l4j.VideoDevice;

public class Test {

	public static void main(String args[]) throws Exception
	{
		VideoDevice vd = new VideoDevice("/dev/video0");
		FrameGrabber fg = vd.getJPEGFrameGrabber(640, 480, 0, 0, 80);
		fg.startCapture();
	
		Thread.sleep(100);
	
		fg.stopCapture();
		vd.releaseFrameGrabber();
		vd.release();

	}
}
