package au.edu.jcu.v4l4j.impl.jni;

import java.io.Serializable;
import java.nio.ByteBuffer;

/**
 * Definition for a struct field.
 * @author mailmindlin
 *
 * @param <T> POJO-mapped type
 */
public interface StructFieldType<T> extends Serializable {
	/**
	 * Get field alignment
	 * @return alignment (in bytes)
	 */
	int getAlignment();
	
	/**
	 * Get field size
	 * @return size (in bytes)
	 */
	int getSize();
	
	boolean expands();
	
	@SuppressWarnings("unchecked")
	default void writeUnchecked(ByteBuffer buffer, Object params) {
		this.write(buffer, (T) params);
	}
	
	/**
	 * Write to buffer at the next position.
	 * @param buffer buffer to write to
	 * @param params POJO to serialize & write
	 */
	void write(ByteBuffer buffer, T params);
	
	/**
	 * Read from buffer at the next position
	 * @param buffer
	 * @param context
	 * @return deserialized POJO
	 */
	T read(ByteBuffer buffer, StructReadingContext context);
}