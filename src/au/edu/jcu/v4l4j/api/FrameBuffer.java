package au.edu.jcu.v4l4j.api;

import java.nio.ByteBuffer;

public interface FrameBuffer {
	ByteBuffer asByteBuffer();
	
	/**
	 * A number that increments with the buffers, indicating their order.
	 * 
	 * Note that the difference between the value returned from this method and
	 * {@link #getTimestamp()} is that the sequence number is relative to other
	 * buffers from the same port, while the timestamp refers to when the data
	 * <i>contained</i> in the buffers was collected.
	 * @return the sequence number for this buffer
	 */
	long getSequenceNumber();
	
	/**
	 * Get the timestamp attached to this buffer.
	 * @see #getSequenceNumber
	 */
	long getTimestamp();
	
	default int getCapacity() {
		return asByteBuffer().capacity();
	}
	
	int getFilled();
	
	int getOffset();
	
	int getInputPort();
	
	int getOutputPort();
}
