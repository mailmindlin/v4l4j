package au.edu.jcu.v4l4j.impl.jni;

import java.nio.ByteBuffer;
import java.util.function.BiConsumer;
import java.util.function.BiFunction;

public enum PrimitiveStructFieldType implements StructFieldType {
	/**
	 * bool type
	 */
	BOOL ((buffer, value) -> {
			boolean bValue;
			if (value == null)
				bValue = false;
			else if (value instanceof Boolean)
				bValue = (Boolean) value;
			else
				bValue = Boolean.valueOf(value.toString());
			buffer.put((byte) (bValue ? 0xFF : 0x00));
		},
		(buffer, params) -> (buffer.get() != 0)),
	CHAR {
		@Override
		public void write(ByteBuffer buffer, Object value) {
			char cValue;
			if (value == null)
				cValue = '\0';
			else if (value instanceof Character)
				cValue = (Character) value;
			else if (value instanceof Number) {
				cValue = (char) ((Number) value).intValue();
			} else {
				throw new IllegalArgumentException("Cannot map to char: " + value);
			}
			writeFWInt(buffer, this.getSize(), cValue);
		}
		
		@Override
		public Object read(ByteBuffer buffer, StructReadingContext context) {
			return (char) readFWInt(buffer, this.getSize());
		}
	},
	SHORT {
		@Override
		public void write(ByteBuffer buffer, Object value) {
			writeFWInt(buffer, this.getSize(), nValue(value).longValue());
		}
		
		@Override
		public Object read(ByteBuffer buffer, StructReadingContext context) {
			return readFWInt(buffer, this.getSize());
		}
	},
	INT(null, null),
	LONG(null, null),
	LONG_LONG(null, null),
	FLOAT(null, null),
	DOUBLE(null, null),
	LONG_DOUBLE(null, null),
	RAW_POINTER(
			(buffer, value) -> {
				if (PrimitiveStructFieldType.values()[7].getSize() == 4)
					buffer.putInt(((Number)value).intValue());
				else
					buffer.putLong(((Number)value).longValue());
			},
			(buffer, context) -> {
				if (PrimitiveStructFieldType.values()[7].getSize() == 4)
					return (long) buffer.getInt();
				else
					return buffer.getLong();
			}),
	INT8(
			(buffer, value) -> buffer.put(((Number)value).byteValue()),
			(buffer, context) -> ((Number) buffer.get())),
	INT16(
			(buffer, value) -> buffer.putShort(((Number)value).shortValue()),
			(buffer, context) -> buffer.getShort()),
	INT32(
			(buffer, value) -> buffer.putInt(((Number)value).intValue()),
			(buffer, context) -> buffer.getInt()),
	INT64(
			(buffer, value) -> buffer.putLong(((Number)value).longValue()),
			(buffer, context) -> buffer.getLong()),
	FLOAT32(
			(buffer, value) -> buffer.putFloat(((Number)value).floatValue()),
			(buffer, context) -> buffer.getFloat()),
	FLOAT64(
			(buffer, value) -> buffer.putDouble(((Number)value).doubleValue()),
			(buffer, context) -> buffer.getDouble()),
	;
	private final int alignment;
	private final int size;
	private final BiConsumer<ByteBuffer, Object> writer;
	private final BiFunction<ByteBuffer, StructReadingContext, Object> reader;
	
	private static long readFWInt(ByteBuffer buffer, int size) {
		switch (size) {
			case 1:
				return buffer.get();
			case 2:
				return buffer.getShort();
			case 4:
				return buffer.getInt();
			case 8:
				return buffer.getLong();
			default:
				throw new IllegalArgumentException();
		}
	}
	
	private static void writeFWInt(ByteBuffer buffer, int size, long value) {
		switch (size) {
			case 1:
				buffer.put((byte) value);
				break;
			case 2:
				buffer.putShort((short) value);
				break;
			case 4:
				buffer.putInt((int) value);
				break;
			case 8:
				buffer.putLong((long) value);
			default:
				throw new IllegalArgumentException();
		}
	}
	
	private static Number nValue(Object value) {
		if (value == null)
			return 0;
		if (value instanceof Number)
			return (Number) value;
		throw new IllegalArgumentException("Cannot map to number: " + value);
	}
	
	PrimitiveStructFieldType() {
		this.alignment = MemoryUtils.getAlignment(this.ordinal());
		this.size = MemoryUtils.getSize(this.ordinal());
		this.reader = null;
		this.writer = null;
	}
	
	PrimitiveStructFieldType(BiConsumer<ByteBuffer, Object> writer, BiFunction<ByteBuffer, StructReadingContext, Object> reader) {
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
	public void write(ByteBuffer buffer, Object params) {
		writer.accept(buffer, params);
	}
	
	@Override
	public Object read(ByteBuffer buffer, StructReadingContext context) {
		return reader.apply(buffer, context);
	}
}
