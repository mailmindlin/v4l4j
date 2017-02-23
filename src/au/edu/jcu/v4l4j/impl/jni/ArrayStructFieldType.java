package au.edu.jcu.v4l4j.impl.jni;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;

public class ArrayStructFieldType implements StructFieldType {
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
	
	@Override
	public void write(ByteBuffer buffer, Object value) {
		//TODO handle primitive arrays
		Object[] values = (Object[]) value;
		final int size = this.baseType.getSize();
		for (int i = 0; i < this.length; i++) {
			ByteBuffer dup = MemoryUtils.sliceBuffer(buffer, size * i, size);
			this.baseType.write(dup, values[i]);
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
