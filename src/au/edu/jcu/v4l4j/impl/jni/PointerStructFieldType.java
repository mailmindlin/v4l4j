package au.edu.jcu.v4l4j.impl.jni;

import java.nio.ByteBuffer;

public class PointerStructFieldType implements StructFieldType {
	protected final StructFieldType farType;
	
	public PointerStructFieldType(StructFieldType farType) {
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
	
	public StructFieldType getFarType() {
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
	
}