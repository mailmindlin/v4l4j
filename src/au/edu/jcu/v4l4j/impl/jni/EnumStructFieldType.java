package au.edu.jcu.v4l4j.impl.jni;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.nio.ByteBuffer;
import java.util.function.IntFunction;
import java.util.function.ToIntFunction;

public class EnumStructFieldType<T extends Enum<T>> implements StructFieldType {
	protected final IntFunction<T> mapper;
	protected final ToIntFunction<T> unmapper;
	
	public EnumStructFieldType(final Class<T> enumClass) {
		this(new IntFunction<T>() {
			T[] values;
			@Override
			public T apply(int value) {
				return values[value];
			}
			@SuppressWarnings("unchecked")
			IntFunction<T> init() {
				//Lookup values via reflection
				try {
					Method valuesMethod = enumClass.getDeclaredMethod("values", new Class<?>[0]);
					this.values = (T[]) valuesMethod.invoke(null, new Object[0]);
				} catch (NoSuchMethodException | SecurityException | IllegalAccessException | IllegalArgumentException | InvocationTargetException e) {
					throw new RuntimeException(e);
				}
				return this;
			}
		}.init(), Enum::ordinal);
	}
	
	public EnumStructFieldType(IntFunction<T> mapper, ToIntFunction<T> unmapper) {
		this.mapper = mapper;
		this.unmapper = unmapper;
	}
	
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
	public void write(ByteBuffer buffer, Object params) {
		int iVal;
		try {
			iVal = unmapper.applyAsInt((T) params);
		} catch (ClassCastException e) {
			//We can also deal with it if the param is a number
			if (params instanceof Number)
				iVal = ((Number)params).intValue();
			else
				throw e;
		}
		
		PrimitiveStructFieldType.INT32.write(buffer, iVal);
	}
	
	@Override
	public Object read(ByteBuffer buffer, StructReadingContext context) {
		Integer iValue = (Integer) PrimitiveStructFieldType.INT32.read(buffer, context);
		try {
			return mapper.apply(iValue);
		} catch (RuntimeException e) {
			return iValue;
		}
	}
}
