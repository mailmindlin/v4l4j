package au.edu.jcu.v4l4j.encoder;

import java.nio.BufferOverflowException;
import java.nio.BufferUnderflowException;
import java.nio.ByteBuffer;

import au.edu.jcu.v4l4j.ImagePalette;

public class ImageFormatConverter implements VideoFrameConverter {
	
	static {
		try {
			System.loadLibrary("v4l4j");
		} catch (UnsatisfiedLinkError e) {
			System.err.println("Cant load v4l4j JNI library");
			throw e;
		}
	}
	
	/**
	 * Pointer to native object
	 */
	protected final long object;
	/**
	 * ID of converter used
	 */
	protected final int converterId;
	/**
	 * Input frame format
	 */
	protected final ImagePalette inFormat;
	/**
	 * Input frame width
	 */
	protected final int inWidth;
	/**
	 * Input frame height
	 */
	protected final int inHeight;
	/**
	 * Estimated (upper bound) source buffer size
	 */
	protected final int estimatedSrcLen;
	/**
	 * Output frame format
	 */
	protected final ImagePalette outFormat;
	/**
	 * Output frame width
	 */
	protected final int outWidth;
	/**
	 * Output frame height
	 */
	protected final int outHeight;
	/**
	 * Estimated (upper bound) output buffer size
	 */
	protected final int estimatedDstLen;
	
	/**
	 * Lookup a <code>v4lconvert_converter_t</code>'s id for a given transformation
	 * @param srcFmt
	 * @param dstFmt
	 * @return
	 */
	protected static native int lookupConverterByConversion(int srcFmt, int dstFmt);
	protected static int lookupConverterByConversion(ImagePalette src, ImagePalette dst) {
		return lookupConverterByConversion(src.getIndex(), dst.getIndex());
	}
	protected static native long initWithConverter(int converterId, int width, int height);
	/**
	 * Returns an array of properties that can be accessed on the native struct. The array will contain the following values:
	 * <ol start=0>
	 * <li>Converter ID</li>
	 * <li>Input format</li>
	 * <li>Output format</li>
	 * <li>Estimated source length</li>
	 * <li>Estimated output length</li>
	 * <li>Input width</li>
	 * <li>Input height</li>
	 * <li>Output width</li>
	 * <li>Output height</li>
	 * </ol>
	 * @param ptr Pointer to native struct
	 * @return
	 */
	private static native int[] getData(long ptr);
	
	protected ImageFormatConverter(long ptr) {
		this.object = ptr;
		int[] data = getData(ptr);
		this.converterId = data[0];
		this.inFormat = ImagePalette.lookup(data[1]);
		this.outFormat = ImagePalette.lookup(data[2]);
		this.estimatedSrcLen = data[3];
		this.estimatedDstLen = data[4];

		this.inWidth = data[5];
		this.inHeight = data[6];
		this.outWidth = data[7];
		this.outHeight = data[8];
	}
	
	/**
	 * Create an ImageFormatConverter wrapping the given converter's id
	 * @param converterId
	 * @param width
	 * @param height
	 */
	public ImageFormatConverter(int converterId, int width, int height) {
		this(initWithConverter(converterId, width, height));
	}
	
	/**
	 * Create an ImageFormatConverter for a given transformation.
	 * @param src
	 * @param dst
	 * @param width
	 * @param height
	 */
	public ImageFormatConverter(ImagePalette src, ImagePalette dst, int width, int height) {
		this(lookupConverterByConversion(src, dst), width, height);
	}

	@Override
	public int getConverterId() {
		return this.converterId;
	}

	@Override
	public int getSourceWidth() {
		return this.inWidth;
	}

	@Override
	public int getSourceHeight() {
		return this.inHeight;
	}

	@Override
	public ImagePalette getSourceFormat() {
		return this.inFormat;
	}

	@Override
	public int getDestinationWidth() {
		return this.outWidth;
	}

	@Override
	public int getDestinationHeight() {
		return this.outHeight;
	}
	
	@Override
	public ImagePalette getDestinationFormat() {
		return this.outFormat;
	}

	@Override
	public native int apply(ByteBuffer src, ByteBuffer dst) throws BufferUnderflowException,
			BufferOverflowException, IllegalArgumentException;

	@Override
	public int estimateSourceLength() {
		return this.estimatedSrcLen;
	}

	@Override
	public int estimateDestinationLength() {
		return this.estimatedDstLen;
	}
	
	@Override
	public long getPointer() {
		return this.object;
	}

	@Override
	public native void close() throws Exception;
}
