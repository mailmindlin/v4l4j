package au.edu.jcu.v4l4j.impl.jni;

import java.nio.ByteBuffer;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicLong;

import au.edu.jcu.v4l4j.api.Rational;

/**
 * Primitive types:
 * bool
 * char
 *  - signed char
 *  - unsigned char
 * short
 *  - signed short
 *  - unsigned short
 * int
 *  - unsigned int
 * long
 *  - unsigned long
 * long long (signed long long)
 *  - unsigned long long
 * float
 * double
 * long double
 * uintptr
 * int8
 * int16
 * int32
 * int64
 * float32
 * float64
 * @author mailmindlin
 *
 * @param <T>
 */
public abstract class PrimitiveStructFieldType<T> implements StructFieldType<T> {
	private static final long serialVersionUID = -1407421225506639940L;
	
	//Indices for lookup by native methods. Change at your own risk.
	protected static final int
			BOOL_IDX = 0,
			
			CHAR_IDX = 1,
			SIGNED_CHAR_IDX = 2,
			UNSIGNED_CHAR_IDX = 3,
			
			SHORT_IDX = 4,
			UNSIGNED_SHORT_IDX = 6,
			
			INT_IDX = 7,
			UNSIGNED_INT_IDX = 9,
			
			LONG_IDX = 4,
			UNSIGNED_LONG_IDX = -1,
			
			LONG_LONG_IDX = 5,
			UNSIGNED_LONG_LONG_IDX = -1,
			
			FLOAT_IDX = 6,
			DOUBLE_IDX = 7,
			LONG_DOUBLE_IDX = 8,
			RAW_POINTER_IDX = 9,
			INT8_IDX = 10,
			INT16_IDX = 11,
			INT32_IDX = 12,
			INT64_IDX = 13,
			FLOAT32_IDX = 14,
			FLOAT64_IDX = 15;
	
	/**
	 * Default values for sizes/alignments of primitives. Used when we can't invoke the native code.
	 * <strong>Change at your own risk.</strong>
	 */
	protected static enum PrimitiveInfo {
		BOOL(1, 1, 0, 1),
		
		CHAR(2, 2, 0, (1 << 8) - 1),
		SCHAR(2, 2, true),
		UCHAR(2, 2, false),
		
		SHORT(2, 2, true),
		USHORT(2, 2, false),
		
		//TODO int only guarunteed range of 2 bytes, but in practice it's (almost) always 4; which should we use?
		INT(4, 4, true),
		UINT(4, 4, false),
		
		LONG(4, 4, true),
		ULONG(4, 4, false),
		
		LLONG(8, 8, true),
		ULLONG(8, 8, false),
		
		FLOAT(),
		DOUBLE(),
		LONG_DOUBLE(),
		RAW_POINTER(),
		
		INT8(1, 1, true),
		UINT8(1, 1, false),
		INT16(2, 2, true),
		UINT16(2, 2, false),
		INT32(4, 4, true),
		UINT32(4, 4, false),
		INT64(8, 8, true),
		UINT64(8, 8, false),
		;
		
		private final int size;
		private final int alignment;
		private final long minValue;
		private final long maxValue;
		PrimitiveInfo() {
			//TODO fix
			throw new UnsupportedOperationException("please finish me.");
		}
		PrimitiveInfo(int size, int alignment, long min, long max) {
			this.size = size;
			this.alignment = alignment;
			this.minValue = min;
			this.maxValue = max;
		}
		
		PrimitiveInfo(int size, int alignment, boolean signed) {
			this.size = size;
			this.alignment = alignment;
			if (signed) {
				long v = 1L << (size * 8 - 1);
				this.minValue = 1 - v;
				this.maxValue = v - 1;
			} else {
				this.minValue = 0;
				this.maxValue = 1L << (size * 8);
			}
		}
		public int getDefaultSize() {
			return this.size;
		}
		
		public int getDefaultAlignment() {
			return this.alignment;
		}
	}
	protected static final int
			BOOL_DEFAULT_SIZE = 1,
			CHAR_DEFAULT_SIZE = Character.BYTES,
			SHORT_DEFAULT_SIZE = Short.BYTES,
			INT_DEFAULT_SIZE = Integer.BYTES,
			LONG_DEFAULT_SIZE = Long.BYTES,
			LONG_LONG_DEFAULT_SIZE = Long.BYTES
			;
	
	static byte[] getNumberBytes(Number n) {
		if (n instanceof Byte)
			return new byte[]{(Byte)n};
		if (n instanceof Short) {
			short s = n.shortValue();
			return new byte[]{(byte) ((s & 0xFF00) >>> 8), (byte) (s & 0x00FF)};
		}
		if (n instanceof Integer) {
			int i = n.intValue();
			return new byte[]{(byte) ((i >>> 24) & 0xFF), (byte) ((i >>> 16) & 0xFF), (byte) ((i >>> 8) & 0xFF), (byte) (i& 0xFF)};
		}
		//TODO finish;
		return null;
	}
	
	public static final PrimitiveStructFieldType<Boolean> BOOL = new PrimitiveStructFieldType<Boolean>(BOOL_IDX) {
		private static final long serialVersionUID = 948638586057437605L;
		
		@Override
		public void writeUnchecked(ByteBuffer buffer, Object params) {
			boolean bValue;
			if (params == null)
				bValue = false;
			else if (params instanceof Boolean)
				bValue = ((Boolean) params).booleanValue();
			else
				bValue = Boolean.valueOf(params.toString());
			this.write(buffer, bValue);
		}
		
		@Override
		public void write(ByteBuffer buffer, Boolean params) {
			boolean bValue = params == null ? false : (boolean) params;
			this.writeFWInt(buffer, bValue ? 1 : 0);
		}
		
		@Override
		public Boolean read(ByteBuffer buffer, StructReadingContext context) {
			return this.readFWInt(buffer).intValue() != 0;
		}
	};
	
	
	/**
	 * C character type.
	 * Range 
	 */
	public static final PrimitiveStructFieldType<Number> CHAR = new PrimitiveStructFieldType<Number>(CHAR_IDX) {
		private static final long serialVersionUID = 1236880998037556331L;
		
		@Override
		public void writeUnchecked(ByteBuffer buffer, Object value) {
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
			this.write(buffer, cValue);
		}
		
		@Override
		public Number read(ByteBuffer buffer, StructReadingContext context) {
			return readFWInt(buffer).shortValue();
		}
		
	};
	
	public static final PrimitiveStructFieldType<Byte> BYTE = new PrimitiveStructFieldType<Byte>(-1) {
		private static final long serialVersionUID = -6996477397334889469L;

		@Override
		public void write(ByteBuffer buffer, Byte params) {
			// TODO Auto-generated method stub
			
		}

		@Override
		public Byte read(ByteBuffer buffer, StructReadingContext context) {
			return null;
		}
		
	};
	
	public static final PrimitiveStructFieldType<Float> FLOAT = new PrimitiveStructFieldType<Float>(FLOAT_IDX) {
		private static final long serialVersionUID = 997689072475219270L;

		@Override
		public void write(ByteBuffer buffer, Float value) {
			writeFWFloat(buffer, value);
		}
		
		@Override
		public Float read(ByteBuffer buffer, StructReadingContext context) {
			return readFWFloat(buffer).floatValue();
		}
	};
	
	public static final PrimitiveStructFieldType<Double> DOUBLE = new PrimitiveStructFieldType<Double>(DOUBLE_IDX) {
		private static final long serialVersionUID = 997689072475219270L;
		
		@Override
		public void write(ByteBuffer buffer, Double value) {
			writeFWFloat(buffer, value);
		}
		
		@Override
		public Double read(ByteBuffer buffer, StructReadingContext context) {
			return readFWFloat(buffer).doubleValue();
		}
	};
	
	public static final PrimitiveStructFieldType<Double> LONG_DOUBLE = new PrimitiveStructFieldType<Double>(LONG_DOUBLE_IDX) {
		private static final long serialVersionUID = 997689072475219270L;
		
		@Override
		public void write(ByteBuffer buffer, Double value) {
			writeFWFloat(buffer, value);
		}
		
		@Override
		public Double read(ByteBuffer buffer, StructReadingContext context) {
			return readFWFloat(buffer).doubleValue();
		}
	};
	
	public static final PrimitiveStructFieldType<Double> RAW_POINTER = new PrimitiveStructFieldType<Double>(RAW_POINTER_IDX) {
		private static final long serialVersionUID = 997689072475219270L;
		
		@Override
		public void write(ByteBuffer buffer, Double value) {
			//TODO finish
			writeFWFloat(buffer, value);
		}
		
		@Override
		public Double read(ByteBuffer buffer, StructReadingContext context) {
			return readFWFloat(buffer).doubleValue();
		}
	};
	
	public static final PrimitiveStructFieldType<Double> INT8 = new PrimitiveStructFieldType<Double>(INT8_IDX) {
		private static final long serialVersionUID = 1585078969687614320L;
	};
	
	
	protected final int alignment;
	protected final int size;
	
	protected PrimitiveStructFieldType(int idx) {
		this.alignment = MemoryUtils.getAlignment(idx);
		this.size = MemoryUtils.getSize(idx);
	}
	
	protected PrimitiveStructFieldType(int idx, int defaultSize, int defaultAlignment) {
		if (!MemoryUtils.isBound()) {
			this.alignment = defaultAlignment;
			this.size = defaultSize;
		} else {
			this.alignment = MemoryUtils.getAlignment(idx);
			this.size = MemoryUtils.getSize(idx);
		}
	}
	
	protected byte[] mapIntInt(Number n, int maxSizeBytes, boolean signed) {
		if (maxSizeBytes < 1)
			throw new IllegalArgumentException();
		
		long v = n.longValue();
		if (signed) {
			if (v <= -(1L << (maxSizeBytes * 8 - 1)) || v >= (1L << (maxSizeBytes * 8 - 1)))
				throw new IllegalArgumentException("Value " + v + " outside range (1 - 2**" + (maxSizeBytes - 1) + ", 2**" +  + (maxSizeBytes - 1) + " - 1)");
		} else {
			if (v < 0 || v >= (1L << (maxSizeBytes * 8)))
				throw new IllegalArgumentException("Value " + v + " outside range (0, 2**" +  + maxSizeBytes + " - 1)");
		}
		
		/*
		if (n instanceof Short) {
			short s = n.shortValue();
			if (maxSizeBytes == 1) {
				if (signed) {
					if (s < 0 || s >= (1 << 8))
						throw new IllegalArgumentException("Value " + s + " outside range (0, 2**8)");
				} else {
					if (s <= -(1 << 7) || s >= (1 << 7))
						throw new IllegalArgumentException("Value " + s + " outside range (1 - 2**7, 2**7 - 1)");
				}
				return mapInt((byte) (s & 0xFF), maxSizeBytes, signed);
			}
			return new byte[] {(byte) ((s & 0xFF00) >>> 8), (byte) (s & 0xFF)};
		} else if ((n instanceof Integer) || (n instanceof AtomicInteger)) {
			int i = n.intValue();
			switch (maxSizeBytes) {
				case 1: {
					if (signed) {
						if (s < 0 || s >= (1 << 8))
							throw new IllegalArgumentException("Value " + s + " outside range (0, 2**8)");
					} else {
						if (s <= -(1 << 7) || s >= (1 << 7))
							throw new IllegalArgumentException("Value " + s + " outside range (1 - 2**7, 2**7 - 1)");
					}
					return mapInt((byte) (s & 0xFF), maxSizeBytes, signed);
				}
			}
		} else if ((n instanceof Long) || (n instanceof AtomicLong)) {
			
		} else if (n instanceof Float) {
			
		} else if (n instanceof Double) {
			
		} else if (n instanceof Rational) {
			
		} else {
			
		}
		*/
		return null;
	}
	
	@Override
	public void write(ByteBuffer buffer, T value) {
		writeFWInt(buffer, (Number) value);
	}
	
	@Override
	@SuppressWarnings("unchecked")
	public T read(ByteBuffer buffer, StructReadingContext context) {
		return (T) readFWInt(buffer);
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
		return this.alignment;
	}
	
	@Override
	public int getSize() {
		return this.size;
	}
	
	@Override
	public boolean expands() {
		return false;
	}
	
	
	/*BOOL ((buffer, value) -> {
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
	
	@Override
	public void write(ByteBuffer buffer, Object params) {
		writer.accept(buffer, params);
	}
	
	@Override
	public Object read(ByteBuffer buffer, StructReadingContext context) {
		return reader.apply(buffer, context);
	}*/
}
