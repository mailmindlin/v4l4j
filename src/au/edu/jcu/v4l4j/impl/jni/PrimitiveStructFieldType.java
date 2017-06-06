package au.edu.jcu.v4l4j.impl.jni;

import java.math.BigDecimal;
import java.math.BigInteger;
import java.nio.ByteBuffer;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicLong;

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
public class PrimitiveStructFieldType<T> implements StructFieldType<T> {
	private static final long serialVersionUID = -1407421225506639940L;
	
	//Indices for lookup by native methods. Change at your own risk.
	protected static final int
			BOOL_IDX = 0,
			
			CHAR_IDX = 1,
			SIGNED_CHAR_IDX = 2,
			UNSIGNED_CHAR_IDX = 3,
			
			SHORT_IDX = 4,
			UNSIGNED_SHORT_IDX = 5,
			
			INT_IDX = 6,
			UNSIGNED_INT_IDX = 7,
			
			LONG_IDX = 8,
			UNSIGNED_LONG_IDX = 9,
			
			LONG_LONG_IDX = 10,
			UNSIGNED_LONG_LONG_IDX = 11,
			
			FLOAT_IDX = 12,
			DOUBLE_IDX = 13,
			LONG_DOUBLE_IDX = 14,
			RAW_POINTER_IDX = 15,
			INT8_IDX = 16,
			UINT8_IDX = 17,
			INT16_IDX = 18,
			UINT16_IDX = 19,
			INT32_IDX = 20,
			UINT32_IDX = 21,
			INT64_IDX = 22,
			UINT64_IDX = 23,
			FLOAT32_IDX = 24,
			FLOAT64_IDX = 25;
	
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
		
		FLOAT32(),
		FLOAT64(),
		;
		
		private final boolean fp;
		private final int size;
		private final int alignment;
		/**
		 * Minimum storable value.
		 * If FP, then # of Mantissa digits
		 */
		private final long minValue;
		/**
		 * Maximum storable value.
		 * If FP, then 
		 */
		private final long maxValue;
		PrimitiveInfo() {
			//TODO fix
			//throw new UnsupportedOperationException("please finish me.");
			this(-1, -1, -1, -1);
		}
		PrimitiveInfo(int size, int alignment, long min, long max) {
			this.fp = false;
			this.size = size;
			this.alignment = alignment;
			this.minValue = min;
			this.maxValue = max;
		}
		
		PrimitiveInfo(int size, int alignment, boolean signed) {
			this.fp = false;
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
	
	public static PrimitiveStructFieldType<?>[] values() {
		return new PrimitiveStructFieldType[] {
				BOOL,
				CHAR,
				SCHAR,
				UCHAR,
				SHORT,
				USHORT,
				INT,
				UINT,
				LONG,
				ULONG,
				LLONG,
				ULLONG,
				FLOAT,
				DOUBLE,
				LONG_DOUBLE,
				RAW_POINTER,
				INT8,
				UINT8,
				INT16,
				UINT16,
				INT32,
				UINT32,
				INT64,
				UINT64,
				FLOAT32,
				FLOAT64
		};
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
	 * 1 byte.
	 */
	public static final PrimitiveStructFieldType<Number> CHAR = new PrimitiveStructFieldType<Number>(CHAR_IDX);
	
	public static final PrimitiveStructFieldType<Number> SCHAR = new PrimitiveStructFieldType<Number>(SIGNED_CHAR_IDX);
	
	public static final PrimitiveStructFieldType<Number> UCHAR = new PrimitiveStructFieldType<Number>(UNSIGNED_CHAR_IDX);
	
	public static final PrimitiveStructFieldType<Number> SHORT = new PrimitiveStructFieldType<Number>(SHORT_IDX);
	public static final PrimitiveStructFieldType<Number> USHORT = new PrimitiveStructFieldType<Number>(UNSIGNED_SHORT_IDX);
	
	public static final PrimitiveStructFieldType<Number> INT = new PrimitiveStructFieldType<Number>(INT_IDX);
	public static final PrimitiveStructFieldType<Number> UINT = new PrimitiveStructFieldType<Number>(UNSIGNED_INT_IDX);
	
	public static final PrimitiveStructFieldType<Number> LONG = new PrimitiveStructFieldType<Number>(LONG_IDX);
	public static final PrimitiveStructFieldType<Number> ULONG = new PrimitiveStructFieldType<Number>(UNSIGNED_LONG_IDX);
	
	public static final PrimitiveStructFieldType<Number> LLONG = new PrimitiveStructFieldType<Number>(LONG_LONG_IDX);
	public static final PrimitiveStructFieldType<Number> ULLONG = new PrimitiveStructFieldType<Number>(UNSIGNED_LONG_LONG_IDX);
	
	public static final PrimitiveStructFieldType<Float> FLOAT = new PrimitiveStructFieldType<Float>(FLOAT_IDX, true);
	public static final PrimitiveStructFieldType<Number> DOUBLE = new PrimitiveStructFieldType<Number>(DOUBLE_IDX, true);
	public static final PrimitiveStructFieldType<Number> LONG_DOUBLE = new PrimitiveStructFieldType<Number>(LONG_DOUBLE_IDX, true);
	
	public static final PrimitiveStructFieldType<Number> RAW_POINTER = new PrimitiveStructFieldType<Number>(RAW_POINTER_IDX);
	
	public static final PrimitiveStructFieldType<Number> INT8 = new PrimitiveStructFieldType<Number>(INT8_IDX);
	public static final PrimitiveStructFieldType<Number> UINT8 = new PrimitiveStructFieldType<Number>(UINT8_IDX);
	
	public static final PrimitiveStructFieldType<Number> INT16 = new PrimitiveStructFieldType<Number>(INT16_IDX);
	public static final PrimitiveStructFieldType<Number> UINT16 = new PrimitiveStructFieldType<Number>(UINT16_IDX);
	
	public static final PrimitiveStructFieldType<Number> INT32 = new PrimitiveStructFieldType<Number>(INT32_IDX);
	public static final PrimitiveStructFieldType<Number> UINT32 = new PrimitiveStructFieldType<Number>(UINT32_IDX);
	
	public static final PrimitiveStructFieldType<Number> INT64 = new PrimitiveStructFieldType<Number>(INT64_IDX);
	public static final PrimitiveStructFieldType<Number> UINT64 = new PrimitiveStructFieldType<Number>(UINT64_IDX);
	
	public static final PrimitiveStructFieldType<Number> FLOAT32 = new PrimitiveStructFieldType<Number>(FLOAT32_IDX, true);
	public static final PrimitiveStructFieldType<Number> FLOAT64 = new PrimitiveStructFieldType<Number>(FLOAT64_IDX, true);
	
	protected final int idx;
	protected final boolean isFP;
	protected final int alignment;
	protected final int size;
	
	protected PrimitiveStructFieldType(int idx) {
		this(idx, false);
	}
	
	protected PrimitiveStructFieldType(int idx, boolean isFP) {
		this.idx = idx;
		this.isFP = isFP;
		if (!MemoryUtils.isBound()) {
			PrimitiveInfo defaultInfo = PrimitiveInfo.values()[idx];
			this.alignment = defaultInfo.getDefaultAlignment();
			this.size = defaultInfo.getDefaultSize();
		} else {
			this.alignment = MemoryUtils.getAlignment(idx);
			this.size = MemoryUtils.getSize(idx);
		}
	}
	
	/**
	 * Map a number to BigIngeger
	 * @param n
	 * @return
	 */
	protected static BigInteger mapToBigInteger(Number n, boolean force) {
		if (n instanceof BigInteger)
			return (BigInteger) n;
		
		if (force
				|| (n instanceof Byte)
				|| (n instanceof Short)
				|| (n instanceof Integer)
				|| (n instanceof AtomicInteger)
				|| (n instanceof Long)
				|| (n instanceof AtomicLong))
			return BigInteger.valueOf(n.longValue());
		
		return null;
	}
	
	protected static BigDecimal mapToBigDecimal(Number n, boolean force) {
		if (n instanceof BigDecimal)
			return (BigDecimal) n;
		
		if (force || n instanceof Float || n instanceof Double)
			return new BigDecimal(n.doubleValue());
		
		return null;
	}
	
	@Override
	public void write(ByteBuffer buffer, T value) {
		if (this.isFP)
			writeFWFloat(buffer, (Number) value);
		else
			writeFWInt(buffer, (Number) value);
	}
	
	@Override
	@SuppressWarnings("unchecked")
	public T read(ByteBuffer buffer, StructReadingContext context) {
		if (this.isFP)
			return (T) readFWFloat(buffer);
		else
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
				//TODO fix for unsigned stuff
				byte[] bytes = new byte[this.getSize()];
				buffer.get(bytes, 0, bytes.length);
				return new BigInteger(bytes);
		}
	}
	
	Number readFWFloat(ByteBuffer buffer) {
		switch (getSize()) {
			case 4:
				return buffer.getFloat();
			case 8:
				return buffer.getDouble();
			default:
				//TODO fix for 80-bit long doubles
				throw new UnsupportedOperationException("Cannot read " + getSize() + "-byte-long float");
		}
	}
	
	void writeFWInt(ByteBuffer buffer, Number value) {
		final int size = this.getSize();
		switch (size) {
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
				if (value instanceof BigInteger) {
					byte[] bytes = ((BigInteger) value).toByteArray();
					//TODO fix byte[] size
					if (bytes.length != size)
						throw new IllegalArgumentException("Bad length: " + bytes.length + "; expected: " + size);
					buffer.put(bytes);
					break;
				}
				throw new IllegalArgumentException("Can't deal with this.");
		}
	}
	
	void writeFWFloat(ByteBuffer buffer, Number value) {
		switch (getSize()) {
			case 4:
				buffer.putFloat(value.floatValue());
			case 8:
				buffer.putDouble(value.doubleValue());
			default:
				throw new UnsupportedOperationException("Cannot write " + getSize() + "-byte-long fp value");
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
	
	public String name() {
		return PrimitiveInfo.values()[this.idx].name();
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
