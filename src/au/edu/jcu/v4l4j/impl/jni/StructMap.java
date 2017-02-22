package au.edu.jcu.v4l4j.impl.jni;

import java.nio.ByteBuffer;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import java.util.Stack;

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
	protected final Stack<AutoCloseable> managedRefs = new Stack<>();
	
	public StructMap(StructPrototype struct) {
		this(struct, MemoryUtils.alloc(struct.getAlignment(), struct.getSize()), struct.getSize(), true);
	}
	
	public StructMap(StructPrototype struct, long pointer, long length) {
		this(struct, pointer, length, false);
	}
	
	protected StructMap(StructPrototype struct, long pointer, long length, boolean deallocOnClose) {
		this(struct, pointer, MemoryUtils.wrap(pointer, length), deallocOnClose);
	}
	
	public StructMap(StructPrototype struct, ByteBuffer buffer) {
		this(struct, MemoryUtils.unwrap(buffer), buffer, false);
	}
	
	protected StructMap(StructPrototype struct, long pointer, ByteBuffer buffer, boolean deallocOnClose) {
		this.struct = struct;
		this.pointer = pointer;
		this.buffer = buffer;
		if (deallocOnClose) {
			this.managedRefs.push(new AutoCloseable() {
				@Override
				public void close() throws Exception {
					MemoryUtils.free(pointer);
				}
			});
		}
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
				//Wrap local field
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
				//Wrap local field
				ByteBuffer dup = MemoryUtils.sliceBuffer(this.buffer, field.getOffset(), field.getSize());
				return new NativeArray((ArrayStructFieldType) type, dup);
			} else if (type instanceof PointerStructFieldType) {
				throw new IllegalStateException("Far pointer is not wrapped/allocated; you may want to call allocateFar/wrapFar");
			} else {
				throw new IllegalArgumentException("Field " + key + " is not an array or pointer(is " + type + ")");
			}
		});
	}
	
	public StructMap getStruct(String name) {
		return this.structs.computeIfAbsent(name, key->{
			StructField field = this.struct.getField(key);
			StructFieldType type = field.getType();
			if (type instanceof StructPrototype) {
				//Wrap local field
				ByteBuffer dup = MemoryUtils.sliceBuffer(this.buffer, field.getOffset(), field.getSize());
				return new StructMap((StructPrototype) type, dup);
			} else if (type instanceof PointerStructFieldType) {
				throw new IllegalStateException("Far pointer is not wrapped/allocated; you may want to call allocateFar/wrapFar");
			} else {
				throw new IllegalArgumentException("Field " + key + " is not a struct or pointer (is " + type + ")");
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
		if (type instanceof PointerStructFieldType) {
			StructFieldType farType = ((PointerStructFieldType) type).getFarType();
			
			final long farPointer = MemoryUtils.alloc(farType.getAlignment(), farType.getSize());
			
			AutoCloseable manager = null;
			
			try {
				final ByteBuffer farBuffer = MemoryUtils.wrap(farPointer, farType.getSize());
				
				if (farType instanceof ArrayStructFieldType) {
					NativeArray array = new NativeArray((ArrayStructFieldType)farType, farBuffer);
					manager = array;
					this.arrays.put(name, array);
				} else if (farType instanceof StructPrototype) {
					StructMap struct = new StructMap((StructPrototype)farType, farBuffer);
					manager = struct;
					this.structs.put(name, struct);
				} else if (farType instanceof UnionPrototype) {
					NativeUnion union = new NativeUnion((UnionPrototype)farType, farBuffer);
					manager = union;
					this.unions.put(name, union);
				} else {
					manager = new AutoCloseable() {
						@Override
						public void close() throws Exception {
							MemoryUtils.free(farPointer);
							managedRefs.remove(this);//Remove self from managedRefs queue
						}
					};
				}
				this.managedRefs.add(manager);
			} catch (Exception e) {
				if (manager != null) {
					try {
						manager.close();
					} catch (Exception e1) {
						e.addSuppressed(e);
					}
				} else {
					MemoryUtils.free(farPointer);
				}
				throw e;
			}
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
			long farPointer = MemoryUtils.align(farType.getAlignment(), farType.getSize());
			ByteBuffer farBuffer = MemoryUtils.wrap(farPointer, farType.getSize());
			
			if (farType instanceof ArrayStructFieldType) {
				NativeArray array = new NativeArray((ArrayStructFieldType)farType, farBuffer);
				this.arrays.put(name, array);
			} else if (farType instanceof StructPrototype) {
				StructMap struct = new StructMap((StructPrototype)farType, farBuffer);
				this.structs.put(name, struct);
			} else if (farType instanceof UnionPrototype) {
				NativeUnion union = new NativeUnion((UnionPrototype)farType, farBuffer);
				this.unions.put(name, union);
			} else {
				
			}
		}
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
				if (pointer != union.pointer()) {
					UnionPrototype prototype = union.type();
					ByteBuffer oldBuffer = union.buffer();
					union.close();
					this.unions.put(name, new NativeUnion(prototype, MemoryUtils.wrap(pointer, oldBuffer.capacity())));
				}
			} else if (this.structs.containsKey(name)) {
				StructMap struct = this.structs.get(name);
				if (pointer != struct.pointer()) {
					StructPrototype prototype = struct.type();
					ByteBuffer oldBuffer = struct.buffer();
					struct.close();
					this.structs.put(name, new StructMap(prototype, pointer, oldBuffer.capacity(), false));
				}
			} else if (this.arrays.containsKey(name)) {
				NativeArray array = this.arrays.get(name);
				if (pointer != array.pointer()) {
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
		while (!this.managedRefs.isEmpty())
			this.managedRefs.pop().close();
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
