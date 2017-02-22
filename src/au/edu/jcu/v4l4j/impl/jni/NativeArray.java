package au.edu.jcu.v4l4j.impl.jni;

public class NativeArray implements List<Object>, AutoCloseable {
	protected final ByteBuffer buffer;
	ArrayStructFieldType arrayType;
	
	@Override
	Object get(int index) {
		return arrayType.readElement(buffer, index)
	}
	
	@Override
	Object set(int index, Object value) {
		Object oldValue = get(index);
		arrayType.writeElement(buffer, index, value);
		return oldValue;
	}
}