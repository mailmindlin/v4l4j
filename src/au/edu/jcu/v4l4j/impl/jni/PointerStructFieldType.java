package au.edu.jcu.v4l4j.impl.jni;

import java.nio.ByteBuffer;

//TODO fix generics
public class PointerStructFieldType implements StructFieldType<Object> {
	private static final long serialVersionUID = -6196112410849049148L;
	
	protected final StructFieldType<?> farType;
	
	public PointerStructFieldType(StructFieldType<?> farType) {
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
	
	public StructFieldType<?> getFarType() {
		return this.farType;
	}
	
	@Override
	public void write(ByteBuffer buffer, Object params) {
		PrimitiveStructFieldType.RAW_POINTER.write(buffer, params);
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