package au.edu.jcu.v4l4j.impl.v4l;

import java.nio.ByteBuffer;
import java.util.concurrent.ThreadFactory;
import java.util.concurrent.atomic.AtomicInteger;

import au.edu.jcu.v4l4j.api.Rational;
import au.edu.jcu.v4l4j.exceptions.JNIException;

public class FrameGrabber implements Runnable {
	private static final int STATE_STOPPED = 0;
	private static final int STATE_RUNNING = 1;
	private static final int STATE_REQUEST_STOP = 2;
	private static final int STATE_DEAD = 3;
	/**
	 * Initializes framegrabber
	 * @param pointer
	 *     Pointer to <code>struct video_device</code>
	 * @param width
	 *     Width 
	 */
	private static native int doInit(long pointer, int width, int height, int channel, int std, int numBuffers) throws JNIException;
	
	/**
	 * Start capture on framegrabber
	 * (Wrapper for <code>start_capture</code>)
	 * @param pointer
	 *     Pointer to capture_device
	 */
	private static native void doStartCapture(long pointer) throws JNIException;
	/**
	 * Stop capture on framegrabber
	 * (Wrapper for <code>stop_capture</code>)
	 * @param pointer
	 *     Pointer to capture_device
	 */
	private static native void doStopCapture(long pointer) throws JNIException;
	/**
	 * Get the current set frame interval for the FrameGrabber.
	 * Rational may not be reduced
	 */
	private static native Rational doGetFrameInterval(long pointer);
	/**
	 * Set frame interval
	 */
	private static native void doSetFrameInterval(long pointer, int numerator, int denominator);
	private static native void doSetVideoInputAndStandard(long pointer, int input, int standard);
	private static native int doGetVideoInput(long pointer);
	private static native int doGetVideoStandard(long pointer);
	private static native int doEnqueueBuffer(long pointer, int index);
	private static native int doFillBuffer(long pointer, ByteBuffer buffer);
	
	protected final VideoDevice device;
	protected long object;
	protected ThreadFactory threadFactory;
	protected Thread thread;
	private final AtomicInteger state = new AtomicInteger(FrameGrabber.STATE_STOPPED);

	
	protected FrameGrabber(VideoDevice device) {
		this.device = device;
		this.object = device.pointer;
	}
	
	public void init() {
		
	}
	
	public void start() {
		if (!state.compareAndSet(FrameGrabber.STATE_STOPPED, FrameGrabber.STATE_RUNNING))
			throw new IllegalStateException();
		this.thread = new Thread(this);
		this.thread.setName(this.thread.getName() + " - v4l4j FrameGrabber");
		this.thread.start();
	}
	
	public void stop() {
		
	}
	
	public void release() {
	
	}

	@Override
	public void run() {
		while (state.get() == FrameGrabber.STATE_RUNNING) {
			
		}
	}
}
