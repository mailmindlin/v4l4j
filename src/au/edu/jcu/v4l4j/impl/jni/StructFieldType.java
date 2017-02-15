package au.edu.jcu.v4l4j.impl.jni;

import java.nio.ByteBuffer;
import java.util.function.BiConsumer;
import java.util.function.Function;

public interface StructFieldType {

	int getAlignment();
	
	int getSize();
	
	boolean expands();
	
	@FunctionalInterface
	public static interface Writer {
		void write(ByteBuffer buffer, Function<String, Object> params);
	}
	
	Writer writer();
	
	@FunctionalInterface
	public static interface Reader {
		void read(ByteBuffer buffer, BiConsumer<String, Object> params);
	}
	
	Reader reader();
}
