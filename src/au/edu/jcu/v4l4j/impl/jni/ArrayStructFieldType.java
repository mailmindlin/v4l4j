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

	@Override
	public boolean expands() {
		return false;
	}
	
	public void write(ByteBuffer buffer, Object value) {
		//TODO handle primitive arrays
		Object[] values = (Object[]) value;
		Writer w = this.baseType.writer();
		for (int i = 0; i < this.length; i++)
			w.write(buffer, values[i]);
	}
	
	public List<Object> read(ByteBuffer buffer, StructReadingContext parentContext) {
		List<Object> values = new ArrayList<>();
		StructReadingContext context = parentContext.child(this, values);
		Reader r = this.baseType.reader();
		for (int i = 0; i < this.length; i++)
			values.add(r.read(buffer, context));
		return values;
	}

	@Override
	public Writer writer() {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Reader reader() {
		// TODO Auto-generated method stub
		return null;
	}
	
}
