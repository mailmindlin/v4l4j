package au.edu.jcu.v4l4j.api.control;

import java.time.Duration;
import java.util.concurrent.Callable;
import java.util.function.Consumer;
import java.util.function.Function;
import java.util.function.Supplier;

public interface Control<T> extends AutoCloseable {
	/**
	 * Get the type of this control.
	 * The returned value for this method should be constant across calls.
	 * @return Type of this control
	 */
	ControlType getType();
	
	/**
	 * Get the name of this control.
	 * The returned value for this method should be constant across calls.
	 * @return this control's name
	 */
	String getName();
	
	void close();
	
	default ControlGetter<T, T> get() {
		return access().get();
	}
	
	ControlAccessor<T, Void> access();
	
	public static interface ControlAccessor<T, R> extends Callable<R> {
		ControlAccessor<T, R> setTimeout(Duration timeout);
		
		ControlGetter<T, T> get();
		
		@Override
		R call() throws Exception;
	}
	
	public static interface ControlGetter<T, R> extends ControlAccessor<T, R> {
		
		@Override
		ControlGetter<T, R> setTimeout(Duration timeout);
		
		ControlGetter<T, R> read(Consumer<T> handler);
		
		ControlUpdater<T, R> write(T value);
		
		ControlUpdater<T, R> write(Supplier<T> supplier);
		
		ControlUpdater<T, R> update(Function<T, T> mappingFunction);
		
		ControlUpdater<T, R> increase();
		
		ControlUpdater<T, R> decrease();
	}
	
	public static interface ControlUpdater<T, R> extends ControlGetter<T, R> {
		
		@Override
		ControlUpdater<T, R> setTimeout(Duration timeout);
		
		ControlAccessor<T, R> set();
		
		ControlGetter<T, R> setAndGet();
	}
}
