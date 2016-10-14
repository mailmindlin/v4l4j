package au.edu.jcu.v4l4j.encoder;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.BlockingQueue;

import au.edu.jcu.v4l4j.BaseVideoFrame;
import au.edu.jcu.v4l4j.ImagePalette;
import au.edu.jcu.v4l4j.VideoFrame;
import au.edu.jcu.v4l4j.exceptions.JNIException;
import au.edu.jcu.v4l4j.exceptions.StateException;
import au.edu.jcu.v4l4j.exceptions.V4L4JException;

/**
 * A wrapper for the <code>libv4lconvert</code> library.
 * @author mailmindlin
 */
public class AbstractVideoFrameEncoder implements VideoFrameEncoder {
	/**
	 * Type that this encoder will convert from
	 */
	protected final ImagePalette typeFrom;
	/**
	 * Type that this encoder will convert frames to
	 */
	protected final ImagePalette typeTo;
	/**
	 * Width of frames
	 */
	protected final int width;
	/**
	 * Height of frames
	 */
	protected final int height;
	/**
	 * 
	 */
	protected int numVideoFrames;
	protected final List<AbstractConvertedVideoFrame> frames;
	protected final BlockingQueue<AbstractConvertedVideoFrame> availableFrames;
	
	/**
	 * A pointer to the native struct in memory.
	 */
	protected final long object;

	static {
		try {
			System.loadLibrary("v4l4j");
		} catch (UnsatisfiedLinkError e) {
			System.err.println("Can't load v4l4j JNI library");
			throw e;
		}
	}
	
	/**
	 * Create the native struct
	 * @param from
	 * 	image type to convert from
	 * @param to
	 * 	image type to convert to
	 * @param width
	 * 	the frame width
	 * @param height
	 * 	the frame height
	 * @return a pointer to the allocated struct
	 * @throws JNIException if there was a problem with allocation or something
	 */
	private native long doInit(int from, int to, int width, int height) throws JNIException;
	/**
	 * Release native memory
	 * @param object
	 * 	a pointer to the struct
	 */
	private native void doRelease();
	protected native void setBufferCapacity(int buffer, int capacity);
	protected native int getBufferCapacity(int buffer);
	protected native int getBufferLimit(int buffer);
	/**
	 * Put data into the encoder's buffer
	 * @param array data to put
	 * @param length length of data
	 * @throws V4L4JException
	 */
	protected native void putBuffer(byte[] array, int length) throws V4L4JException;
	/**
	 * Get the data from the encoder's buffer
	 * @param array
	 * @return length of read data
	 * @throws V4L4JException
	 */
	protected native int getBuffer(byte[] array) throws V4L4JException;
	
	/**
	 * Set JPEG image quality
	 * @param quality
	 */
	protected native void setQuality(int quality);
	
	protected native void doConvert();
	
	/**
	 * Get the ids of the converters used internally by libv4lconvert.
	 * <p>
	 * I don't see how anyone would need to use this method for anything,
	 * because the actual IDs of the converters may be subject to change,
	 * but it's here for debugging.
	 * </p>
	 * @return IDs of converters used
	 */
	public native int[] getConverterIds();
	
	protected AbstractVideoFrameEncoder(int width, int height, ImagePalette from, ImagePalette to) {
		this.width = width;
		this.height = height;
		this.typeFrom = from;
		this.typeTo = to;
		
		this.numVideoFrames = (System.getProperty("v4l4j.num_encoder_buffers") != null) ? Integer.parseInt(System.getProperty("v4l4j.num_encoder_buffers")) : 1;
		
		this.frames = new ArrayList<>(numVideoFrames);
		this.availableFrames = new ArrayBlockingQueue<>(numVideoFrames);
		
		this.object = doInit(from.getIndex(), to.getIndex(), width, height);
		
		System.out.println("Built encoder from " + from + " to " + to + '.');
		System.out.println("\tIds " + Arrays.toString(getConverterIds()));
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
	
	@Override
	public VideoFrame encode(BaseVideoFrame frame) throws V4L4JException {
		AbstractConvertedVideoFrame outFrame = availableFrames.poll();
		
		this.putBuffer(frame.getBytes(), frame.getFrameLength());
		this.doConvert();
		int length = this.getBuffer(outFrame.getBytes());
		outFrame.prepareForDelivery(length, 0, frame.getSequenceNumber(), frame.getCaptureTime());
		return outFrame;
	}
	
	@Override
	public void close() throws Exception {
		doRelease();
	}
}
