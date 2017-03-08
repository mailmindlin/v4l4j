package au.edu.jcu.v4l4j.api;

import java.util.function.Function;

@FunctionalInterface
public interface ObjIntFunction<T, R> {
	R apply(int i, T t);
	
	default <V> ObjIntFunction<T, V> andThen(Function<? super R, ? extends V> after) {
		return (i,t)->after.apply(this.apply(i, t));
	}
}
