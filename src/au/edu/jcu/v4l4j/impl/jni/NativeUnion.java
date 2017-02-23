package au.edu.jcu.v4l4j.impl.jni;

import java.nio.ByteBuffer;
import java.util.Collection;
import java.util.Map;
import java.util.Set;

public class NativeUnion<T> extends NativeWrapper<String, T> implements Map<String, Object> {
	
	public NativeUnion(UnionPrototype union, ByteBuffer buffer) {
		this(union, MemoryUtils.unwrap(buffer), buffer, false);
	}
	
	protected NativeUnion(UnionPrototype union, long pointer, ByteBuffer buffer, boolean releaseOnClose) {
		super(union, pointer, buffer, releaseOnClose);
	}
	
	@Override
	public UnionPrototype type() {
		return (UnionPrototype) this.type;
	}

	@Override
	public ByteBuffer buffer() {
		return this.buffer;
	}

	@Override
	public int size() {
		return 0;
	}

	@Override
	public boolean isEmpty() {
		return false;
	}

	@Override
	public boolean containsKey(Object key) {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	@SuppressWarnings("unchecked")
	public <U, V extends NativePointer<U>> V getChild(String name) {
		return (V) this.localWrappers.computeIfAbsent(name, key->{
			StructField option = type().getOption(key);
			if (option == null)
				return null;
			return doWrapLocalChild(option.getType(), option.getOffset(), option.getSize());
		});
	}

	@Override
	public Object get(Object key) {
		return this.type().readField(this.buffer(), key.toString());
	}
	
	@Override
	public Object put(String key, Object value) {
		Object old = this.get(key);
		this.type().writeField(this.buffer(), key, value);
		return old;
	}
	
	@Override
	public void wrapChildRemote(String name) {
		StructField field = this.type().getOption(name);
		StructFieldType type = field.getType();
		if (!(type instanceof PointerStructFieldType))
			throw new IllegalArgumentException("Field '" + name + "' is not a pointer");
		
		//Register this name for updates
		this.wrappedNames.add(name);
		final StructFieldType farType = ((PointerStructFieldType) type).getFarType();

		//Read pointer
		final long farPointer = ((Number)this.get(name)).longValue();
		
		this.remoteWrappers.compute(name, (key, oldValue) -> {
			if (oldValue != null) {
				//Check if the old pointer is still valid
				if (oldValue.address() == farPointer)
					return oldValue;
				//Release the old pointer
				try {
					oldValue.close();
					this.managedRefs.remove(oldValue);
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
			
			ByteBuffer farBuffer = MemoryUtils.wrap(farPointer, farType.getSize());
			return doWrapPointer(farType, farPointer, farBuffer, false);
		});
	}

	@Override
	public void allocChildRemote(String name) {
		final StructField field = this.type().getOption(name);
		if (field == null)
			throw new IllegalArgumentException("No such field called '" + name + "'");
		
		StructFieldType type = field.getType();
		
		if (type instanceof PointerStructFieldType) {
			final StructFieldType farType = ((PointerStructFieldType) type).getFarType();
			
			this.remoteWrappers.compute(name, (key, oldVal) -> {
				if (oldVal != null) {
					try {
						oldVal.close();
						this.managedRefs.remove(oldVal);
					} catch (Exception e) {
						e.printStackTrace();
					}
				}
				
				NativePointer<?> newVal = doAllocChild(farType, farType.getAlignment(), farType.getSize());
				
				//Update local pointer
				this.put(key, newVal.address());
				
				return newVal;
			});
		}
	}

	@Override
	public void putAll(Map<? extends String, ? extends Object> m) {
		for (Entry<? extends String, ? extends Object> e : m.entrySet())
			this.put(e.getKey(), e.getValue());
	}

	@Override
	public Set<String> keySet() {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public void clear() {
		// TODO Auto-generated method stub
		
	}

	@Override
	public Object remove(Object key) {
		throw new UnsupportedOperationException();
	}

	@Override
	public boolean containsValue(Object value) {
		throw new UnsupportedOperationException();
	}

	@Override
	public Collection<Object> values() {
		throw new UnsupportedOperationException();
	}

	@Override
	public Set<Entry<String, Object>> entrySet() {
		throw new UnsupportedOperationException();
	}
}
