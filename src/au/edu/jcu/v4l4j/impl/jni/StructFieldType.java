package au.edu.jcu.v4l4j.impl.jni;

import java.io.Serializable;
import java.nio.ByteBuffer;

public interface StructFieldType extends Serializable {

	int getAlignment();
	
	int getSize();
	
	boolean expands();
	
	void write(ByteBuffer buffer, Object params);
	
	Object read(ByteBuffer buffer, StructReadingContext context);
}