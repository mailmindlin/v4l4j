package au.edu.jcu.v4l4j.encoder;

import java.nio.ByteBuffer;
import java.util.concurrent.atomic.AtomicBoolean;

public final class V4lconvertBuffer {
	protected final AtomicBoolean recycled = new AtomicBoolean(false);
	protected final long object;
	protected final ByteBuffer src;
	protected final ByteBuffer dst;
	protected final ByteBuffer imed;
	
	public V4lconvertBuffer(long ptr) {
		this.object = ptr;
		this.src = null;
		this.dst = null;
		this.imed = null;
	}
	
	public V4lconvertBuffer(long ptr, ByteBuffer src, ByteBuffer dst, ByteBuffer imed) {
		this.object = ptr;
		this.src = src;
		this.dst = dst;
		this.imed = imed;
	}
	
	public ByteBuffer getSourceBuffer() {
		return this.src;
	}
	
	public ByteBuffer getDestinationBuffer() {
		return this.dst;
	}
	
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
	
	native void refreshBuffers();
	
	public native void release();
}
