package au.edu.jcu.v4l4j.impl.jni;

import java.io.ObjectStreamException;
import java.nio.ByteBuffer;

public class QStructFieldType implements StructFieldType {
	private static final long serialVersionUID = -4262354267638314597L;
	
	/**
	 * Constant for the q16 ({@code q16.16}) datatype
	 */
	public static final QStructFieldType Q16 = new QStructFieldType(16);
	
	/**
	 * Get a QStructFieldType for the type {@code q(n-1).n}.
	 * @param n
	 * @return
	 */
	public static QStructFieldType of(int n) {
		if (n == 16)
			return Q16;
		return new QStructFieldType(n);
	}
	
	private int n;
	
	protected QStructFieldType(int n) {
		this.n = n;
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
		int iValue = (int) Math.scalb(((Number)params).floatValue(), - this.n);
		PrimitiveStructFieldType.INT32.write(buffer, iValue);
	}
	
	@Override
	public Object read(ByteBuffer buffer, StructReadingContext context) {
		float fValue = ((Number) PrimitiveStructFieldType.INT32.read(buffer, context)).floatValue();
		return Math.scalb(fValue, this.n);
	}
	
	@Override
	public int hashCode() {
		/*
		 * Use QStructFieldType.serialVersionUID as a nice constant that will differentiate the hashcode of
		 * QStructFieldType's from other StructFieldType hashcodes, with minimal performance cost (as opposed
		 * to just returning this.n.
		 */
		return (int) (QStructFieldType.serialVersionUID * this.n);
	}
	
	@Override
	public boolean equals(Object o) {
		if (this == o)
			return true;
		
		if (!(o instanceof QStructFieldType))
			return false;
		
		return ((QStructFieldType)o).n == this.n;
	}
	
	/**
	 * Method called in deserialization.
	 * This method may replace the current object with a cached QStructFieldType.
	 * @return resolved type
	 * @throws ObjectStreamException actually won't
	 * @see Serializable
	 */
	private Object readResolve() throws ObjectStreamException {
		if (this.n == 16)
			return QStructFieldType.Q16;
		return this;
	}
}