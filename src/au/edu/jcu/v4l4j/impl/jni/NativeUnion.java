package au.edu.jcu.v4l4j.impl.jni;

import java.nio.ByteBuffer;
import java.util.Collection;
import java.util.HashMap;
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
		return false;
	}

	@Override
	public boolean containsKey(Object key) {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean containsValue(Object value) {
		throw new UnsupportedOperationException();
	}
	
	@Override
	public <U, V extends NativePointer<U>> V getChild(String name) {
		return (V) this.wrappers.computeIfAbsent(name, key->{
			StructField option = type().getOption(key);
			if (option == null)
				return null;
			return doWrapChild(option.getType(), option.getOffset(), option.getSize());
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
		// TODO Auto-generated method stub
		
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
}
