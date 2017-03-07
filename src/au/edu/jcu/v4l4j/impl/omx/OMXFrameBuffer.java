package au.edu.jcu.v4l4j.impl.omx;

import java.nio.ByteBuffer;

import au.edu.jcu.v4l4j.api.FrameBuffer;

public class OMXFrameBuffer implements FrameBuffer {
	protected final long pointer;
	protected final ByteBuffer buffer;
	protected int inPortIdx;
	protected int outPortIdx;
	private int ticks;
	private long timestamp;
	private int flags;
	
	protected OMXFrameBuffer(long pointer, ByteBuffer buffer, int inPortIdx, int outPortIdx) {
		this.pointer = pointer;
		this.buffer = buffer;
		this.inPortIdx = inPortIdx;
		this.outPortIdx = outPortIdx;
	}
	
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
