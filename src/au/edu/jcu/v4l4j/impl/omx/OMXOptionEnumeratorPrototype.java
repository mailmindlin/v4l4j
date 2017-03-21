package au.edu.jcu.v4l4j.impl.omx;

import java.util.NoSuchElementException;
import java.util.function.Function;

import au.edu.jcu.v4l4j.api.CloseableIterator;
import au.edu.jcu.v4l4j.api.ObjIntFunction;
import au.edu.jcu.v4l4j.impl.jni.NativeStruct;

public class OMXOptionEnumeratorPrototype<T> {
	protected int queryIdx;
	protected final Function<NativeStruct, T> resultMapper;
	protected final ObjIntFunction<NativeStruct, NativeStruct> queryGenerator;
	
	public OMXOptionEnumeratorPrototype(int queryIdx, ObjIntFunction<NativeStruct, NativeStruct> queryGenerator, Function<NativeStruct, T> resultMapper) {
		this.queryIdx = queryIdx;
		this.queryGenerator = queryGenerator;
		this.resultMapper = resultMapper;	
	}
	
	public CloseableIterator<T> iterate(OMXComponent component, int port) {
		return new OMXOptionEnumerator(component, port);
	}
	
	protected class OMXOptionEnumerator implements CloseableIterator<T> {
		protected final OMXComponent component;
		protected final int port;
		protected NativeStruct query;
		T next = null;
		int idx = 0;
		
		public OMXOptionEnumerator(OMXComponent component, int port) {
			this.component = component;
			this.port = port;
		}
		
		protected void doGetNext() {
			try {
				this.query = queryGenerator.apply(this.idx++, query);
				if (port >= 0)
					query.put("nPortIndex", port);
				component.getConfig(true, queryIdx, query.buffer());
				this.next = resultMapper.apply(query);
			} catch (Exception e) {
				try {
					this.close();
				} catch (Exception e1) {
					e1.printStackTrace();
				}
				//Store exception to use as a cause of a NoSuchElementException
				//this.next = e;
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

		@Override
		public void close() throws Exception {
			this.idx = -1;
			this.next = null;
			if (this.query != null)
				this.query.close();
			this.query = null;
		}
	}
}
