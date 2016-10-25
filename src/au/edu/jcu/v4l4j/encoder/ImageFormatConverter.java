package au.edu.jcu.v4l4j.encoder;

import java.nio.BufferUnderflowException;

import au.edu.jcu.v4l4j.ImagePalette;
import au.edu.jcu.v4l4j.exceptions.BufferOverflowException;
import au.edu.jcu.v4l4j.exceptions.UnsupportedMethod;
import au.edu.jcu.v4l4j.exceptions.V4L4JException;

public class ImageFormatConverter implements VideoFrameEncoder {
	protected static native int lookupConverterByConversion(int srcPalette, int dstPalette);
	
	private static native long init(int converterId, int width, int height);
	
	protected final int width;
	protected final int height;
	protected final int srcPaletteIdx;
	protected final int dstPaletteIdx;
	protected final long object;
	
	public static ImageFormatConverter forConversion(ImagePalette src, ImagePalette dst, int width, int height) {
		int converterId = lookupConverterByConversion(src.getIndex(), dst.getIndex());
		if (converterId < 0)
			throw new UnsupportedMethod("Cannot convert " + src.name() + " to " + dst.name());
		return new ImageFormatConverter(converterId, width, height);
	}
	
	public ImageFormatConverter(int converterId, int width, int height) {
		this(converterId, width, height, null, null);
	}
	
	protected ImageFormatConverter(int converterId, int width, int height, ImagePalette src, ImagePalette dst) {
		this(init(converterId, width, height), width, height, src, dst);
	}
	
	protected ImageFormatConverter(long ptr, int width, int height, ImagePalette src, ImagePalette dst) {
		this.object = ptr;
		this.width = width;
		this.height = height;
		this.srcPaletteIdx = src == null ? getSourcePaletteId() : src.getIndex();
		this.dstPaletteIdx = dst == null ? getOutputPaletteId() : dst.getIndex();
	}
	
	private native int getSourcePaletteId();
	
	private native int getOutputPaletteId();
	
	public native int getConverterId();
	
	@Override
	public ImagePalette getSourcePalette() {
		return ImagePalette.lookup(this.srcPaletteIdx);
	}
	
	@Override
	public int getSourceWidth() {
		return this.width;
	}
	
	@Override
	public int getSourceHeight() {
		return this.height;
	}
	
	@Override
	public ImagePalette getOutputPalette() {
		return ImagePalette.lookup(this.dstPaletteIdx);
	}
	
	@Override
	public int getOutputWidth() {
		return this.getSourceWidth();
	}
	
	@Override
	public int getOutputHeight() {
		return this.getSourceHeight();
	}
	
	@Override
	public int minimumSourceBufferLength() {
		// TODO use JNI to call v4lconvert_estimateBufferSize
		return getSourcePalette().getColorDepth() * width * height;
	}
	
	@Override
	public int minimumOutputBufferLength() {
		return getOutputPalette().getColorDepth() * width * height;
	}
	
	@Override
	public int minimumIntermediateBufferLength() {
		return 0;
	}
	
	@Override
	public native void close() throws Exception;
	
	protected native int encode(long bufferPtr) throws BufferUnderflowException, BufferOverflowException, NullPointerException, V4L4JException;
	
	@Override
	public int encode(V4lconvertBuffer buffer) throws BufferUnderflowException, BufferOverflowException, NullPointerException, V4L4JException {
		buffer.push();
		int result = this.encode(buffer.getPointer());
		buffer.pull();
		return result;
	}
	
}
