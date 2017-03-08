package au.edu.jcu.v4l4j.impl.jni;

import java.nio.ByteBuffer;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

public abstract class NativeWrapper<K, T> extends NativePointer<T> {
	protected final Map<K, NativePointer<?>> localWrappers = new HashMap<>();
	protected final Map<K, NativePointer<?>> remoteWrappers = new HashMap<>();
	
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
	
	@SuppressWarnings("unchecked")
	protected <U, V extends NativePointer<U>> V doWrapPointer(StructFieldType type, long childAddress, ByteBuffer childBuffer, boolean freeOnClose) {
//		System.out.println("Wrapping pointer of type " + type);
		if (type instanceof StructPrototype)
			return (V) new NativeStruct((StructPrototype) type, childAddress, childBuffer, freeOnClose);
		else if (type instanceof UnionPrototype)
			return (V) new NativeUnion<U>((UnionPrototype) type, childAddress, childBuffer, freeOnClose);
		else if (type instanceof ArrayStructFieldType)
			return (V) new NativeArray((ArrayStructFieldType) type, childAddress, childBuffer, freeOnClose);
		else
			return (V) new NativePointer<U>(type, childAddress, childBuffer, false);
	}
	
	protected <U, V extends NativePointer<U>> V doWrapLocalChild(StructFieldType type, int offset, int size) {
		long childAddress = this.address() + offset;
		ByteBuffer childBuffer = MemoryUtils.sliceBuffer(this.buffer(), offset, size);
		return doWrapPointer(type, childAddress, childBuffer, false);
	}
	
	protected <U, V extends NativePointer<U>> V doAllocChild(StructFieldType type, int alignment, int size) {
		long childAddress = 0;
		V result;
		try {
			childAddress = MemoryUtils.alloc(alignment, size);
			ByteBuffer childBuffer = MemoryUtils.wrap(childAddress, size);
			result = doWrapPointer(type, childAddress, childBuffer, true);
			if (!this.managedRefs.add(result))
				throw new RuntimeException("Could not add managed ref to list");
		} catch (Throwable t) {
			//Yes, I know that this is pretty unsafe, but we have to make sure that
			//if we failed to add to managedRefs, we at least free the memory
			try {
				if (childAddress != 0)
					MemoryUtils.free(childAddress);
			} catch (Exception e) {
				t.addSuppressed(e);
			}
			//Rethrow t
			throw t;
		}
		return result;
	}
	
	public abstract <U, V extends NativePointer<U>> V getChild(K key);
	
	@SuppressWarnings("unchecked")
	public <U, V extends NativePointer<U>> V getChildRemote(K key) {
		return (V) this.remoteWrappers.get(key);
	}
	
	public <U> NativeUnion<U> getUnion(K key) {
		NativeUnion<U> result = getChildRemote(key);
		if (result != null)
			return result;
		return getChild(key);
	}
	
	public NativeStruct getStruct(K key) {
		NativeStruct result = getChildRemote(key);
		if (result != null)
			return result;
		return getChild(key);
	}
	
	public NativeArray getArray(K key) {
		NativeArray result = getChildRemote(key);
		if (result != null)
			return result;
		return getChild(key);
	}
	
	public abstract void wrapChildRemote(K key);
	
	public abstract void allocChildRemote(K key);
	
}
