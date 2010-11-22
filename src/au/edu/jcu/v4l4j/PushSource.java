package au.edu.jcu.v4l4j;

import au.edu.jcu.v4l4j.exceptions.StateException;
import au.edu.jcu.v4l4j.exceptions.V4L4JException;


/**
 * PushSource instances create their own thread which polls
 * a frame grabber and notify the 
 * {@link PushSourceCallback} object given in the constructor
 * each time a new frame is available.
 * @author gilles
 *
 */
class PushSource implements Runnable {
	private PushSourceCallback 		callback;
	private AbstractGrabber			frameGrabber;
	private Thread					thread;
	
	private int						state;
	private static final int		STATE_STOPPED = 0;
	private static final int		STATE_RUNNING = 1;
	private static final int		STATE_ABOUT_TO_STOP = 2;
	
	/**
	 * This method builds a new <code>PushSource</code> instance
	 * which will obtain frames from the given frame grabber and
	 * pass them to the given callback object.
	 * @param grabber the {@link FrameGrabber} instance on which
	 * this push source will repeatedly call {@link FrameGrabber#getVideoFrame()}.
	 * @param callback an object implementing the {@link PushSourceCallback}
	 * interface to which the frames will be delivered through the 
	 * {@link PushSourceCallback#nextFrame(VideoFrame)}.
	 */
	public PushSource(AbstractGrabber grabber, PushSourceCallback callback) {
		if ((grabber == null) || (callback == null))
			throw new NullPointerException("the frame grabber and callback cannot be null");
		
		this.callback = callback;
		frameGrabber = grabber;
		thread = new Thread(this, "Frame pusher");
		state = STATE_STOPPED;
	}
	
	/**
	 * This method instructs this source to start the capture and to push
	 * to the captured frame to the  
	 * {@link PushSourceCallback}. 
	 * @throws StateException if the capture has already been started
	 */
	public synchronized final void startCapture() throws V4L4JException{
		if (state != STATE_STOPPED)
			throw new StateException("The capture has already been started");
		
		// Update our state and start the thread
		state = STATE_RUNNING;		
		thread.start();
	}
	
	/**
	 * This method instructs this source to stop frame delivery
	 * to the {@link PushSourceCallback} object.
	 * @throws StateException if the capture has already been stopped
	 */
	public final void stopCapture() {
		synchronized (this) {
			// make sure we are running
			if ((state == STATE_STOPPED) || (state == STATE_ABOUT_TO_STOP))
				//throw new StateException("The capture is about to stop");
				return;

			// update our state
			state = STATE_ABOUT_TO_STOP;
		}
		
		if (thread.isAlive()) {
			thread.interrupt();
			try {
				// wait for thread to exit
				if (! Thread.currentThread().equals(thread))
					thread.join();
				thread = null;
			} catch (InterruptedException e) {
				System.err.println("interrupted while waiting for frame pusher thread to complete");
				e.printStackTrace();
				throw new StateException("interrupted while waiting for frame pusher thread to complete", e);
			}
		}
	}
	
	/**
	 * This method return the thread ID of the thread used by this source
	 * @return the thread id of the thread used by this source
	 */
	public long getThreadId() {
		return thread.getId();
	}

	@Override
	public final void run() {
		while (! Thread.interrupted()){
			try {
				// Get the next frame and deliver it to the callback object
				callback.nextFrame(frameGrabber.getVideoFrame());
			} catch (Exception e) {
				// Received an exception. If we are in the middle of a capture (ie. it does not
				// happen as the result of the capture having been stopped or the frame 
				// grabber released), then pass it on to the callback object.
				//e.printStackTrace();
				try {
					if(frameGrabber.isStarted())
						callback.exceptionReceived(new V4L4JException("Exception received while grabbing next frame", e));
				} catch (Exception e2) {
					// either the frame grabber has been released or the callback raised
					// an exception. do nothing, just exit.
				}
				// and make this thread exit
				Thread.currentThread().interrupt();
			}
		}
		AbstractGrabber.Log("exiting");
		// update state
		state = STATE_STOPPED;
	}
}
