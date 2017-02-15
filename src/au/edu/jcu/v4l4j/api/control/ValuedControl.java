package au.edu.jcu.v4l4j.api.control;

import java.time.Duration;
import java.util.concurrent.Callable;
import java.util.function.Consumer;
import java.util.function.Function;
import java.util.function.Supplier;

public interface ValuedControl<T> extends Control {
	
	ValuedControlGetter<T, T> get();
	
	ValuedControlAccessor<T, T> access();
	
	public static interface ValuedControlAccessor<T, R> extends Callable<R> {
		ValuedControlAccessor<T, R> setTimeout(Duration timeout);
		
		ValuedControlGetter<T, T> get();
	}
	
	public static interface ValuedControlGetter<T, R> extends ValuedControlAccessor<T, R> {
		ValuedControlGetter<T, R> read(Consumer<T> handler);
		
		ValuedControlUpdater<T, R> write(T value);
		
		ValuedControlUpdater<T, R> write(Supplier<T> supplier);
		
		ValuedControlUpdater<T, R> update(Function<T, T> mappingFunction);
		
		ValuedControlUpdater<T, R> increase();
		
		ValuedControlUpdater<T, R> decrease();
	}
	
	public static interface ValuedControlUpdater<T, R> extends ValuedControlGetter<T, R> {
		ValuedControlAccessor<T, R> set();
		
		ValuedControlGetter<T, R> setAndGet();
	}
}
