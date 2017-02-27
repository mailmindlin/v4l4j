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
	CompositeControlAccessor<Void, Map<String, Object>, Map<String, Object>> get();
	
	@Override
	CompositeControlAccessor<Void, Map<String, Object>, Void> access();
	
	@Override
	default ControlType getType() {
		return ControlType.COMPOSITE;
	}
	
	public static interface CompositeControlAccessor<P, T, R> extends ControlAccessor<P, T, R> {
		@Override
		CompositeControlAccessor<P, T, R> setTimeout(Duration timeout);
		
		@Override
		CompositeControlAccessor<P, T, T> get();
		
		@Override
		CompositeControlAccessor<P, T, R> read(Consumer<T> handler);
		
		<E> CompositeControlAccessor<P, T, R> read(String name, Consumer<E> handler);
		
		@Override
		default CompositeControlAccessor<P, T, R> write(T value) {
			return write(()->value);
		}
		
		@Override
		CompositeControlAccessor<P, T, R> write(Supplier<T> supplier);
		
		default <E> CompositeControlAccessor<P, T, R> write(String name, E value) {
			return write(name, ()->value);
		}
		
		<E> CompositeControlAccessor<P, T, R> write(String name, Supplier<E> supplier);
		
		@Override
		CompositeControlAccessor<P, T, R> update(Function<T, T> mappingFunction);
		
		<E> CompositeControlAccessor<P, T, R> update(String name, BiFunction<String, E, E> mappingFunction);
		
		@Override
		CompositeControlAccessor<P, T, R> increase();
		
		@Override
		CompositeControlAccessor<P, T, R> decrease();
		
		@Override
		CompositeControlAccessor<P, T, R> set();
		
		@Override
		default CompositeControlAccessor<P, T, T> setAndGet() {
			return set().get();
		}
		
		@Override
		R call() throws Exception;
	}
}
