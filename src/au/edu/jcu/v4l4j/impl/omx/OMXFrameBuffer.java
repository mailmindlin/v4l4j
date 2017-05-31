package au.edu.jcu.v4l4j.impl.omx;

import java.nio.ByteBuffer;

import au.edu.jcu.v4l4j.api.FrameBuffer;

/**
 * Java object to wrap a OMX_FRAMEBUFFERTYPE.
 * @author mailmindlin
 */
public class OMXFrameBuffer implements FrameBuffer {
	/**
	 * Pointer to native memory
	 */
	protected final long pointer;
	/**
	 * ByteBuffer wrapping the frame buffer's data.
	 * Note that this is independent of the memory referenced by {@link #pointer}.
	 */
	protected final ByteBuffer buffer;
	/**
	 * Input port index
	 */
	protected int inPortIdx;
	protected int outPortIdx;
	private int ticks;
	private long timestamp;
	private int flags;
	
	/**
	 * Contruct an OMXFrameBuffer.
	 * This method is designed to be called from JNI code.
	 */
	/*
	 * Non-javadoc:
	 * Because this constructor is called by native code, any changes to the signature (and possibly behavior)
	 * should be reflected therein.
	 */
	protected OMXFrameBuffer(long pointer, ByteBuffer buffer, int inPortIdx, int outPortIdx) {
		this.pointer = pointer;
		this.buffer = buffer;
		this.inPortIdx = inPortIdx;
		this.outPortIdx = outPortIdx;
	}
	
	/**
	 * Called before being sent to a handler, updating internal fields that may have been changed by
	 * the component.
	 */
	protected void prepare(int ticks, long timestamp, int offset, int filled, int flags) {
		this.ticks = ticks;
		this.timestamp = timestamp;
		this.buffer.position(offset);
		this.buffer.limit(filled);
		this.flags = flags;
	}
	
	@Override
	public ByteBuffer asByteBuffer() {
		return this.buffer;
	}
	
	@Override
	public long getSequenceNumber() {
		return this.ticks;
	}
	
	@Override
	public long getTimestamp() {
		return this.timestamp;
	}
	
	@Override
	public int getCapacity() {
		return buffer.capacity();
	}
	
	@Override
	public int getFilled() {
		return buffer.remaining();
	}
	
	@Override
	public int getOffset() {
		return buffer.position();
	}
	
	@Override
	public int getInputPort() {
		return this.inPortIdx;
	}
	
	@Override
	public int getOutputPort() {
		return this.outPortIdx;
	}
	
	/**
	 * Get the flags set on this buffer
	 * @return flags
	 */
	public int getFlags() {
		return this.flags;
	}
	
	@Override
	public int hashCode() {
		return (int) ((this.pointer >> 32) & this.pointer) & 0xFFFFFFFF;
	}

	@Override
	public void setSequenceNumber(long sequence) throws UnsupportedOperationException {
		if (sequence < Integer.MIN_VALUE || sequence > Integer.MAX_VALUE)
			throw new IllegalArgumentException();
		this.ticks = (int) sequence;
	}

	@Override
	public void setTimestamp(long timestamp) throws UnsupportedOperationException {
		this.timestamp = timestamp;
	}
	
}
