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
	
	public static interface CompositeControlAccessor<P, T, R> extends ControlAccessor<P, T, R> {
		@Override
		CompositeControlAccessor<P, T, R> setTimeout(Duration timeout);
		
		@Override
		CompositeControlAccessor<P, T, T> get();
		
		@Override
		CompositeControlAccessor<P, T, R> read(Consumer<T> handler);
		
		@Override
		CompositeControlAccessor<P, T, R> write(T value);
		
		@Override
		CompositeControlAccessor<P, T, R> write(Supplier<T> supplier);
		
		@Override
		CompositeControlAccessor<P, T, R> update(Function<T, T> mappingFunction);
		
		@Override
		CompositeControlAccessor<P, T, R> increase();
		
		@Override
		CompositeControlAccessor<P, T, R> decrease();
		
		@Override
		CompositeControlAccessor<P, T, R> set();
		
		@Override
		CompositeControlAccessor<P, T, R> setAndGet();
		
		<Ct, C extends ControlAccessor<CompositeControlAccessor<P, T, R>, Ct, R>> C withChild(String name);
		
		@Override
		R call() throws Exception;
	}
}
