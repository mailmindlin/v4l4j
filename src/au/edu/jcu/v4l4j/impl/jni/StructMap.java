package au.edu.jcu.v4l4j.impl.jni;

import java.lang.ref.WeakReference;
import java.nio.ByteBuffer;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

public class StructMap implements Map<String, Object>, AutoCloseable {
	protected final StructPrototype struct;
	protected final long pointer;
	protected final ByteBuffer buffer;
	/**
	 * Map for structs that are far (accessible through a pointer), but we didn't allocate
	 */
	protected final Map<String, WeakReference<StructMap>> farStructRefs = new HashMap<>();
	/**
	 * Lookup for far structs (accessible through a pointer) that we allocated and linked
	 */
	protected final Map<String, StructMap> farStructs = new HashMap<>();
	protected final Map<String, WeakReference<NativeArray>> farArrayRefs = new HashMap<>();
	protected final Map<String, NativeArray> farArrays = new HashMap<>();
	/**
	 * Map for 
	 */
	protected final Map<String, Long> farPointers = new HashMap<>();
	
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
		String key = _key.toString();
		int dotIdx = key.indexOf(".", 1);
		int bracketIdx = key.indexOf("[", 1);
		int derefIdx = key.indexOf("->", 1);
		
		int baseEndIdx = key.length();
		//Find nonnegative minimum value
		if (dotIdx > -1)
			baseEndIdx = dotIdx;
		if (bracketIdx > -1 && bracketIdx < baseEndIdx)
			baseEndIdx = bracketIdx;
		if (derefIdx > -1 && derefIdx < baseEndIdx)
			baseEndIdx = derefIdx;
		
		String baseKey = key.substring(0, baseEndIdx);
		
		if (baseKey.startsWith("->")) {
			//Dereferenced access 
			baseKey = baseKey.substring(2);
			if (this.farStructRefs.containsKey(baseKey)) {
				StructMap far = this.farStructRefs.compute(baseKey, (_k, oldRef) -> {
					//Re-create far reference object if deleted
					StructMap old = oldRef == null ? null : oldRef.get();
					if (old != null)
						return oldRef;
					//We have to get a new wrapper
					StructField localField = this.struct.getField(baseKey);
					//TODO finish
					StructMap newMap = null;
					return null;
				}).get();
				far.get(key.substring(baseEndIdx));
			}
		}
		
		if (this.farStructRefs.containsKey(baseKey) || this.farStructs.containsKey(baseKey)) {
			StructMap far = 
		}
		return null;
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
	
	public ByteBuffer getBuffer() {
		return this.buffer;
	}
}
