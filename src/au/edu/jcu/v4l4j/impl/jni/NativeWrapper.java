package au.edu.jcu.v4l4j.impl.jni;

import java.nio.ByteBuffer;

public interface NativeWrapper extends AutoCloseable {
	ByteBuffer buffer();
	
	long pointer();
	
	StructFieldType type();
}
