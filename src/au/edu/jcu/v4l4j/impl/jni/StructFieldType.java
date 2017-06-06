package au.edu.jcu.v4l4j.impl.jni;

import java.io.Serializable;
import java.nio.ByteBuffer;

public interface StructFieldType<T> extends Serializable {

	int getAlignment();
	
	int getSize();
	
	boolean expands();
	
	@SuppressWarnings("unchecked")
	default void writeUnchecked(ByteBuffer buffer, Object params) {
		this.write(buffer, (T) params);
	}
	
	void write(ByteBuffer buffer, T params);
	
	T read(ByteBuffer buffer, StructReadingContext context);
}