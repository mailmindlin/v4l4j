package au.edu.jcu.v4l4j.impl.jni;

import java.nio.ByteBuffer;
import java.util.Collection;
import java.util.Map;
import java.util.Set;
import java.util.stream.Collectors;

public class NativeStruct extends NativeWrapper<String, Map<String, Object>> implements Map<String, Object> {
	
	public NativeStruct(StructPrototype struct) {
		this(struct, MemoryUtils.alloc(struct.getAlignment(), struct.getSize()), struct.getSize(), true);
	}
	
	public NativeStruct(StructPrototype struct, long address, long length) {
		this(struct, address, length, false);
	}
	
	protected NativeStruct(StructPrototype struct, long address, long length, boolean deallocOnClose) {
		this(struct, address, MemoryUtils.wrap(address, length), deallocOnClose);
	}
	
	public NativeStruct(StructPrototype struct, ByteBuffer buffer) {
		this(struct, MemoryUtils.unwrap(buffer), buffer, false);
	}
	
	protected NativeStruct(StructPrototype struct, long address, ByteBuffer buffer, boolean freeOnClose) {
		super(struct, address, buffer, freeOnClose);
	}

	@Override
	public boolean isEmpty() {
		return false;
	}

	@Override
	public boolean containsKey(Object key) {
		throw new UnsupportedOperationException();
	}

	@Override
	@SuppressWarnings("unchecked")
	public <U, V extends NativePointer<U>> V getChild(String name) {
		return (V) this.localWrappers.computeIfAbsent(name, key->{
			StructField field = type().getField(key);
			if (field == null)
				return null;
			return doWrapLocalChild(field.getType(), field.getOffset(), field.getSize());
		});
	}

	/**
	 * Allocate a native memory structure on the end of a pointer
	 * @param name
	 */
	@Override
	public void allocChildRemote(String name) {
		final StructField field = this.type().getField(name);
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
	
	/**
	 * Wrap a native memory structure on the end of a pointer
	 * @param name
	 */
	@Override
	public void wrapChildRemote(String name) {
		StructField field = this.type().getField(name);
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
	public Object get(Object _key) {
		return this.type().readField(this.buffer, _key.toString());
	}

	@Override
	public Object put(String key, Object value) {
		Object old = get(key);
		this.type().writeField(this.buffer, key, value);
		return old;
	}
	
	public void refresh() throws Exception {
		this.wrappedNames.forEach(this::wrapChildRemote);
	}

	@Override
	public void putAll(Map<? extends String, ? extends Object> m) {
		for (Entry<? extends String, ? extends Object> e : m.entrySet())
			this.put(e.getKey(), e.getValue());
	}

	@Override
	public Set<String> keySet() {
		return this.type().fields().stream()
				.map(StructField::getName)
				.collect(Collectors.toSet());
	}
	
	@Override
	public StructPrototype type() {
		return (StructPrototype) this.type;
	}

	@Override
	public Collection<Object> values() {
		throw new UnsupportedOperationException();
	}

	@Override
	public Set<Entry<String, Object>> entrySet() {
		throw new UnsupportedOperationException();
	}

	@Override
	public Object remove(Object key) {
		throw new UnsupportedOperationException();
	}

	@Override
	public int size() {
		//I really don't want to implement this now
		throw new UnsupportedOperationException("I'm lazy");
	}

	@Override
	public boolean containsValue(Object value) {
		throw new UnsupportedOperationException();
	}
}
