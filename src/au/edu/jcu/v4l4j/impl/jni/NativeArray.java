package au.edu.jcu.v4l4j.impl.jni;

import java.nio.ByteBuffer;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;
import java.util.ListIterator;

public class NativeArray<E> extends NativeWrapper<Integer, List<E>> implements List<E>, AutoCloseable {
	
	public NativeArray(ArrayStructFieldType<E> arrayType, ByteBuffer buffer) {
		this(arrayType, MemoryUtils.unwrap(buffer), buffer, false);
	}
	
	protected NativeArray(ArrayStructFieldType<E> arrayType, long address, long length, boolean releaseOnClose) {
		this(arrayType, address, MemoryUtils.wrap(address, length), releaseOnClose);
	}
	
	protected NativeArray(ArrayStructFieldType<E> arrayType, long address, ByteBuffer buffer, boolean releaseOnClose) {
		super(arrayType, address, buffer, releaseOnClose);
	}
	
	@Override
	public E get(int index) {
		return this.type().readElement(buffer, index);
	}
	
	@Override
	public E set(int index, Object value) {
		E oldValue = get(index);
		this.type().writeElementUnchecked(buffer, index, value);
		return oldValue;
	}

	@Override
	@SuppressWarnings("unchecked")
	public <U, V extends NativePointer<U>> V getChild(Integer key) {
		return (V) this.localWrappers.computeIfAbsent(key, k -> {
			StructFieldType<E> base = type().getBaseType();
			return doWrapLocalChild(base, base.getSize() * key, base.getSize());
		});
	}
	
	@Override
	public void wrapChildRemote(Integer key) {
		if (key > this.size() || key < 0)
			throw new IndexOutOfBoundsException("Illegal index " + key);
		

		StructFieldType<E> type = type().getBaseType();
		
		//Register this name for updates
		this.wrappedNames.add(key);
		final StructFieldType<?> farType = ((PointerStructFieldType) type).getFarType();

		//Read pointer
		final long farPointer = ((Number)this.get(key)).longValue();
		
		this.remoteWrappers.compute(key, (idx, oldValue) -> {
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
	public void allocChildRemote(Integer _key) {
		if (_key > this.size() || _key < 0)
			throw new IndexOutOfBoundsException("Illegal index " + _key);
		
		StructFieldType<E> type = this.type().getBaseType();
		
		if (type instanceof PointerStructFieldType) {
			final StructFieldType<?> farType = ((PointerStructFieldType) type).getFarType();
			
			this.remoteWrappers.compute(_key, (key, oldVal) -> {
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
				this.set(key, newVal.address());
				
				return newVal;
			});
		}
	}
	
	@Override
	public ByteBuffer buffer() {
		return this.buffer;
	}
	
	@Override
	public ArrayStructFieldType<E> type() {
		return (ArrayStructFieldType<E>) this.type;
	}
	
	@Override
	public boolean add(Object value) {
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
	public Iterator<E> iterator() {
		// TODO Auto-generated method stub
		return null;
	}
	
	@Override
	public int lastIndexOf(Object arg0) {
		// TODO Auto-generated method stub
		return 0;
	}
	
	@Override
	public ListIterator<E> listIterator() {
		// TODO Auto-generated method stub
		return null;
	}
	
	@Override
	public ListIterator<E> listIterator(int arg0) {
		// TODO Auto-generated method stub
		return null;
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
		return this.type().getLength();
	}
	
	@Override
	public List<E> subList(int arg0, int arg1) {
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
	public void add(int arg0, Object value) {
		throw new UnsupportedOperationException();
	}
	
	@Override
	public boolean addAll(Collection<? extends E> arg0) {
		throw new UnsupportedOperationException();
	}
	
	@Override
	public boolean addAll(int arg0, Collection<? extends E> arg1) {
		throw new UnsupportedOperationException();
	}
	
	@Override
	public boolean remove(Object arg0) {
		throw new UnsupportedOperationException();
	}
	
	@Override
	public E remove(int arg0) {
		throw new UnsupportedOperationException();
	}
}