package au.edu.jcu.v4l4j.impl.jni;

import java.nio.Buffer;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;

public class ArrayStructFieldType implements StructFieldType {
	private static final long serialVersionUID = 2215780449028701935L;
	
	protected final StructFieldType baseType;
	protected final int length;
	
	public ArrayStructFieldType(StructFieldType baseType, int length) {
		this.baseType = baseType;
		this.length = length;
	}
	
	@Override
	public int getAlignment() {
		//http://stackoverflow.com/questions/13284208/how-is-an-array-aligned-in-c-compared-to-a-type-contained#comment18111863_13284297
		return baseType.getAlignment();
	}

	@Override
	public int getSize() {
		//We can be reasonably sure that there isn't padding between the elements
		return baseType.getSize() * length;
	}
	
	public int getLength() {
		return this.length;
	}

	@Override
	public boolean expands() {
		return false;
	}
	
	public void writeElement(ByteBuffer buffer, int index, Object element) {
		final int size = this.baseType.getSize();
		ByteBuffer dup = MemoryUtils.sliceBuffer(buffer, size * index, size);
		this.baseType.write(dup, element);
	}
	
	public Object readElement(ByteBuffer buffer, int index) {
		final int size = this.baseType.getSize();
		ByteBuffer dup = MemoryUtils.sliceBuffer(buffer, size * index, size);
		return this.baseType.read(dup, null);
	}
	
	protected void writePrimitiveArray(Object p, ByteBuffer buffer) {
		Class<?> componentClass = p.getClass().getComponentType();
		final int size = this.baseType.getSize();
		if (componentClass == Byte.TYPE) {
			byte[] value = (byte[]) p;
			if (this.baseType == PrimitiveStructFieldType.INT8) {
				buffer.put(value);
				return;
			}
			for (int i = 0; i < value.length; i++)
				this.baseType.write(MemoryUtils.sliceBuffer(buffer, size * i, size), value[i]);
		} else if (componentClass == Short.TYPE) {
			short[] value = (short[]) p;
			if (this.baseType == PrimitiveStructFieldType.INT16) {
				buffer.asShortBuffer().put(value);
				buffer.position(buffer.position() + size * value.length);
				return;
			}
			for (int i = 0; i < value.length; i++)
				this.baseType.write(MemoryUtils.sliceBuffer(buffer, size * i, size), value[i]);
		} else if (componentClass == Integer.TYPE) {
			int[] value = (int[]) p;
			if (this.baseType == PrimitiveStructFieldType.INT32
					|| (this.baseType == PrimitiveStructFieldType.RAW_POINTER
					&& PrimitiveStructFieldType.RAW_POINTER.getAlignment() == PrimitiveStructFieldType.INT64.getAlignment()
					&& PrimitiveStructFieldType.RAW_POINTER.getSize() == PrimitiveStructFieldType.INT64.getSize())) {
				buffer.asIntBuffer().put(value);
				buffer.position(buffer.position() + size * value.length);
				return;
			}
			for (int i = 0; i < value.length; i++)
				this.baseType.write(MemoryUtils.sliceBuffer(buffer, size * i, size), value[i]);
		} else if (componentClass == Long.TYPE) {
			long[] value = (long[]) p;
			if (this.baseType == PrimitiveStructFieldType.INT64
					|| (this.baseType == PrimitiveStructFieldType.RAW_POINTER
							&& PrimitiveStructFieldType.RAW_POINTER.getAlignment() == PrimitiveStructFieldType.INT64.getAlignment()
							&& PrimitiveStructFieldType.RAW_POINTER.getSize() == PrimitiveStructFieldType.INT64.getSize())) {
				buffer.asLongBuffer().put(value);
				buffer.position(buffer.position() + size * value.length);
				return;
			}
			for (int i = 0; i < value.length; i++)
				this.baseType.write(MemoryUtils.sliceBuffer(buffer, size * i, size), value[i]);
		} else if (componentClass == Float.TYPE) {
			float[] value = (float[]) p;
			if (this.baseType == PrimitiveStructFieldType.FLOAT32) {
				buffer.asFloatBuffer().put(value);
				buffer.position(buffer.position() + size * value.length);
				return;
			}
			for (int i = 0; i < value.length; i++)
				this.baseType.write(MemoryUtils.sliceBuffer(buffer, size * i, size), value[i]);
		} else if (componentClass == Double.TYPE) {
			double[] value = (double[]) p;
			if (this.baseType == PrimitiveStructFieldType.FLOAT64) {
				buffer.asDoubleBuffer().put(value);
				buffer.position(buffer.position() + size * value.length);
				return;
			}
			for (int i = 0; i < value.length; i++)
				this.baseType.write(MemoryUtils.sliceBuffer(buffer, size * i, size), value[i]);
		} else if (componentClass == Character.TYPE) {
			char[] value = (char[]) p;
			if (this.baseType == PrimitiveStructFieldType.INT32) {
				buffer.asCharBuffer().put(value);
				buffer.position(buffer.position() + size * value.length);
				return;
			}
			for (int i = 0; i < value.length; i++)
				this.baseType.write(MemoryUtils.sliceBuffer(buffer, size * i, size), value[i]);
		} else if (componentClass == Boolean.TYPE) {
			boolean[] value = (boolean[]) p;
			if (this.baseType == PrimitiveStructFieldType.INT8) {
				for (int i = 0; i < value.length; i++)
					buffer.put((byte) (value[i] ? 0xFF : 0x00));
				buffer.position(buffer.position() + size * value.length);
				return;
			}
			for (int i = 0; i < value.length; i++)
				this.baseType.write(MemoryUtils.sliceBuffer(buffer, size * i, size), value[i]);
		} else {
			throw new IllegalArgumentException("Type is not primitive");
		}
	}
	
	@Override
	public void write(ByteBuffer buffer, Object value) {
		if (value instanceof Object[]) {
			Object[] values = (Object[]) value;
			final int size = this.baseType.getSize();
			for (int i = 0; i < this.length; i++) {
				ByteBuffer dup = MemoryUtils.sliceBuffer(buffer, size * i, size);
				this.baseType.write(dup, values[i]);
			}
		} else if (value instanceof String) {
			//Write chars
			if (this.baseType == PrimitiveStructFieldType.INT8) {
				//We can optimize the many writes
				byte[] bytes = ((String) value).getBytes();
				buffer.put(bytes);
			} else {
				this.writePrimitiveArray(((String) value).toCharArray(), buffer);
			}
		} else if (value instanceof Buffer) {
			Buffer bValue = (Buffer) value;
			if (bValue.hasArray()) {
				this.writePrimitiveArray(bValue.array(), buffer);
				return;
			}
			//TODO finish
		} else if (value.getClass().isArray()) {
			//Handle primitive arrays
			this.writePrimitiveArray(value, buffer);
		}
	}
	
	@Override
	public Object read(ByteBuffer buffer, StructReadingContext parentContext) {
		List<Object> values = new ArrayList<>();
		StructReadingContext context = parentContext.child(this, values);
		for (int i = 0; i < this.length; i++)
			values.add(this.baseType.read(buffer, context));
		return values;
	}
	
	public StructFieldType getBaseType() {
		return this.baseType;
	}

}
