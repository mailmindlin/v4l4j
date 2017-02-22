package au.edu.jcu.v4l4j.impl.jni;

import java.lang.ref.WeakReference;
import java.nio.ByteBuffer;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

public class StructMap implements Map<String, Object>, AutoCloseable, NativeWrapper {
	protected final StructPrototype struct;
	protected final long pointer;
	protected final ByteBuffer buffer;
	/**
	 * Lookup for far structs (accessible through a pointer) that we allocated and linked
	 */
	protected final Map<String, StructMap> structs = new HashMap<>();
	protected final Map<String, NativeArray> arrays = new HashMap<>();
	protected final Map<String, NativeUnion> unions = new HashMap<>();
	/**
	 * Set of names that have been wrapped by Java, and could
	 * be broken by an external write
	 */
	protected final Set<String> wrappedNames = new HashSet<>();
	/**
	 * Set of objects that we have to release when closing
	 */
	protected final Set<AutoCloseable> managedRefs = new HashSet<>();
	
	public StructMap(StructPrototype struct) {
		this(struct, MemoryUtils.alloc(struct.getSize()), struct.getSize());
	}
	
	public StructMap(StructPrototype struct, long pointer, long length) {
		this(struct, pointer, MemoryUtils.wrap(pointer, length));
	}
	
	public StructMap(StructPrototype struct, ByteBuffer buffer) {
		this(struct, MemoryUtils.unwrap(buffer), buffer);
	}
	
	protected StructMap(StructPrototype struct, long pointer, ByteBuffer buffer) {
		this.struct = struct;
		this.pointer = pointer;
		this.buffer = buffer;
	}
	
	@Override
	public int size() {
		//I really don't want to implement this now
		throw new UnsupportedOperationException("I'm lazy");
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
	public boolean containsValue(Object value) {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public Object get(Object _key) {
		return this.struct.readField(this.buffer, _key.toString());
	}
	
	public NativeUnion getUnion(String name) {
		return this.unions.computeIfAbsent(name, key -> {
			StructField field = this.struct.getField(key);
			StructFieldType type = field.getType();
			if (type instanceof UnionPrototype) {
				//Near fixed-size array
				ByteBuffer dup = MemoryUtils.sliceBuffer(this.buffer, field.getOffset(), field.getSize());
				return new NativeUnion((UnionPrototype) type, dup);
			} else if (type instanceof PointerStructFieldType) {
				throw new IllegalStateException("Far pointer is not wrapped/allocated; you may want to call allocateFar/wrapFar");
			} else {
				throw new IllegalArgumentException("Field " + key + " is not an union or pointer (is " + type + ")");
			}
		});
	}
	
	public NativeArray getArray(String name) {
		return this.arrays.computeIfAbsent(name, key -> {
			StructField field = this.struct.getField(key);
			StructFieldType type = field.getType();
			if (type instanceof ArrayStructFieldType) {
				//Near fixed-size array
				ByteBuffer dup = MemoryUtils.sliceBuffer(this.buffer, field.getOffset(), field.getSize());
				return new NativeArray((ArrayStructFieldType) type, dup);
			} else if (type instanceof PointerStructFieldType) {
				throw new IllegalStateException("Far pointer is not wrapped/allocated; you may want to call allocateFar/wrapFar");
			} else {
				throw new IllegalArgumentException("Field " + key + " is not an array or pointer(is " + type + ")");
			}
		});
	}
	
	/**
	 * Allocate a native memory structure on the end of a pointer
	 * @param name
	 */
	public void allocateFar(String name) {
		StructField field = this.struct.getField(name);
		StructFieldType type = field.getType();
		
	}
	
	/**
	 * Wrap a native memory structure on the end of a pointer
	 * @param name
	 */
	public void wrapFar(String name) {
		
	}
	
	public StructMap getStruct(String name) {
		return this.structs.computeIfAbsent(name, key->{
			StructField field = this.struct.getField(key);
			StructFieldType type = field.getType();
			if (type instanceof StructPrototype) {
				//Near fixed-size array
				ByteBuffer dup = MemoryUtils.sliceBuffer(this.buffer, field.getOffset(), field.getSize());
				return new StructMap((StructPrototype) type, dup);
			} else if (type instanceof PointerStructFieldType) {
				throw new IllegalStateException("Far pointer is not wrapped/allocated; you may want to call allocateFar/wrapFar");
			} else {
				throw new IllegalArgumentException("Field " + key + " is not a struct or pointer (is " + type + ")");
			}
		});
	}

	@Override
	public Object put(String key, Object value) {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Object remove(Object key) {
		throw new UnsupportedOperationException();
	}

	@Override
	public void putAll(Map<? extends String, ? extends Object> m) {
		for (Entry<? extends String, ? extends Object> e : m.entrySet())
			this.put(e.getKey(), e.getValue());
	}

	@Override
	public void clear() {
		throw new UnsupportedOperationException();
	}

	@Override
	public Set<String> keySet() {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Collection<Object> values() {
		throw new UnsupportedOperationException();
	}

	@Override
	public Set<Entry<String, Object>> entrySet() {
		// TODO Auto-generated method stub
		return null;
	}
	
	@Override
	public void close() throws Exception {
		
	}
	
	@Override
	public StructPrototype type() {
		return this.struct;
	}
	
	@Override
	public long pointer() {
		return this.pointer;
	}
	
	@Override
	public ByteBuffer buffer() {
		return this.buffer;
	}
}
