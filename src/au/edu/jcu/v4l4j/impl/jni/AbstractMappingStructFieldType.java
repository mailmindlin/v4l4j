package au.edu.jcu.v4l4j.impl.jni;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.nio.ByteBuffer;
import java.util.function.IntFunction;
import java.util.function.ToIntFunction;

public abstract class AbstractMappingStructFieldType<T> implements StructFieldType<T> {
	private static final long serialVersionUID = 8797715570154592010L;

	protected abstract T map(int iValue);
	protected abstract int unmap(T value);
	
	@Override
	public int getAlignment() {
		return PrimitiveStructFieldType.INT32.getAlignment();
	}
	
	@Override
	public int getSize() {
		return PrimitiveStructFieldType.INT32.getSize();
	}
	
	@Override
	public boolean expands() {
		return false;
	}

	@Override
	@SuppressWarnings("unchecked")
	public void write(ByteBuffer buffer, Object params) {
		//Map params to int
		int iVal;
		try {
			iVal = unmap((T) params); 
		} catch (ClassCastException e) {
			//We can also deal with it if the param is a number
			if (params != null && params instanceof Number)
				iVal = ((Number)params).intValue();
			else
				throw e;
		}
		
		//Write as int32 (TODO: check if this is system-dependent)
		PrimitiveStructFieldType.INT32.write(buffer, iVal);
	}

	@Override
	@SuppressWarnings("unchecked")
	public T read(ByteBuffer buffer, StructReadingContext context) {
		Integer iValue = (Integer) PrimitiveStructFieldType.INT32.read(buffer, context);
		
		//Map to enum constant
		try {
			return map(iValue);
		} catch (RuntimeException e) {
			//Mapping failed, return raw value
			//TODO fix casting stuff
			return (T) (Integer) iValue;
		}
	}
	
	public static class EnumStructFieldType<T extends Enum<T>> extends AbstractMappingStructFieldType<T> {
		private static final long serialVersionUID = 1L;

		protected final Class<T> enumClass;
		
		private transient T[] values = null;
		
		public EnumStructFieldType(final Class<T> enumClass) {
			this.enumClass = enumClass;
		}
		
		@SuppressWarnings("unchecked")
		@Override
		protected T map(int iValue) {
			T[] values = this.values;
			if (values == null) {
				synchronized (this) {
					if (this.values == null) {
						try {
							//Lookup values via reflection
							/*MethodHandle mh = MethodHandles.lookup().findStatic(enumClass, "values", MethodType.methodType(enumClass));
							mh.invoke(null);*/
							Method valuesMethod = enumClass.getDeclaredMethod("values", new Class<?>[0]);
							this.values = (T[]) valuesMethod.invoke(null, new Object[0]);
						} catch (NoSuchMethodException | SecurityException | IllegalAccessException | IllegalArgumentException | InvocationTargetException e) {
							throw new RuntimeException(e);
						}
					}
				}
				values = this.values;
			}
			if (iValue > values.length || iValue < 0)
				throw new IllegalArgumentException("Cannot map " + iValue + " to value of enum " + enumClass);
			return values[iValue];
		}

		@Override
		protected int unmap(T value) {
			return value.ordinal();
		}
	}
	
	public static class MappingStructFieldType<T> extends AbstractMappingStructFieldType<T> {
		private static final long serialVersionUID = 1L;
		protected final IntFunction<T> mapper;
		protected final ToIntFunction<T> unmapper;
		
		public MappingStructFieldType(IntFunction<T> mapper, ToIntFunction<T> unmapper) {
			this.mapper = mapper;
			this.unmapper = unmapper;
		}

		@Override
		protected T map(int iValue) {
			return mapper.apply(iValue);
		}

		@Override
		protected int unmap(T value) {
			return unmapper.applyAsInt(value);
		}
	}
}
