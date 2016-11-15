package au.edu.jcu.v4l4j.encoder;

import java.nio.ByteBuffer;

import au.edu.jcu.v4l4j.ImagePalette;
import au.edu.jcu.v4l4j.exceptions.BufferOverflowException;
import au.edu.jcu.v4l4j.exceptions.BufferUnderflowException;
import au.edu.jcu.v4l4j.exceptions.JNIException;

/**
 * A wrapper for the <code>libv4lconvert</code> library.
 * 
 * @author mailmindlin
 */
public class VideoFrameEncoderSeries implements VideoFrameTransformer {
	
	static {
		try {
			System.loadLibrary("v4l4j");
		} catch (UnsatisfiedLinkError e) {
			System.err.println("Can't load v4l4j JNI library");
			throw e;
		}
	}
	
	protected VideoFrameTransformer[] encoders;
	/**
	 * A pointer to the native <code>struct v4lconvert_encoder_series</code> in
	 * memory.
	 */
	protected long object;
	
	private static native long initWithEncoders(long[] encoders);
	/**
	 * Create the native struct
	 * 
	 * @param from
	 *            image type to convert from
	 * @param to
	 *            image type to convert to
	 * @param width
	 *            the frame width
	 * @param height
	 *            the frame height
	 * @return a pointer to the allocated struct
	 * @throws JNIException
	 *             if there was a problem with allocation or something
	 */
	private static native long doInit(int from, int to, int width, int height) throws JNIException;
	
	/**
	 * Apply series to buffer
	 * @param seriesPtr Pointer to native <code>v4lconvert_encoder_series</code> object
	 * @param bufPtr Pointer to native <code>v4lconvert_buffer</code> object
	 * @return Number of bytes written to output buffer
	 * @throws NullPointerException if the seriesPtr or bufPtr point to an invalid target
	 */
	private static native int doApply(long seriesPtr, long bufPtr);
	
	/**
	 * Get the pointers to the encoders for
	 * @param seriesObject
	 * @return
	 * @throws JNIException
	 */
	private static native long[] getEncoderPointers(long seriesObject) throws JNIException;
	
	public VideoFrameEncoderSeries(VideoFrameTransformer...encoders) {
		if (encoders == null || encoders.length < 1)
			throw new NullPointerException();
		//validate encoders
		VideoFrameTransformer encoder0 = encoders[0];
		int lastWidth = encoder0.getSourceWidth();
		int lastHeight = encoder0.getSourceHeight();;
		ImagePalette lastFmt = encoder0.getSourceFormat();
		long[] ptrs = new long[encoders.length];
		for (int i = 0; i < encoders.length; i++) {
			VideoFrameTransformer encoder = this.encoders[i];
			if (encoder == null)
				throw new NullPointerException("Cannot apply null encoder (#" + i + ")");
			if (lastWidth != encoder.getSourceWidth() || lastHeight != encoder.getSourceHeight() || lastFmt != encoder.getSourceFormat())
				throw new IllegalArgumentException(String.format("Illegal conversion between encoders #%d to #%d (%S & %dx%d => %S @ %dx%d)", i, i + 1, lastFmt, lastWidth, lastHeight, encoder.getSourceFormat(), encoder.getSourceWidth(), encoder.getSourceHeight()));
			if ((ptrs[i] = encoder.getPointer()) == 0)
				throw new IllegalArgumentException("No pointer available for encoder #" + i + ": " + encoder);
			lastWidth = encoder.getDestinationWidth();
			lastHeight = encoder.getDestinationHeight();
			lastFmt = encoder.getDestinationFormat();
		}
		this.object = VideoFrameEncoderSeries.initWithEncoders(ptrs);
		//Copy the array, so changes to the argument array don't affect the state of the series
		this.encoders = new VideoFrameTransformer[encoders.length];
		System.arraycopy(encoders, 0, this.encoders, 0, encoders.length);
	}
	
	protected VideoFrameEncoderSeries(int width, int height, ImagePalette from, ImagePalette to) {
		this.object = doInit(from.getIndex(), to.getIndex(), width, height);
		
		long[] converters = VideoFrameEncoderSeries.getEncoderPointers(this.object);
		this.encoders = new VideoFrameTransformer[converters.length];
		for (int i = 0; i < converters.length; i++)
			this.encoders[i] = VideoFrameConverter.wrap(converters[i]);
		System.out.println("Built encoder from " + from + " to " + to + '.');
	}
	
	@Override
	public native void close() throws Exception;
	
	@Override
	public int getConverterId() {
		return -1;
	}
	
	/**
	 * Get length of chain of converters
	 * @return length
	 */
	public int getLength() {
		return this.encoders.length;
	}
	
	/**
	 * Get converter by index
	 * @param index
	 * @return converter
	 */
	public VideoFrameTransformer getEncoder(int index) {
		return this.encoders[index];
	}
	
	@Override
	public ImagePalette getSourceFormat() {
		return this.encoders[0].getSourceFormat();
	}
	
	@Override
	public int getSourceWidth() {
		return this.encoders[0].getSourceWidth();
	}
	
	@Override
	public int getSourceHeight() {
		return this.encoders[0].getSourceHeight();
	}
	
	@Override
	public int estimateSourceLength() {
		return this.encoders[0].estimateSourceLength();
	}
	
	@Override
	public ImagePalette getDestinationFormat() {
		return this.encoders[this.encoders.length - 1].getDestinationFormat();
	}
	
	@Override
	public int getDestinationWidth() {
		return this.encoders[this.encoders.length - 1].getDestinationWidth();
	}
	
	@Override
	public int getDestinationHeight() {
		return this.encoders[this.encoders.length - 1].getDestinationHeight();
	}
	
	@Override
	public int estimateDestinationLength() {
		return this.encoders[this.encoders.length - 1].estimateDestinationLength();
	}
	
	
	@Override
	public int apply(V4lconvertBuffer buf) {
		return VideoFrameEncoderSeries.doApply(this.object, buf.getPointer());
	}
	
	public V4lconvertBuffer createBuffer() {
		//TODO finish
		return null;
	}
	
	@Override
	public int apply(ByteBuffer src, ByteBuffer dst) throws BufferUnderflowException, BufferOverflowException, IllegalArgumentException {
		//Only supporting V4lconvertBuffer's
		throw new UnsupportedOperationException();
	}
	
	@Override
	public long getPointer() {
		//No available v4lconvert_converter pointer (maybe will change in the future)
		return 0;
	}
}
