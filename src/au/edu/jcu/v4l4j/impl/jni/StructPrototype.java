package au.edu.jcu.v4l4j.impl.jni;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.function.IntFunction;
import java.util.function.ToIntFunction;

public class StructPrototype implements StructFieldType {
	protected final StructField[] fields;
	protected final Map<String, StructField> fieldMap;

	public static StructPrototypeBuilder builder() {
		return new StructPrototypeBuilder();
	}

	protected static final StructField[] calculateOffsets(StructField... fields) {
		int lastOffset = 0;
		StructField[] offsetFields = new StructField[fields.length];
		for (int i = 0; i < fields.length; i++) {
			StructField oldField = fields[i];
			lastOffset = (int) MemoryUtils.align(oldField.getAlignment(), lastOffset);
			offsetFields[i] = oldField.withOffset(lastOffset);
			lastOffset += oldField.getSize();
		}
		return offsetFields;
	}
	
	protected static String rewrite(String prefix, String ext) {
		return (prefix.isEmpty() ? prefix : (prefix + ".")) + ext;
	}

	public StructPrototype(Collection<StructField> fields) {
		this(fields.toArray(new StructField[fields.size()]));
	}

	protected StructPrototype(StructField[] fields, Map<String, StructField> fieldMap) {
		this.fields = calculateOffsets(fields);
		this.fieldMap = fieldMap;
	}

	public StructPrototype(StructField... fields) {
		this.fields = calculateOffsets(fields);

		this.fieldMap = new HashMap<>();
		for (StructField field : this.fields)
			this.fieldMap.put(field.getName(), field);
	}

	public List<StructField> fields() {
		return new ArrayList<>(Arrays.asList(fields));
	}
	
	public StructField getField(String name) {
		return this.fieldMap.get(name);
	}

	/*protected Object readField(ByteBuffer buffer, StructField field, int offset, StructReadingContext context) {
			case POINTER: {
				long ptr;
				if (field.getType().getSize() == 8)
					ptr = buffer.getLong(offset);
				else
					ptr = (long) buffer.getInt(offset);
				PointerStructField pField = (PointerStructField) field;
				if (pField.dereference) {
					ByteBuffer dereferenced = MemoryUtils.wrap(ptr, pField.remote.getSize());
					return this.readField(dereferenced, pField.remote, 0, context);
				} else {
					return ptr;
				}
			}
		}
	}*/

	@Override
	public Map<String, Object> read(ByteBuffer buffer, StructReadingContext parentContext) {
		Map<String, Object> value = new HashMap<>();
		StructReadingContext context = new StructReadingContext(parentContext, this, value);
		for (int i = 0; i < this.fields.length; i++) {
			StructField field = this.fields[i];

			ByteBuffer dup = MemoryUtils.sliceBuffer(buffer, field.getOffset(), field.getType().expands() ? field.getSize() : -1);
			
			value.put(field.getName(), field.getType().read(dup, context));
		}
		return value;
	}

	@Override
	public void write(ByteBuffer buffer, Object value) {
		@SuppressWarnings("unchecked")
		Map<String, Object> params = (Map<String, Object>)value;
		for (int i = 0; i < this.fields.length; i++) {
			StructField field = this.fields[i];

			ByteBuffer dup = MemoryUtils.sliceBuffer(buffer, field.getOffset(), field.getType().expands() ? field.getSize() : -1);
			
			field.getType().write(dup, params.get(field.getName()));
		}
	}
	
	public Object readField(ByteBuffer buffer, String fieldName) {
		StructField field = this.fieldMap.get(fieldName);
		if (field == null)
			throw new IllegalArgumentException("Unknown field '" + fieldName + "'");
		
		ByteBuffer dup = MemoryUtils.sliceBuffer(buffer, field.getOffset(), field.getType().expands() ? field.getSize() : -1);
		
		return field.getType().read(dup, null);
	}
	
	public void writeField(ByteBuffer buffer, String fieldName, Object value) {
		StructField field = this.fieldMap.get(fieldName);
		if (field == null)
			throw new IllegalArgumentException("Unknown field '" + fieldName + "'");
		
		ByteBuffer dup = MemoryUtils.sliceBuffer(buffer, field.getOffset(), field.getType().expands() ? field.getSize() : -1);
		
		field.getType().write(dup, value);
	}

	public StructBuilder make() {
		return new StructBuilder();
	}

	@Override
	public int getSize() {
		if (this.fields == null || this.fields.length == 0)
			//We have no fields, and therefore no size
			return 0;
		StructField lastField = this.fields[this.fields.length - 1];
		//We can calculate our size by last field offset + last field size
		return lastField.getOffset() + lastField.getSize();
	}

	@Override
	public int getAlignment() {
		if (this.fields == null || this.fields.length == 0)
			//We have no fields, so alignment is 0
			return 1;
		//Our alignment is the same as our first field
		return this.fields[0].getAlignment();
	}

	@Override
	public boolean expands() {
		return this.fields[this.fields.length - 1].getType().expands();
	}
	
	public Map<String, Object> read(ByteBuffer buffer) {
		return this.read(buffer, null);
	}

	public static class StructPrototypeBuilder {
		protected final List<StructField> fields = new ArrayList<>();

		public StructPrototypeBuilder addBoolean(String name) {
			add(PrimitiveStructFieldType.BOOL, name);
			return this;
		}

		public StructPrototypeBuilder addInt8(String name) {
			add(PrimitiveStructFieldType.INT8, name);
			return this;
		}

		public StructPrototypeBuilder addInt16(String name) {
			add(PrimitiveStructFieldType.INT16, name);
			return this;
		}

		public StructPrototypeBuilder addInt32(String name) {
			add(PrimitiveStructFieldType.INT32, name);
			return this;
		}

		public StructPrototypeBuilder addInt64(String name) {
			add(PrimitiveStructFieldType.INT64, name);
			return this;
		}

		public StructPrototypeBuilder addFloat32(String name) {
			add(PrimitiveStructFieldType.FLOAT32, name);
			return this;
		}

		public StructPrototypeBuilder addFloat64(String name) {
			add(PrimitiveStructFieldType.FLOAT64, name);
			return this;
		}
		
		public StructPrototypeBuilder addPointer(StructFieldType farType, String name) {
			add(new PointerStructFieldType(farType), name);
			return this;
		}

		public StructPrototypeBuilder addPointer(String name) {
			add(PrimitiveStructFieldType.RAW_POINTER, name);
			return this;
		}

		public StructPrototypeBuilder addStruct(StructPrototype struct, String name) {
			add(struct, name);
			return this;
		}
		
		public StructPrototypeBuilder addString(int length, String name) {
			add(new StringStructFieldType(length), name);
			return this;
		}
		
		public <E extends Enum<E>> StructPrototypeBuilder addEnum(Class<E> enumClass, String name) {
			add(new EnumStructFieldType<>(enumClass), name);
			return this;
		}
		
		public <E extends Enum<E>> StructPrototypeBuilder addEnum(IntFunction<E> mapper, ToIntFunction<E> unmapper, String name) {
			add(new EnumStructFieldType<>(mapper, unmapper), name);
			return this;
		}

		public StructPrototypeBuilder add(StructFieldType type, String name) {
			this.fields.add(new StructField(type, name, -1));
			return this;
		}
		
		public StructPrototype build() {
			return new StructPrototype(this.fields);
		}
	}

	public class StructBuilder {
		Map<String, Object> values;

		public StructBuilder set(String name, Object value) {
			this.values.put(name, value);
			return this;
		}

		public void writeTo(ByteBuffer buffer) {
			StructPrototype.this.write(buffer, values);
		}
	}
}
