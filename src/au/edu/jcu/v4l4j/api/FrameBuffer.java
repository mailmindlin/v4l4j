package au.edu.jcu.v4l4j.api;

import java.nio.ByteBuffer;

public interface FrameBuffer {
	ByteBuffer asByteBuffer();
	
	long getTimestamp();
	
	default int getCapacity() {
		return asByteBuffer().capacity();
	}
	
	int getFilled();
	
	int getOffset();
	
	int getInputPort();
	
	int getOutputPort();
}
