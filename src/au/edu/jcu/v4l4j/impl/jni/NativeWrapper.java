package au.edu.jcu.v4l4j.impl.jni;

import java.nio.ByteBuffer;

public abstract class NativeWrapper<K, T> extends NativePointer<T> {
	protected final Map<K, Pointer<?>> localWrappers = new HashMap<>();
	protected final Map<K, Pointer<?>> remoteWrappers = new HashMap<>();
	
	/**
	 * Set of names that have been wrapped by Java, and could
	 * be broken by an external write
	 */
	protected final Set<K> wrappedNames = new HashSet<>();
	
	protected static String appendPath(String base, String separator, String addon) {
		if (base == null)
			return addon;
		if (addon == null)
			return base;
		if (base.isEmpty() || addon.isEmpty())
			return base + addon;
		return base + separator + addon;
	}
	
	protected NativeWrapper(StructFieldType type, long address, ByteBuffer buffer, boolean freeOnClose) {
		super(type, address, buffer, freeOnClose);
	}
	
	private <U, V extends NativePointer<U>> V doWrapPointer(StructFieldType type, long addr, ByteBuffer buffer, boolean freeOnClose) {
		if (type instanceof StructPrototype)
			return new NativeStruct((StructPrototype) type, childAddress, childBuffer, freeOnClose);
		else if (type instanceof UnionPrototype)
			return new NativeUnion((UnionPrototype) type, childAddress, childBuffer, freeOnClose);
		else if (type instanceof ArrayStructFieldType)
			return new NativeArray((ArrayStructFieldType) type, childAddress, childBuffer, freeOnClose);
		else
			return new NativePointer(type, childAddress, childBuffer, false);
	}
	
	protected <U, V extends NativePointer<U>> V doWrapChild(StructFieldType type, int offset, int size) {
		long childAddress = this.address() + offset;
		ByteBuffer childBuffer = MemoryUtils.sliceBuffer(this.buffer(), offfset, size);
		return doWrapPointer(type, childAddress, childBuffer, false);
	}
	
	protected <U, V extends NativePointer<U>> V doAllocChild(StructFieldType type, int alignment, int size) {
		long childAddress = 0;
		V result;
		try {
			childAddress = MemoryUtils.alloc(alignment, size);
			ByteBuffer childBuffer = MemoryUtils.wrap(childAddress, size);
			result = (V) doWrapPointer(type, childAddress, childBuffer, true);
			if (!this.managedRefs.add(v))
				//TODO throw something
		} catch (Throwable t) {
			//Yes, I know that this is pretty unsafe, but we have to make sure that
			//if we failed to add to managedRefs, we at least free the memory
			if (childAddress != 0)
				MemoryUtils.free(childAddress);
			throw t;
		}
		return result;
	}
	
	public abstract <U, V extends NativePointer<U>> V getChild(K key);
	
	public abstract <U, V extends NativePointer<U>> V getChildRemote(K key);
	
	public NativeUnion getUnion(K key) {
		return (NativeUnion) getChild(key);
	}
	
	public NativeArray getArray(K key) {
		return (NativeArray) getChild(key);
	}
	
	public NativeStruct getStruct(K key) {
		return (NativeStruct) getChild(key);
	}
}
