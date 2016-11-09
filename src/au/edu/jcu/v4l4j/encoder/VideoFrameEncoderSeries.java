package au.edu.jcu.v4l4j.encoder;

import java.nio.ByteBuffer;
import java.util.Arrays;

import au.edu.jcu.v4l4j.ImagePalette;
import au.edu.jcu.v4l4j.exceptions.BufferOverflowException;
import au.edu.jcu.v4l4j.exceptions.BufferUnderflowException;
import au.edu.jcu.v4l4j.exceptions.JNIException;
import au.edu.jcu.v4l4j.exceptions.V4L4JException;

/**
 * A wrapper for the <code>libv4lconvert</code> library.
 * 
 * @author mailmindlin
 */
public class VideoFrameEncoderSeries implements VideoFrameEncoder {
	protected VideoFrameEncoder[] encoders;
	/**
	 * A pointer to the native <code>struct v4lconvert_encoder_series</code> in
	 * memory.
	 */
	protected long object;
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
	private native long doInit(int from, int to, int width, int height) throws JNIException;
	
	protected native int getBufferLimit(int buffer);
	
	/**
	 * Put data into the encoder's buffer
	 * 
	 * @param array
	 *            data to put
	 * @param length
	 *            length of data
	 * @throws V4L4JException
	 */
	protected native void putBuffer(byte[] array, int length) throws V4L4JException;
	
	/**
	 * Get the data from the encoder's buffer
	 * 
	 * @param array
	 * @return length of read data
	 * @throws V4L4JException
	 */
	protected native int getBuffer(byte[] array) throws V4L4JException;
	
	/**
	 * Set JPEG image quality
	 * 
	 * @param quality
	 */
	protected native void setQuality(int quality);
	
	protected native void doConvert();
	
	/**
	 * Get the ids of the converters used internally by libv4lconvert.
	 * <p>
	 * I don't see how anyone would need to use this method for anything,
	 * because the actual IDs of the converters may be subject to change, but
	 * it's here for debugging.
	 * </p>
	 * 
	 * @return IDs of converters used
	 */
	public native int[] getConverterIds();
	
	public VideoFrameEncoderSeries(VideoFrameEncoder...encoders) {
		if (encoders == null || encoders.length < 1)
			throw new NullPointerException();
		//validate encoders
		{
			VideoFrameEncoder encoder0 = encoders[0];
			int lastWidth = encoder0.getSourceWidth();
			int lastHeight = encoder0.getSourceHeight();;
			ImagePalette lastFmt = encoder0.getSourceFormat();
			for (int i = 0; i < encoders.length; i++) {
				VideoFrameEncoder encoder = this.encoders[i];
				if (encoder == null)
					throw new NullPointerException("Cannot apply null encoder (#" + i + ")");
				if (lastWidth != encoder.getSourceWidth() || lastHeight != encoder.getSourceHeight() || lastFmt != encoder.getSourceFormat())
					throw new IllegalArgumentException(String.format("Illegal conversion between encoders #%d to #%d (%S & %dx%d => %S @ %dx%d)", i, i + 1, lastFmt, lastWidth, lastHeight, encoder.getSourceFormat(), encoder.getSourceWidth(), encoder.getSourceHeight()));
				lastWidth = encoder.getDestinationWidth();
				lastHeight = encoder.getDestinationHeight();
				lastFmt = encoder.getDestinationFormat();
			}
		}
		//Copy the array, so changes to the argument array don't affect the state of the series
		this.encoders = new VideoFrameEncoder[encoders.length];
		System.arraycopy(encoders, 0, this.encoders, 0, encoders.length);
	}
	
	protected VideoFrameEncoderSeries(int width, int height, ImagePalette from, ImagePalette to) {
		this.object = doInit(from.getIndex(), to.getIndex(), width, height);
		
		System.out.println("Built encoder from " + from + " to " + to + '.');
		System.out.println("\tIds " + Arrays.toString(getConverterIds()));
	}
	
	@Override
	public native void close() throws Exception;
	
	@Override
	public int getConverterId() {
		return -1;
	}
	
	public int getLength() {
		return this.encoders.length;
	}
	
	public VideoFrameEncoder getEncoder(int index) {
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
		// TODO Auto-generated method stub
		return -1;
	}
	
	public V4lconvertBuffer createBuffer() {
		//TODO finish
		return null;
	}
	
	@Override
	public int apply(ByteBuffer src, ByteBuffer dst)
			throws BufferUnderflowException, BufferOverflowException, IllegalArgumentException {
		throw new UnsupportedOperationException();
	}
}
