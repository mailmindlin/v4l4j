package au.edu.jcu.v4l4j.api;

import java.nio.ByteBuffer;

public interface FrameBuffer {
	ByteBuffer asByteBuffer();
	long getTimestamp();
}
