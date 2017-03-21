package au.edu.jcu.v4l4j.impl.jni;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;

public class NativePointer<T> implements AutoCloseable {
	protected final long address;
	protected final StructFieldType type;
	protected ByteBuffer buffer;
	
	/**
	 * Set of objects that we have to release when closing
	 */
	protected final List<AutoCloseable> managedRefs = new ArrayList<>();
	
	protected NativePointer(StructFieldType type, long address, ByteBuffer buffer, boolean freeOnClose) {
		this.type = type;
		this.address = address;
		this.buffer = buffer;
		if (freeOnClose)
			managedRefs.add(this);
	}
	
	@SuppressWarnings("unchecked")
	public T get() {
		return (T) this.type().read(this.buffer(), null);
	}
	
	public void set(T value) {
		this.type().write(this.buffer(), value);
	}
	
	public void clear() {
		MemoryUtils.memset(this.address(), this.buffer().remaining(), 0x00);
	}
	
	public <U> NativePointer<U> dereference() {
		StructFieldType type = this.type();
		if (!(type instanceof PointerStructFieldType))
			throw new UnsupportedOperationException("Cannot dereference non-pointer");
		long farAddr = ((Number)type.read(buffer, null)).longValue();
		if (farAddr == 0)
			return null;
		StructFieldType farType = ((PointerStructFieldType)type).getFarType();
		return new NativePointer<U>(farType, farAddr, MemoryUtils.wrap(farAddr, farType.getSize()), false);
	}
	
	public StructFieldType type() {
		return type;
	}
	
	public long address() {
		return this.address;
	}
	
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
				e.printStackTrace();
			}
		}
		this.managedRefs.clear();
	}

	@Override
	protected void finalize() throws Throwable {
		super.finalize();
		if (this.managedRefs != null && !this.managedRefs.isEmpty())
			for (int i = 0; i < 50; i++)
				System.err.printf("Error: did not close pointer 0x%08x\n", this.address);
	}
}