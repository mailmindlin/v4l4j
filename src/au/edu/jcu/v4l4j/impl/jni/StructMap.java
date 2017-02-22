package au.edu.jcu.v4l4j.impl.jni;

import java.nio.ByteBuffer;
import java.util.Collection;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;

/**
 * Class to wrap and manipulate actual memory with the given struct prototype.
 */
public class StructMap implements Map<String, Object>, AutoCloseable {
	protected final StructPrototype struct;
	protected final long pointer;
	protected final ByteBuffer buffer;
	/**
	 * Arrays reachable by pointer
	 */
	protected final Map<String, NativeArray> farArrays = new HashMap<>();
	/**
	 * Structs reachable by pointer
	 */
	protected final Map<String, StructMap> farStructs = new HashMap<>();
	protected final Map<String, Long> farPointers = new HashMap<>();
	protected final Set<Long> unmanagedRefs = new HashSet<>();
	
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
		int dotIdx = key.indexOf(".");
		int bracketIdx = key.indexOf("[");
		
		if (dotIdx == -1 && bracketIdx == -1) {
			//Flat value
			return this.struct.readField(buffer, key);
		} else if (dotIdx > -1 || bracketIdx > -1) {
			//Has some complexity to it
			String key0 = key.substring(0, Math.min(dotIdx, bracketIdx));
			
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
		// TODO Auto-generated method stub
		return null;
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
