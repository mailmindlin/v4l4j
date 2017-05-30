package au.edu.jcu.v4l4j.impl.jni;

import java.nio.Buffer;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.Enumeration;
import java.util.Iterator;
import java.util.List;
import java.util.Objects;
import java.util.RandomAccess;

public class ArrayStructFieldType<T> implements StructFieldType<List<T>> {
	private static final long serialVersionUID = 2215780449028701935L;
	
	protected final StructFieldType<T> baseType;
	protected final int length;
	
	public ArrayStructFieldType(StructFieldType<T> baseType, int length) {
		Objects.requireNonNull(baseType, "Base type for array may not be null");
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
	
	/**
	 * Get the maximum number of elements that this array can hold.
	 * @return
	 */
	public int getLength() {
		return this.length;
	}

	@Override
	public boolean expands() {
		return false;
	}
	
	public void writeElementUnchecked(ByteBuffer buffer, int index, Object element) {
		this.ensureCapacity(index);
		
		final int size = this.baseType.getSize();
		ByteBuffer dup = MemoryUtils.sliceBuffer(buffer, size * index, size);
		this.baseType.writeUnchecked(dup, element);
	}
	
	public void writeElement(ByteBuffer buffer, int index, T element) {
		this.ensureCapacity(index);
		
		final int size = this.baseType.getSize();
		ByteBuffer dup = MemoryUtils.sliceBuffer(buffer, size * index, size);
		this.baseType.write(dup, element);
	}
	
	public T readElement(ByteBuffer buffer, int index) {
		this.ensureCapacity(index);
		
		final int size = this.baseType.getSize();
		ByteBuffer dup = MemoryUtils.sliceBuffer(buffer, size * index, size);
		return this.baseType.read(dup, null);
	}
	
	/**
	 * Ensures that this field has enough capacity to write some length of elements.
	 * @param length
	 */
	protected void ensureCapacity(int length) throws IllegalArgumentException {
		final int myLen = this.getLength();
		if (length > myLen && myLen != -1)
			throw new IllegalArgumentException("Length exceeded maximum (expected: <" + myLen + "; was: " + length + ")");
	}
	
	protected void writePrimitiveArray(Object p, ByteBuffer buffer) {
		Class<?> componentClass = p.getClass().getComponentType();
		final int size = this.baseType.getSize();
		
		if (componentClass == Byte.TYPE) {
			byte[] value = (byte[]) p;
			this.ensureCapacity(value.length);
			
			if (this.baseType == PrimitiveStructFieldType.INT8) {
				buffer.put(value);
				return;
			}
			
			
			@SuppressWarnings("unchecked")
			StructFieldType<? super Byte> base = (StructFieldType<? super Byte>) this.baseType;
			
			for (int i = 0; i < value.length; i++)
				base.write(MemoryUtils.sliceBuffer(buffer, size * i, size), value[i]);
			
		} else if (componentClass == Short.TYPE) {
			short[] value = (short[]) p;
			this.ensureCapacity(value.length);
			
			if (this.baseType == PrimitiveStructFieldType.INT16) {
				buffer.asShortBuffer().put(value);
				buffer.position(buffer.position() + size * value.length);
				return;
			}
			
			
			@SuppressWarnings("unchecked")
			StructFieldType<? super Short> base = (StructFieldType<? super Short>) this.baseType;
			
			for (int i = 0; i < value.length; i++)
				base.write(MemoryUtils.sliceBuffer(buffer, size * i, size), value[i]);
			
		} else if (componentClass == Integer.TYPE) {
			int[] value = (int[]) p;
			this.ensureCapacity(value.length);
			
			if (this.baseType == PrimitiveStructFieldType.INT32
					|| (this.baseType == PrimitiveStructFieldType.RAW_POINTER
					&& PrimitiveStructFieldType.RAW_POINTER.getAlignment() == PrimitiveStructFieldType.INT64.getAlignment()
					&& PrimitiveStructFieldType.RAW_POINTER.getSize() == PrimitiveStructFieldType.INT64.getSize())) {
				buffer.asIntBuffer().put(value);
				buffer.position(buffer.position() + size * value.length);
				return;
			}
			
			
			@SuppressWarnings("unchecked")
			StructFieldType<? super Integer> base = (StructFieldType<? super Integer>) this.baseType;
			
			for (int i = 0; i < value.length; i++)
				base.write(MemoryUtils.sliceBuffer(buffer, size * i, size), value[i]);
			
		} else if (componentClass == Long.TYPE) {
			long[] value = (long[]) p;
			this.ensureCapacity(value.length);
			
			if (this.baseType == PrimitiveStructFieldType.INT64
					|| (this.baseType == PrimitiveStructFieldType.RAW_POINTER
							&& PrimitiveStructFieldType.RAW_POINTER.getAlignment() == PrimitiveStructFieldType.INT64.getAlignment()
							&& PrimitiveStructFieldType.RAW_POINTER.getSize() == PrimitiveStructFieldType.INT64.getSize())) {
				buffer.asLongBuffer().put(value);
				buffer.position(buffer.position() + size * value.length);
				return;
			}
			
			
			@SuppressWarnings("unchecked")
			StructFieldType<? super Long> base = (StructFieldType<? super Long>) this.baseType;
			
			for (int i = 0; i < value.length; i++)
				base.write(MemoryUtils.sliceBuffer(buffer, size * i, size), value[i]);
			
		} else if (componentClass == Float.TYPE) {
			float[] value = (float[]) p;
			this.ensureCapacity(value.length);
			
			if (this.baseType == PrimitiveStructFieldType.FLOAT32) {
				buffer.asFloatBuffer().put(value);
				buffer.position(buffer.position() + size * value.length);
				return;
			}
			
			
			@SuppressWarnings("unchecked")
			StructFieldType<? super Float> base = (StructFieldType<? super Float>) this.baseType;
			
			for (int i = 0; i < value.length; i++)
				base.write(MemoryUtils.sliceBuffer(buffer, size * i, size), value[i]);
			
		} else if (componentClass == Double.TYPE) {
			double[] value = (double[]) p;
			this.ensureCapacity(value.length);
			
			if (this.baseType == PrimitiveStructFieldType.FLOAT64) {
				buffer.asDoubleBuffer().put(value);
				buffer.position(buffer.position() + size * value.length);
				return;
			}
			
			@SuppressWarnings("unchecked")
			StructFieldType<? super Double> base = (StructFieldType<? super Double>) this.baseType;
			
			for (int i = 0; i < value.length; i++)
				base.write(MemoryUtils.sliceBuffer(buffer, size * i, size), value[i]);
			
		} else if (componentClass == Character.TYPE) {
			char[] value = (char[]) p;
			this.ensureCapacity(value.length);
			
			if (this.baseType == PrimitiveStructFieldType.INT32) {
				buffer.asCharBuffer().put(value);
				buffer.position(buffer.position() + size * value.length);
				return;
			}
			
			
			@SuppressWarnings("unchecked")
			StructFieldType<? super Character> base = (StructFieldType<? super Character>) this.baseType;
			
			for (int i = 0; i < value.length; i++)
				base.write(MemoryUtils.sliceBuffer(buffer, size * i, size), value[i]);
			
		} else if (componentClass == Boolean.TYPE) {
			boolean[] value = (boolean[]) p;
			this.ensureCapacity(value.length);
			
			if (this.baseType == PrimitiveStructFieldType.INT8) {
				for (int i = 0; i < value.length; i++)
					buffer.put((byte) (value[i] ? 0xFF : 0x00));
				buffer.position(buffer.position() + size * value.length);
				return;
			}
			
			
			@SuppressWarnings("unchecked")
			StructFieldType<? super Boolean> base = (StructFieldType<? super Boolean>) this.baseType;
			
			for (int i = 0; i < value.length; i++)
				base.write(MemoryUtils.sliceBuffer(buffer, size * i, size), value[i]);
			
		} else {
			throw new IllegalArgumentException("Type is not primitive");
		}
	}
	
	@Override
	public void write(ByteBuffer buffer, List<T> value) {
		final int len = value.size();
		final int size = this.baseType.getSize();
		this.ensureCapacity(len);
		
		if (value instanceof RandomAccess) {
			//List supports fast (usually O(1)) random access (e.g., ArrayList)
			for (int i = 0; i < value.size(); i++)
				this.baseType.write(MemoryUtils.sliceBuffer(buffer, i * size, size), value.get(i));
		} else {
			//Fall back on Iterator for other implementations (LinkedList is O(n) for random access, but O(1) for each iterator step).
			int i = 0;
			Iterator<? extends T> iterator = value.iterator();
			while (iterator.hasNext()) {
				this.baseType.write(MemoryUtils.sliceBuffer(buffer, i * size, size), iterator.next());
				i++;
			}
		}
	}
	
	@Override
	public void writeUnchecked(ByteBuffer buffer, Object value) {
		//TODO can we optimize this?
		if (value instanceof Object[]) {
			Object[] values = (Object[]) value;
			this.ensureCapacity(values.length);
			
			final int size = this.baseType.getSize();
			
			@SuppressWarnings("unchecked")
			StructFieldType<? super Object> base = (StructFieldType<? super Object>) this.baseType;
			
			for (int i = 0; i < this.length; i++) {
				ByteBuffer dup = MemoryUtils.sliceBuffer(buffer, size * i, size);
				base.write(dup, values[i]);
			}
		} else if (value instanceof String) {
			//Write chars
			if (this.baseType == PrimitiveStructFieldType.INT8) {
				//We can optimize the many writes
				byte[] bytes = ((String) value).getBytes();
				this.ensureCapacity(bytes.length);
				
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
			throw new UnsupportedOperationException("Not finished");
			//TODO finish
		} else if (value.getClass().isArray()) {
			//Handle primitive arrays
			this.writePrimitiveArray(value, buffer);
		} else if (value instanceof List) {
			@SuppressWarnings("unchecked")
			List<T> v = (List<T>) value;
			this.write(buffer, v);
		} else if (value instanceof Collection) {
			@SuppressWarnings("unchecked")
			Collection<? extends T> v = (Collection<? extends T>) value;
			this.write(buffer, new ArrayList<T>(v));
		} else if (value instanceof Enumeration) {
			@SuppressWarnings("unchecked")
			Enumeration<T> v = (Enumeration<T>) value;
			this.write(buffer, Collections.list(v));
		} else {
			throw new IllegalArgumentException("Cannot write: " + value);
		}
	}
	
	@Override
	public List<T> read(ByteBuffer buffer, StructReadingContext parentContext) {
		List<T> values = new ArrayList<>();
		StructReadingContext context = parentContext.child(this, values);
		for (int i = 0; i < this.length; i++)
			values.add(this.baseType.read(buffer, context));
		return values;
	}
	
	public StructFieldType<T> getBaseType() {
		return this.baseType;
	}

}
