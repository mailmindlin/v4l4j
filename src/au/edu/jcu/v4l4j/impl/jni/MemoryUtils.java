package au.edu.jcu.v4l4j.impl.jni;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

import au.edu.jcu.v4l4j.V4L4JUtils;

public final class MemoryUtils {

	static {
		V4L4JUtils.loadLibrary();
	}
	
	static native int getAlignment(int type);
	static native int getSize(int type);
	private static native ByteBuffer doWrap(long pointer, long length);
	
	public static native long unwrap(ByteBuffer buffer);
	
	public static native long alloc(int alignment, long length);
	public static native long memset(long pointer, long length, int value);
	public static native long realloc(long pointer, long length);
	public static native void free(long pointer);
	
	public static ByteBuffer wrap(long pointer, long length) {
		ByteBuffer result = doWrap(pointer, length);
		result.order(ByteOrder.nativeOrder());
		return result;
	}
	
	public static long align(int alignment, long pointer) {
		return (pointer + alignment - 1) & ~(alignment - 1L);
	}
	

	public static ByteBuffer sliceBuffer(ByteBuffer buf, int offset, int length) {
		ByteBuffer dup = buf.duplicate();
		dup.position(buf.position() + offset);
		if (length > -1)
			dup.limit(dup.position() + length);
		dup.order(buf.order());
		return dup;
	}
}