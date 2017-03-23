package au.edu.jcu.v4l4j.impl.jni;

import java.nio.Buffer;
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
			writeFWInt(buffer, (short) cValue);
		}
		
		@Override
		public Object read(ByteBuffer buffer, StructReadingContext context) {
			return (char) readFWInt(buffer).shortValue();
		}
	},
	SHORT,
	INT,
	LONG,
	LONG_LONG,
	FLOAT {
		@Override
		public void write(ByteBuffer buffer, Object value) {
			writeFWFloat(buffer, nValue(value));
		}
		
		@Override
		public Object read(ByteBuffer buffer, StructReadingContext context) {
			return readFWFloat(buffer);
		}
	},
	DOUBLE {
		@Override
		public void write(ByteBuffer buffer, Object value) {
			writeFWFloat(buffer, nValue(value));
		}
		
		@Override
		public Object read(ByteBuffer buffer, StructReadingContext context) {
			return readFWFloat(buffer);
		}
	},
	LONG_DOUBLE {
		@Override
		public void write(ByteBuffer buffer, Object value) {
			writeFWFloat(buffer, nValue(value));
		}
		
		@Override
		public Object read(ByteBuffer buffer, StructReadingContext context) {
			return readFWFloat(buffer);
		}
	},
	RAW_POINTER {
		@Override
		public void write(ByteBuffer buffer, Object value) {
			long lValue;
			if (value instanceof Buffer) {
				if (!((Buffer)value).isDirect())
					throw new IllegalArgumentException("Cannot get pointer to non-direct buffer");
				lValue = MemoryUtils.unwrap((ByteBuffer) value);
			} else if (value instanceof NativePointer)
				lValue = ((NativePointer<?>) value).address();
			else
				lValue = nValue(value).longValue();
			writeFWInt(buffer, lValue);
		}
	},
	INT8,
	INT16,
	INT32,
	INT64,
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
	
	private static Number nValue(Object value) {
		if (value == null)
			return 0;
		if (value instanceof Number)
			return (Number) value;
		//TODO support BigInteger/BigDecimal?
		throw new IllegalArgumentException("Cannot map to number: " + value);
	}
	
	PrimitiveStructFieldType() {
		this.alignment = MemoryUtils.getAlignment(this.ordinal());
		this.size = MemoryUtils.getSize(this.ordinal());
		this.writer = (buffer, value) -> writeFWInt(buffer, nValue(value));
		this.reader = (buffer, ctx) -> readFWInt(buffer);
	}
	
	PrimitiveStructFieldType(BiConsumer<ByteBuffer, Object> writer, BiFunction<ByteBuffer, StructReadingContext, Object> reader) {
		this.writer = writer;
		this.reader = reader;
		this.alignment = MemoryUtils.getAlignment(this.ordinal());
		this.size = MemoryUtils.getSize(this.ordinal());
	}
	
	Number readFWInt(ByteBuffer buffer) {
		switch (getSize()) {
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
	
	Number readFWFloat(ByteBuffer buffer) {
		switch (getSize()) {
			case 4:
				return buffer.getFloat();
			case 8:
				return buffer.getDouble();
			default:
				throw new UnsupportedOperationException("Cannot read " + getSize() + "-byte-long float");
		}
	}
	
	void writeFWInt(ByteBuffer buffer, Number value) {
		switch (getSize()) {
			case 1:
				buffer.put(value.byteValue());
				break;
			case 2:
				buffer.putShort(value.shortValue());
				break;
			case 4:
				buffer.putInt(value.intValue());
				break;
			case 8:
				buffer.putLong(value.longValue());
			default:
				throw new IllegalArgumentException();
		}
	}
	
	void writeFWFloat(ByteBuffer buffer, Number value) {
		switch (getSize()) {
			case 4:
				buffer.putFloat(value.floatValue());
			case 8:
				buffer.putDouble(value.doubleValue());
			default:
				throw new UnsupportedOperationException("Cannot write " + getSize() + "-byte-long float");
		}
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
