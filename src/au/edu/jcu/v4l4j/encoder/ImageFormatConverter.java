package au.edu.jcu.v4l4j.encoder;

import java.nio.BufferOverflowException;
import java.nio.BufferUnderflowException;
import java.nio.ByteBuffer;

import au.edu.jcu.v4l4j.ImagePalette;

public class ImageFormatConverter implements VideoFrameEncoder {
	protected final long ptr;
	protected final int converterId;
	protected final ImagePalette inFormat;
	protected final int inWidth;
	protected final int inHeight;
	protected final int estimatedSrcLen;
	protected final ImagePalette outFormat;
	protected final int outWidth;
	protected final int outHeight;
	protected final int estimatedDstLen;
	
	protected static native int getConverterIDForTransformation(int srcFmt, int dstFmt);
	protected static int getConverterIDForTransformation(ImagePalette src, ImagePalette dst) {
		return getConverterIDForTransformation(src.getIndex(), dst.getIndex());
	}
	protected static native long initWithConverter(int converterId, int width, int height);
	private static native int[] getData(long ptr);
	
	protected ImageFormatConverter(long ptr) {
		this.ptr = ptr;
		int[] data = getData(ptr);
		this.converterId = data[0];
		this.inFormat = ImagePalette.lookup(data[1]);
		this.inWidth = data[2];
		this.inHeight = data[3];
		this.estimatedSrcLen = data[4];
		this.outFormat = ImagePalette.lookup(data[5]);
		this.outWidth = data[6];
		this.outHeight = data[7];
		this.estimatedDstLen = data[8];
	}
	
	public ImageFormatConverter(int converterId, int width, int height) {
		this(initWithConverter(converterId, width, height));
	}
	
	public ImageFormatConverter(ImagePalette src, ImagePalette dst, int width, int height) {
		this(getConverterIDForTransformation(src, dst), width, height);
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
	public native void close() throws Exception;
}
