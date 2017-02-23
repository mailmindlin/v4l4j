package au.edu.jcu.v4l4j.impl.jni;

import java.nio.ByteBuffer;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import java.util.Stack;
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
	public <U, V extends NativePointer<U>> V getChild(String name) {
		return (V) this.localWrappers.computeIfAbsent(name, key->{
			StructField field = type().getField(key);
			if (field == null)
				return null;
			return doWrapChild(field.getType(), field.getOffset(), field.getSize());
		});
	}
	
	@Override
	public <U, V extends NativePointer<U>> V getChildRemote(String name) {
		return (V) this.remoteWrappers.get(name);
	}
	
	/**
	 * Allocate a native memory structure on the end of a pointer
	 * @param name
	 */
	public void allocateRemote(String name) {
		final StructField field = this.struct.getField(name);
		if (field == null)
			throw new IllegalArgumentException("No such field by name " + name);
		
		StructFieldType type = field.getType();
		
		if (type instanceof PointerStructFieldType) {
			final StructFieldType farType = ((PointerStructFieldType) type).getFarType();
			
			this.remoteWrappers.compute(name, (key, oldVal) -> {
				if (oldVal != null) {
					try {
						oldVal.close();
					} catch (Exception e) {
						e.printStackTrace();
					}
				}
				
				Pointer<?> newVal = doAllocChild(farType, farType.getAlignment(), farType.getSize());
				
				return newVal;
			});
		}
	}
	
	/**
	 * Wrap a native memory structure on the end of a pointer
	 * @param name
	 */
	public void wrapFar(String name) {
		StructField field = this.struct.getField(name);
		StructFieldType type = field.getType();
		if (type instanceof PointerStructFieldType) {
			//Register this name for updates
			this.wrappedNames.add(name);
			StructFieldType farType = ((PointerStructFieldType) type).getFarType();
			//Read pointer
			long farPointer = ((Number)this.struct.readField(this.buffer, name)).longValue();
			ByteBuffer farBuffer = MemoryUtils.wrap(farPointer, farType.getSize());
			
			if (farType instanceof ArrayStructFieldType) {
				NativeArray array = new NativeArray((ArrayStructFieldType)farType, farPointer, farBuffer, false);
				this.arrays.put(name, array);
			} else if (farType instanceof StructPrototype) {
				NativeStruct struct = new NativeStruct((StructPrototype)farType, farPointer, farBuffer, false);
				this.structs.put(name, struct);
			} else if (farType instanceof UnionPrototype) {
				NativeUnion union = new NativeUnion((UnionPrototype)farType, farPointer, farBuffer, false);
				this.unions.put(name, union);
			} else {
				
			}
		}
	}

	@Override
	public Object get(Object _key) {
		return this.struct.readField(this.buffer, _key.toString());
	}

	@Override
	public Object put(String key, Object value) {
		Object old = get(key);
		this.struct.writeField(this.buffer, key, value);
		return old;
	}
	
	public void refresh() throws Exception {
		for (String name : this.wrappedNames) {
			long pointer = ((Number) this.struct.readField(this.buffer, name)).longValue();
			
			if (this.unions.containsKey(name)) {
				NativeUnion union = this.unions.get(name);
				if (pointer != union.address()) {
					UnionPrototype prototype = union.type();
					ByteBuffer oldBuffer = union.buffer();
					union.close();
					this.unions.put(name, new NativeUnion(prototype, pointer, MemoryUtils.wrap(pointer, oldBuffer.capacity()), false));
				}
			} else if (this.structs.containsKey(name)) {
				NativeStruct struct = this.structs.get(name);
				if (pointer != struct.address()) {
					StructPrototype prototype = struct.type();
					ByteBuffer oldBuffer = struct.buffer();
					struct.close();
					this.structs.put(name, new NativeStruct(prototype, pointer, oldBuffer.capacity(), false));
				}
			} else if (this.arrays.containsKey(name)) {
				NativeArray array = this.arrays.get(name);
				if (pointer != array.address()) {
					ArrayStructFieldType prototype = array.type();
					ByteBuffer oldBuffer = array.buffer();
					array.close();
					this.arrays.put(name, new NativeArray(prototype, pointer, oldBuffer.capacity(), false));
				}
			} else {
				//TODO fix for wrapped pointers
				System.err.println("Unable to refresh wrapped field '" + name + "'");
			}
		}
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
		return this.struct;
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
	public void clear() {
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
