package au.edu.jcu.v4l4j.encoder;

import java.util.Vector;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.BlockingQueue;

import au.edu.jcu.v4l4j.BaseVideoFrame;
import au.edu.jcu.v4l4j.ImagePalette;
import au.edu.jcu.v4l4j.VideoFrame;
import au.edu.jcu.v4l4j.exceptions.StateException;
import au.edu.jcu.v4l4j.exceptions.V4L4JException;

public class AbstractVideoFrameEncoder implements VideoFrameEncoder {
	protected final ImagePalette typeFrom;
	protected final ImagePalette typeTo;
	protected final int width;
	protected final int height;
	protected int numVideoFrames;
	protected final Vector<AbstractConvertedVideoFrame> frames;
	protected final BlockingQueue<AbstractConvertedVideoFrame> availableFrames;
	
	/**
	 * JNI returns a long (which is really a pointer) when a device is allocated
	 * for use. This field is read-only (!!!)
	 */
	protected final long object;

	static {
		try {
			System.loadLibrary("v4l4j");
		} catch (UnsatisfiedLinkError e) {
			System.err.println("Cant load v4l4j JNI library");
			throw e;
		}
	}
	
	/**
	 * Create native struct
	 * @param from
	 * @param to
	 * @param width
	 * @param height
	 * @return
	 */
	private native long doInit(int from, int to, int width, int height);
	/**
	 * Release native data
	 * @param object pointer to struct
	 */
	private native void doRelease(long object);
	protected native void setBufferCapacity(long o, int buffer, int capacity);
	protected native int getBufferCapacity(long o, int buffer);
	protected native int getBufferLimit(long o, int buffer);
	/**
	 * Put data into the encoder's buffer
	 * @param o the object
	 * @param array data to put
	 * @param length length of data
	 * @throws V4L4JException
	 */
	protected native void putBuffer(long o, byte[] array, int length) throws V4L4JException;
	/**
	 * Get the data from the encoder's buffer
	 * @param object
	 * @param array
	 * @return length of read data
	 * @throws V4L4JException
	 */
	protected native int getBuffer(long object, byte[] array) throws V4L4JException;
	
	/**
	 * Set JPEG image quality
	 * @param object
	 * @param quality
	 */
	protected native void setQuality(long object, int quality);
	
	protected native void doConvert(long object);
	
	/**
	 * 
	 * @param object
	 * @param out
	 * @return
	 */
	protected native int getConverterIds(long object, int[] out);
	
	protected AbstractVideoFrameEncoder(int width, int height, ImagePalette from, ImagePalette to) {
		this.width = width;
		this.height = height;
		this.typeFrom = from;
		this.typeTo = to;
		
		this.numVideoFrames = (System.getProperty("v4l4j.num_encoder_buffers") != null) ? Integer.parseInt(System.getProperty("v4l4j.num_encoder_buffers")) : 1;
		
		this.frames = new Vector<>(numVideoFrames);
		this.availableFrames = new ArrayBlockingQueue<>(numVideoFrames);
		
		this.object = doInit(from.getIndex(), to.getIndex(), width, height);
	}
	
	public void init() {
		
	}
	
	/**
	 * Called for implementing class to fill {@link #frames} with the specified
	 * number of type-specific {@link AbstractConvertedVideoFrame}.
	 * 
	 * @param buffers number of buffers requested
	 */
	protected void createBuffers(int buffers) {
		
	}
	
	protected AbstractConvertedVideoFrame getAvailableVideoFrame() {
		try {
			return availableFrames.take();
		} catch (InterruptedException e) {
			throw new StateException("Interrupted while trying to get a videoFrame", e);
		}
	}
	
	public void release() {
		doRelease(object);
	}
	
	@Override
	public VideoFrame encode(BaseVideoFrame frame) throws V4L4JException {
		AbstractConvertedVideoFrame outFrame = availableFrames.poll();
		
		this.putBuffer(this.object, frame.getBytes(), frame.getFrameLength());
		this.doConvert(this.object);
		int length = this.getBuffer(this.object, outFrame.getBytes());
		outFrame.prepareForDelivery(length, 0, frame.getSequenceNumber(), frame.getCaptureTime());
		return outFrame;
	}
}
