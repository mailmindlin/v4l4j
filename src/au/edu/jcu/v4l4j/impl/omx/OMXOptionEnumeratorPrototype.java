package au.edu.jcu.v4l4j.impl.omx;

import java.util.Iterator;
import java.util.NoSuchElementException;
import java.util.function.Function;
import java.util.function.IntFunction;

import au.edu.jcu.v4l4j.impl.jni.NativeStruct;

public class OMXOptionEnumeratorPrototype<T> {
	protected int queryIdx;
	protected final Function<NativeStruct, T> resultMapper;
	protected final IntFunction<NativeStruct> queryGenerator;
	
	public OMXOptionEnumeratorPrototype(int queryIdx, IntFunction<NativeStruct> queryGenerator, Function<NativeStruct, T> resultMapper) {
		this.queryIdx = queryIdx;
		this.queryGenerator = queryGenerator;
		this.resultMapper = resultMapper;	
	}
	
	public Iterator<T> iterate(OMXComponent component) {
		return new OMXOptionEnumerator(component);
	}
	
	protected class OMXOptionEnumerator implements Iterator<T> {
		protected final OMXComponent component;
		T next = null;
		int idx = 0;
		
		public OMXOptionEnumerator(OMXComponent component) {
			this.component = component;
		}
		
		protected void doGetNext() {
			try (NativeStruct query = queryGenerator.apply(this.idx++)) {
				component.getConfig(true, queryIdx, query.buffer());
				this.next = resultMapper.apply(query);
			} catch (Exception e) {
				this.idx = -1;
			}
		}
		
		@Override
		public boolean hasNext() {
			if (idx < 0)
				return false;
			if (next == null)
				doGetNext();
			return idx >= 0;
		}
	
		@Override
		public T next() {
			if (!hasNext())
				throw new NoSuchElementException();
			T result = next;
			next = null;
			return result;
		}
	}
}
