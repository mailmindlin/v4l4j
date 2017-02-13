package au.edu.jcu.v4l4j.impl.omx;

import java.nio.ByteBuffer;

import au.edu.jcu.v4l4j.api.FrameBuffer;

public class OMXFrameBuffer implements FrameBuffer {
	protected final long pointer;
	private final ByteBuffer buffer;
	
	protected OMXFrameBuffer(long pointer, ByteBuffer buffer) {
		this.pointer = pointer;
		this.buffer = buffer;
	}
	
	@Override
	public ByteBuffer asByteBuffer() {
		return this.buffer;
	}
	
	@Override
	public long getTimestamp() {
		// TODO Auto-generated method stub
		return 0;
	}
	
	@Override
	public int getCapacity() {
		// TODO Auto-generated method stub
		return 0;
	}
	
	@Override
	public int getFilled() {
		// TODO Auto-generated method stub
		return 0;
	}
	
	@Override
	public int getOffset() {
		// TODO Auto-generated method stub
		return 0;
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
