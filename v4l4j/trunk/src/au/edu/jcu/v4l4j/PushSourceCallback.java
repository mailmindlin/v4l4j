package au.edu.jcu.v4l4j;

import au.edu.jcu.v4l4j.exceptions.V4L4JException;

/**
 * Objects implementing this interface act as observers of a 
 * frame grabber in push mode. They receive a notification 
 * when a new captured frame becomes available or an exception
 * has occured while capturing. By implementing this interface
 * and passing it to v4l4j, new frames are delivered to your 
 * application as soon as they arrive.It is extremely important
 * that you minimise the amount of code and the time spent in
 * the {@link #nextFrame(VideoFrame)} method.<br>
 * Check the {@link FrameGrabber} page to find out how to start
 * a capture in push mode.
 * @author gilles
 *
 */
public interface PushSourceCallback {
	
	/**
	 * During a capture, this method is called by v4l4j to provide
	 * the latest video frame. It is important that you minimise
	 * the amount of code and processing done in this method in 
	 * order to maintain the appropriate frame rate. 
	 * @param frame the latest captured frame
	 */
	public void nextFrame(VideoFrame frame);
	
	/**
	 * During a capture, this method is called if an exception is
	 * raised. 
	 * @param e the exception that was raised during the capture.
	 */
	public void exceptionReceived(V4L4JException e);

}
