package au.edu.jcu.v4l4j.impl.jni;

import java.nio.ByteBuffer;

//TODO fix generics
public class PointerStructFieldType<T> implements StructFieldType<Object> {
	private static final long serialVersionUID = -6196112410849049148L;
	
	protected final StructFieldType<T> farType;
	
	public PointerStructFieldType(StructFieldType<T> farType) {
		this.farType = farType;
	}
	
	@Override
	public int getAlignment() {
		return PrimitiveStructFieldType.RAW_POINTER.getAlignment();
	}
	
	@Override
	public int getSize() {
		return PrimitiveStructFieldType.RAW_POINTER.getSize();
	}
	
	@Override
	public boolean expands() {
		return false;
	}
	
	public StructFieldType<T> getFarType() {
		return this.farType;
	}
	
	@Override
	public void write(ByteBuffer buffer, Object params) {
		PrimitiveStructFieldType.RAW_POINTER.writeUnchecked(buffer, params);
	}
	
	@Override
	public Object read(ByteBuffer buffer, StructReadingContext context) {
		return PrimitiveStructFieldType.RAW_POINTER.read(buffer, context);
	}
	
	@Override
	public String toString() {
		return "PointerType{farType=" + getFarType() + ",alignment=" + getAlignment() + ",size=" + getSize()
				+ ",expands=" + expands() + "}";
	}
}