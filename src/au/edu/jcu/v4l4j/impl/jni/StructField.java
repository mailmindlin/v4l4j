package au.edu.jcu.v4l4j.impl.jni;

public class StructField {
	protected final String name;
	protected final StructFieldType type;

	public StructField(StructFieldType type, String name) {
		this.type = type;
		this.name = name;
	}

	public String getName() {
		return name;
	}

	public StructFieldType getType() {
		return type;
	}

	public int getSize() {
		return type.getSize();
	}
	
	public int getAlignment() {
		return type.getAlignment();
	}

	@Override
	public String toString() {
		return "StructField{name=" + getName() + ";type=" + getType() + ";size=" + getSize() + "}";
	}

	public static class PointerStructField extends StructField {
		protected final boolean dereference;
		protected final StructFieldType remote;

		public PointerStructField(boolean dereference, StructFieldType remote, String name) {
			super(PrimitiveStructFieldType.RAW_POINTER, name);
			this.dereference = dereference;
			this.remote = remote;
		}

	}
}
