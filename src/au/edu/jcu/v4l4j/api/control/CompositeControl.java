package au.edu.jcu.v4l4j.api.control;

import java.time.Duration;
import java.util.Map;
import java.util.Set;
import java.util.function.BiFunction;
import java.util.function.Consumer;
import java.util.function.Function;
import java.util.function.Supplier;

public interface CompositeControl extends Control<Map<String, Object>> {
	Set<Control<?>> getChildren();
	
	Control<?> getChildByName(String name);
	
	@Override
	CompositeControlGetter<Map<String, Object>, Map<String, Object>> get();
	
	@Override
	CompositeControlAccessor<Map<String, Object>, Void> access();
	
	@Override
	default ControlType getType() {
		return ControlType.COMPOSITE;
	}
	
	public static interface CompositeControlAccessor<T, R> extends ControlAccessor<T, R> {
		@Override
		CompositeControlAccessor<T, R> setTimeout(Duration timeout);
		
		@Override
		CompositeControlGetter<T, T> get();
		
		@Override
		R call() throws Exception;
	}
	
	public static interface CompositeControlGetter<T, R> extends ControlGetter<T, R>, CompositeControlAccessor<T, R> {
		
		@Override
		CompositeControlGetter<T, R> setTimeout(Duration timeout);
		
		@Override
		CompositeControlGetter<T, R> read(Consumer<T> handler);
		
		<E> CompositeControlGetter<T, R> read(String name, Consumer<E> handler);
		
		@Override
		CompositeControlUpdater<T, R> write(T value);
		
		@Override
		CompositeControlUpdater<T, R> write(Supplier<T> supplier);
		
		<E> CompositeControlUpdater<T, R> write(String name, Supplier<E> supplier);
		
		<E> CompositeControlUpdater<T, R> write(String name, E value);
		
		@Override
		CompositeControlUpdater<T, R> update(Function<T, T> mappingFunction);
		
		<E> CompositeControlUpdater<T, R> update(String name, BiFunction<String, E, E> mappingFunction);
		
		@Override
		default CompositeControlUpdater<T, R> increase() {
			throw new UnsupportedOperationException();
		}
		
		@Override
		default CompositeControlUpdater<T, R> decrease() {
			throw new UnsupportedOperationException();
		}
		
	}
	
	public static interface CompositeControlUpdater<T, R> extends ControlUpdater<T, R>, CompositeControlGetter<T, R> {
		
		@Override
		CompositeControlUpdater<T, R> setTimeout(Duration timeout);
		
		@Override
		CompositeControlAccessor<T, R> set();
		
		@Override
		CompositeControlGetter<T, R> setAndGet();
	}
}
