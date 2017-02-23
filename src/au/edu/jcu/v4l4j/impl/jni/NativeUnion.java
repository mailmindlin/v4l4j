package au.edu.jcu.v4l4j.impl.jni;

import java.nio.ByteBuffer;
import java.util.Collection;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;

public class NativeUnion implements AutoCloseable, NativeWrapper, Map<String, Object> {
	protected final UnionPrototype union;
	protected final long pointer;
	protected final ByteBuffer buffer;
	protected final Map<String, NativeWrapper> wrappers = new HashMap<>();
	
	public NativeUnion(UnionPrototype union, ByteBuffer buffer) {
		this(union, MemoryUtils.unwrap(buffer), buffer, false);
	}
	
	protected NativeUnion(UnionPrototype union, long pointer, ByteBuffer buffer, boolean releaseOnClose) {
		this.union = union;
		this.pointer = pointer;
		this.buffer = buffer;
	}
	
	@Override
	public long pointer() {
		return this.pointer;
	}
	
	@Override
	public UnionPrototype type() {
		return this.union;
	}

	@Override
	public void close() throws Exception {
		// TODO Auto-generated method stub
		
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
		// TODO Auto-generated method stub
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
	public Object get(Object key) {
		return this.type().readField(this.buffer(), key.toString());
	}
	
	public StructMap getStruct(String name) {
		return (StructMap) this.wrappers.computeIfAbsent(name, key->{
			StructField field = this.type().getOption(key);
			return new StructMap((StructPrototype) field.getType(), this.pointer(), this.buffer(), false);
		});
	}
	
	public NativeArray getArray(String name) {
		return (NativeArray) this.wrappers.computeIfAbsent(name, key->{
			StructField field = this.type().getOption(key);
			return new NativeArray((ArrayStructFieldType) field.getType(), this.pointer(), this.buffer(), false);
		});
	}
	
	public NativeUnion getUnion(String name) {
		return (NativeUnion) this.wrappers.computeIfAbsent(name, key->{
			StructField field = this.type().getOption(key);
			return new NativeUnion((UnionPrototype) field.getType(), this.pointer(), this.buffer(), false);
		});
	}

	@Override
	public Object put(String key, Object value) {
		this.type().writeField(this.buffer(), key, value);
		return get(key);
	}

	@Override
	public Object remove(Object key) {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public void putAll(Map<? extends String, ? extends Object> m) {
		for (Entry<? extends String, ? extends Object> e : m.entrySet())
			this.put(e.getKey(), e.getValue());
	}

	@Override
	public void clear() {
		// TODO Auto-generated method stub
		
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
	public Set<java.util.Map.Entry<String, Object>> entrySet() {
		// TODO Auto-generated method stub
		return null;
	}
}
