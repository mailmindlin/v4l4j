package au.edu.jcu.v4l4j.impl.jni;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;

/**
 * POJO that wraps the idea of a pointer.
 * @author mailmindlin
 *
 * @param <T> Pointed-to type
 */
public class NativePointer<T> implements AutoCloseable {
	protected final long address;
	protected final StructFieldType<T> type;
	protected ByteBuffer buffer;
	
	/**
	 * Set of objects that we have to release when closing
	 */
	protected final List<AutoCloseable> managedRefs = new ArrayList<>();
	
	protected NativePointer(StructFieldType<T> type, long address, ByteBuffer buffer, boolean freeOnClose) {
		this.type = type;
		this.address = address;
		this.buffer = buffer;
		if (freeOnClose)
			managedRefs.add(this);
	}
	
	/**
	 * Read value from pointer (equivalent to <code>*p</code> in C).
	 * @return value
	 */
	public T get() {
		return (T) this.type().read(this.buffer(), null);
	}
	
	/**
	 * Write value to pointer (equivalent to <code>*p = value</code> in C).
	 * @param value to set
	 */
	public void set(T value) {
		//TODO use writeUnchecked?
		this.type().write(this.buffer(), value);
	}
	
	/**
	 * Sets all bytes in memory to 0x00. Think memset.
	 */
	public void clear() {
		MemoryUtils.memset(this.address(), this.buffer().remaining(), 0x00);
	}
	
	/**
	 * Get pointer pointed to by this. Kinda like {@link #get()}, but wraps it in a pointer for you.
	 * @return far pointer
	 */
	public <U> NativePointer<U> dereference() {
		StructFieldType<T> type = this.type();
		if (!(type instanceof PointerStructFieldType))
			throw new UnsupportedOperationException("Cannot dereference non-pointer");
		
		long farAddr = ((Number)type.read(buffer, null)).longValue();
		if (farAddr == 0)
			return null;
		
		@SuppressWarnings("unchecked")
		StructFieldType<U> farType = ((PointerStructFieldType<U>)type).getFarType();
		
		return new NativePointer<U>(farType, farAddr, MemoryUtils.wrap(farAddr, farType.getSize()), false);
	}
	
	/**
	 * Get wrapped type
	 * @return type
	 */
	public StructFieldType<T> type() {
		return type;
	}
	
	/**
	 * Get native address wrapped by this
	 * @return address
	 */
	public long address() {
		return this.address;
	}
	
	/**
	 * Get buffer view of memory represented by this pointer.
	 * @return buffer view
	 */
	public ByteBuffer buffer() {
		return this.buffer;
	}
	
	@Override
	public void close() {
		for (AutoCloseable ref : this.managedRefs) {
			try {
				if (ref == this)
					MemoryUtils.free(this.address());
				else
					ref.close();
			} catch (Exception e) {
				//TODO: shouldn't we do something with this exception?
				e.printStackTrace();
			}
		}
		this.managedRefs.clear();
	}

	@Override
	protected void finalize() throws Throwable {
		//Yell at the user for not closing their pointers
		super.finalize();
		if (this.managedRefs != null && !this.managedRefs.isEmpty())
			for (int i = 0; i < 50; i++)
				System.err.printf("Error: did not close pointer 0x%08x\n", this.address);
	}
}