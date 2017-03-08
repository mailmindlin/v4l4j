package au.edu.jcu.v4l4j.api;

import java.util.Iterator;
import java.util.function.Consumer;

public interface CloseableIterator<T> extends Iterator<T>, AutoCloseable {
	public static <T> CloseableIterator<T> wrap(Iterator<T> iterator) {
		return new CloseableIterator<T>() {
			@Override
			public boolean hasNext() {
				return iterator.hasNext();
			}

			@Override
			public T next() {
				return iterator.next();
			}

			@Override
			public void remove() {
				iterator.remove();
			}

			@Override
			public void forEachRemaining(Consumer<? super T> action) {
				iterator.forEachRemaining(action);
			}
			
			@Override
			public void close() throws Exception {
			}
			
		};
	}
	@Override
	default void close() throws Exception {
	}
}
