package au.edu.jcu.v4l4j.encoder;

import java.io.Closeable;
import java.nio.ByteBuffer;
import java.util.concurrent.atomic.AtomicBoolean;

import au.edu.jcu.v4l4j.V4L4JUtils;
import au.edu.jcu.v4l4j.exceptions.JNIException;
import au.edu.jcu.v4l4j.exceptions.ReleaseException;

public final class V4lconvertBuffer implements Closeable {
	
	static {
		V4L4JUtils.loadLibrary();
	}
	
	private static native long allocate(int srcLen, int dstLen, int imedLen, boolean allocSrc, boolean allocDst);
	
	/**
	 * Wrap buffers
	 * @param src
	 * @param dst
	 * @param imed
	 * @return
	 */
	private static native long wrap(ByteBuffer src, ByteBuffer dst, ByteBuffer imed);
	
	protected final AtomicBoolean recycled = new AtomicBoolean(false);
	protected final long object;
	/**
	 * Whether the source buffer 'belongs' to this object, or whether it may be
	 * changed
	 */
	protected final boolean srcBound;
	/**
	 * Whether the destination buffer 'belongs' to this object
	 */
	protected final boolean dstBound;
	protected ByteBuffer src;
	protected ByteBuffer dst;
	protected ByteBuffer imed;
	
	protected V4lconvertBuffer(long ptr, boolean srcBound, boolean dstBound) {
		this.object = ptr;
		this.srcBound = srcBound;
		this.dstBound = dstBound;
		createBuffers();
		pull();
	}
	
	public V4lconvertBuffer(int srcLen, int dstLen) {
		this(srcLen, dstLen, 0, true, true);
	}
	
	public V4lconvertBuffer(int srcLen, int dstLen, int imedLen) {
		this(srcLen, dstLen, imedLen, true, true);
	}
	
	public V4lconvertBuffer(int srcLen, int dstLen, int imedLen, boolean srcAttached, boolean dstAttached) {
		this(allocate(srcLen, dstLen, imedLen, srcAttached, dstAttached), srcAttached, dstAttached);
	}
	
	public V4lconvertBuffer(ByteBuffer src, ByteBuffer dst, ByteBuffer imed) {
		this.object = wrap(src, dst, imed);
		this.src = src;
		this.dst = dst;
		this.imed = imed;
		this.srcBound = false;
		this.dstBound = false;
	}
	
	public ByteBuffer getSourceBuffer() {
		return this.src;
	}
	
	/**
	 * Set a direct ByteBuffer as the source for this buffer. Will fail if the
	 * current source ByteBuffer is attached, or the passed ByteBuffer is not
	 * direct.
	 * 
	 * @param src
	 *            Buffer to set as source. Must be direct.
	 * @return Whether this method succeeded or not
	 */
	public native boolean setSource(ByteBuffer src);
	
	public ByteBuffer getOutputBuffer() {
		return this.dst;
	}
	
	public native boolean setOutput(ByteBuffer dst) throws JNIException;
	
	public ByteBuffer getIntermediateBuffer() {
		return this.imed;
	}
	
	public boolean isRecycled() {
		return recycled.get();
	}
	
	public void recycle() {
		recycled.set(true);
	}
	
	public void refurbish() {
		recycled.set(false);
	}
	
	public long getPointer() {
		return this.object;
	}
	
	/**
	 * Put direct ByteBuffers referencing those of the native object
	 */
	private native void createBuffers();
	public native void push() throws JNIException;
	public native void pull() throws JNIException;
	
	@Override
	public void close() {
		release();
	}
	
	public native void release() throws JNIException, ReleaseException;
}
