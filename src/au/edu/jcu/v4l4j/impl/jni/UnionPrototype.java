package au.edu.jcu.v4l4j.impl.jni;

import java.nio.ByteBuffer;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

public class UnionPrototype implements StructFieldType {
	private static final long serialVersionUID = -8610611270611404524L;
	
	protected Set<StructField> options;
	protected transient Map<String, StructField> optionLookup;
	protected transient int alignment = -1;
	protected transient int size = -1;
	
	public static UnionPrototypeBuilder builder() {
		return new UnionPrototypeBuilder();
	}
	
	public UnionPrototype(Set<StructField> options) {
		this.options = new HashSet<>(options);
	}
	
	@Override
	public int getAlignment() {
		if (this.alignment < 0) {
			int alignment = 0;
			for (StructField option : options)
				alignment = Math.max(alignment, option.getAlignment());
			this.alignment = alignment;
		}
		return this.alignment;
	}

	@Override
	public int getSize() {
		if (this.size < 0) {
			int size = 0;
			for (StructField option : options)
				size = Math.max(size, option.getSize());
			this.size = size;
		}
		return this.size;
	}
	
	public Set<StructField> getOptions() {
		return Collections.unmodifiableSet(this.options);
	}

	@Override
	public boolean expands() {
		return false;
	}
	
	public StructField getOption(String name) {
		Map<String, StructField> lookup = this.optionLookup;
		if (lookup == null) {
			this.optionLookup = lookup = new HashMap<>();
			for (StructField option : this.options)
				lookup.put(option.getName(), option);
		}
		return lookup.get(name);
	}
	
	public void writeField(ByteBuffer buffer, String fieldName, Object value) {
		StructField field = getOption(fieldName);
		field.getType().write(buffer, value);
	}
	
	public Object readField(ByteBuffer buffer, String fieldName) {
		return getOption(fieldName).getType().read(buffer, null);
	}

	@Override
	public void write(ByteBuffer buffer, Object params) {
		@SuppressWarnings("unchecked")
		Map<String, Object> value = (Map<String, Object>) params;
		for (StructField option : options) {
			if (value.containsKey(option.getName())) {
				option.getType().write(buffer, value.get(option.getName()));
				return;
			}
		}
	}

	@Override
	public Object read(ByteBuffer buffer, StructReadingContext parentContext) {
		Map<String, Object> result = new HashMap<>();
		StructReadingContext context = new StructReadingContext(parentContext, this, result);
		for (StructField option : this.getOptions()) {
			try {
				result.put(option.getName(), option.getType().read(buffer, context));
			} catch (Exception e) {
				//Swallow
			}
		}
		return result;
	}
	
	public static class UnionPrototypeBuilder {
		protected final Set<StructField> fields = new HashSet<>();

		public UnionPrototypeBuilder addBoolean(String name) {
			add(PrimitiveStructFieldType.BOOL, name);
			return this;
		}

		public UnionPrototypeBuilder addInt8(String name) {
			add(PrimitiveStructFieldType.INT8, name);
			return this;
		}

		public UnionPrototypeBuilder addInt16(String name) {
			add(PrimitiveStructFieldType.INT16, name);
			return this;
		}

		public UnionPrototypeBuilder addInt32(String name) {
			add(PrimitiveStructFieldType.INT32, name);
			return this;
		}

		public UnionPrototypeBuilder addInt64(String name) {
			add(PrimitiveStructFieldType.INT64, name);
			return this;
		}

		public UnionPrototypeBuilder addFloat32(String name) {
			add(PrimitiveStructFieldType.FLOAT32, name);
			return this;
		}

		public UnionPrototypeBuilder addFloat64(String name) {
			add(PrimitiveStructFieldType.FLOAT64, name);
			return this;
		}
		
		public UnionPrototypeBuilder addPointer(StructFieldType farType, String name) {
			add(new PointerStructFieldType(farType), name);
			return this;
		}

		public UnionPrototypeBuilder addPointer(String name) {
			add(PrimitiveStructFieldType.RAW_POINTER, name);
			return this;
		}

		public UnionPrototypeBuilder addStruct(StructPrototype struct, String name) {
			add(struct, name);
			return this;
		}

		public UnionPrototypeBuilder add(StructFieldType type, String name) {
			this.fields.add(new StructField(type, name, 0));
			return this;
		}

		public UnionPrototype build() {
			return new UnionPrototype(this.fields);
		}
	}
}
