package au.edu.jcu.v4l4j.impl.jni;

public enum PrimitiveStructFieldType implements StructFieldType {
	/**
	 * bool type
	 */
	BOOL ((buffer, params) -> {
			Object value = params.apply("");
			boolean bValue;
			if (value == null)
				bValue = false;
			else if (value instanceof Boolean)
				bValue = (Boolean) value;
			else
				bValue = Boolean.valueOf(value.toString());
			buffer.put((byte) (bValue ? 0xFF : 0x00));
		},
		(buffer, params) -> params.accept("", buffer.get() != 0)),
	INT8(
			(buffer, params) -> buffer.put(((Number)params.apply("")).byteValue()),
			(buffer, params) -> params.accept("", buffer.get())),
	INT16(
			(buffer, params) -> buffer.putShort(((Number)params.apply("")).shortValue()),
			(buffer, params) -> params.accept("", buffer.getShort())),
	INT32(
			(buffer, params) -> buffer.putInt(((Number)params.apply("")).intValue()),
			(buffer, params) -> params.accept("", buffer.getInt())),
	INT64(
			(buffer, params) -> buffer.putLong(((Number)params.apply("")).longValue()),
			(buffer, params) -> params.accept("", buffer.getLong())),
	FLOAT32(
			(buffer, params) -> buffer.putFloat(((Number)params.apply("")).floatValue()),
			(buffer, params) -> params.accept("", buffer.getFloat())),
	FLOAT64(
			(buffer, params) -> buffer.putDouble(((Number)params.apply("")).doubleValue()),
			(buffer, params) -> params.accept("", buffer.getDouble())),
	RAW_POINTER(
			(buffer, params) -> {
				long value = ((Number)params.apply("")).longValue();
				if (PrimitiveStructFieldType.values()[7].getSize() == 4)
					buffer.putInt((int)value);
				else
					buffer.putLong(value);
			},
			(buffer, params) -> {
				long value;
				if (PrimitiveStructFieldType.values()[7].getSize() == 4)
					value = buffer.getInt();
				else
					value = buffer.getLong();
				params.accept("", value);
			}),
	;
	private final int alignment;
	private final int size;
	private final StructFieldType.Writer writer;
	private final StructFieldType.Reader reader;
	
	PrimitiveStructFieldType(StructFieldType.Writer writer, StructFieldType.Reader reader) {
		this.writer = writer;
		this.reader = reader;
		this.alignment = MemoryUtils.getAlignment(this.ordinal());
		this.size = MemoryUtils.getSize(this.ordinal());
	}
	
	@Override
	public int getAlignment() {
		return alignment;
	}
	
	@Override
	public int getSize() {
		return size;
	}

	@Override
	public boolean expands() {
		return false;
	}

	@Override
	public Writer writer() {
		return writer;
	}

	@Override
	public Reader reader() {
		return reader;
	}
}
