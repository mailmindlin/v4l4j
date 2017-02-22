package au.edu.jcu.v4l4j.impl.jni;

import java.nio.ByteBuffer;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;
import java.util.ListIterator;

public class NativeArray implements List<Object>, AutoCloseable, NativeWrapper {
	protected final long pointer;
	protected final ByteBuffer buffer;
	protected final ArrayStructFieldType arrayType;
	
	public NativeArray(ArrayStructFieldType arrayType, ByteBuffer buffer) {
		this(arrayType, MemoryUtils.unwrap(buffer), buffer, false);
	}
	
	protected NativeArray(ArrayStructFieldType arrayType, long pointer, long length, boolean releaseOnClose) {
		this(arrayType, pointer, MemoryUtils.wrap(pointer, length), releaseOnClose);
	}
	
	protected NativeArray(ArrayStructFieldType arrayType, long pointer, ByteBuffer buffer, boolean releaseOnClose) {
		this.arrayType = arrayType;
		this.pointer = pointer;
		this.buffer = buffer;
	}
	
	
	
	@Override
	public Object get(int index) {
		return arrayType.readElement(buffer, index);
	}
	
	@Override
	public Object set(int index, Object value) {
		Object oldValue = get(index);
		arrayType.writeElement(buffer, index, value);
		return oldValue;
	}
	
	@Override
	public boolean add(Object value) {
		throw new UnsupportedOperationException();
	}
	
	@Override
	public void add(int arg0, Object value) {
		throw new UnsupportedOperationException();
	}
	
	@Override
	public boolean addAll(Collection<? extends Object> arg0) {
		throw new UnsupportedOperationException();
	}
	
	@Override
	public boolean addAll(int arg0, Collection<? extends Object> arg1) {
		throw new UnsupportedOperationException();
	}
	
	@Override
	public void clear() {
		throw new UnsupportedOperationException();
	}
	
	@Override
	public boolean contains(Object arg0) {
		// TODO Auto-generated method stub
		return false;
	}
	
	@Override
	public boolean containsAll(Collection<?> arg0) {
		// TODO Auto-generated method stub
		return false;
	}
	
	@Override
	public int indexOf(Object arg0) {
		// TODO Auto-generated method stub
		return 0;
	}
	
	@Override
	public boolean isEmpty() {
		return false;
	}
	
	@Override
	public Iterator<Object> iterator() {
		// TODO Auto-generated method stub
		return null;
	}
	
	@Override
	public int lastIndexOf(Object arg0) {
		// TODO Auto-generated method stub
		return 0;
	}
	
	@Override
	public ListIterator<Object> listIterator() {
		// TODO Auto-generated method stub
		return null;
	}
	
	@Override
	public ListIterator<Object> listIterator(int arg0) {
		// TODO Auto-generated method stub
		return null;
	}
	
	@Override
	public boolean remove(Object arg0) {
		throw new UnsupportedOperationException();
	}
	
	@Override
	public Object remove(int arg0) {
		throw new UnsupportedOperationException();
	}
	
	@Override
	public boolean removeAll(Collection<?> arg0) {
		// TODO Auto-generated method stub
		return false;
	}
	
	@Override
	public boolean retainAll(Collection<?> arg0) {
		throw new UnsupportedOperationException();
	}
	
	@Override
	public int size() {
		return this.arrayType.getLength();
	}
	
	@Override
	public List<Object> subList(int arg0, int arg1) {
		// TODO Auto-generated method stub
		return null;
	}
	
	@Override
	public Object[] toArray() {
		return null;
	}
	
	@Override
	public <T> T[] toArray(T[] arg0) {
//		return this.arrayType.read(this.buffer, null).toArray(arg0);
		return arg0;
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
	public long pointer() {
		return this.pointer;
	}
	
	@Override
	public ArrayStructFieldType type() {
		return this.arrayType;
	}
}