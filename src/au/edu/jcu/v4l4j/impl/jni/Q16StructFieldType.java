package au.edu.jcu.v4l4j.impl.jni;

import java.nio.ByteBuffer;

public class QStructFieldType implements StructFieldType {
	private final int n;
	transient float factor;
	
	public QStructFieldType(int n) {
		this.n = n;
		this.factor = Math.pow(2.0f, n);
	}
	
	@Override
	public int getAlignment() {
		return PrimitiveStructFieldType.INT32.getAlignment();
	}
	
	@Override
	public int getSize() {
		return PrimitiveStructFieldType.INT32.getSize();
	}

	@Override
	public boolean expands() {
		return false;
	}
	
	@Override
	public void write(ByteBuffer buffer, Object params) {
		float fValue = ((Number)params).floatValue();
		int iValue = (int) (fValue * this.factor);
		PrimitiveStructFieldType.INT32.write(buffer, iValue);
	}
	
	@Override
	public Object read(ByteBuffer buffer, StructReadingContext context) {
		int iValue = PrimitiveStructFieldType.INT32.read(buffer, context);
		return ((float)iValue) / this.factor;
	}
}