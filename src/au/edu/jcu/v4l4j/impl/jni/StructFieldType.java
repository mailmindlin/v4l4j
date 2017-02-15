package au.edu.jcu.v4l4j.impl.jni;

import java.nio.ByteBuffer;

public interface StructFieldType {

	int getAlignment();
	
	int getSize();
	
	boolean expands();
	
	@FunctionalInterface
	public static interface Writer {
		void write(ByteBuffer buffer, Object params);
	}
	
	Writer writer();
	
	@FunctionalInterface
	public static interface Reader {
		Object read(ByteBuffer buffer, StructReadingContext context);
	}
	
	Reader reader();
}
