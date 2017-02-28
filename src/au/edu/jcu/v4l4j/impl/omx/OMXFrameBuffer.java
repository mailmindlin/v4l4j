package au.edu.jcu.v4l4j.impl.omx;

import java.nio.ByteBuffer;

import au.edu.jcu.v4l4j.api.FrameBuffer;

public class OMXFrameBuffer implements FrameBuffer {
	protected final long pointer;
	private final ByteBuffer buffer;
	private int ticks;
	private long timestamp;
	private int flags;
	
	protected OMXFrameBuffer(long pointer, ByteBuffer buffer) {
		this.pointer = pointer;
		this.buffer = buffer;
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
		return this.buffer.duplicate();
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
		// TODO Auto-generated method stub
		return 0;
	}
	
	@Override
	public int getOutputPort() {
		// TODO Auto-generated method stub
		return 0;
	}
	
}
