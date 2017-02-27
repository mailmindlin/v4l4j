package au.edu.jcu.v4l4j.api.control;

import java.time.Duration;
import java.util.Set;
import java.util.concurrent.Callable;
import java.util.function.BiFunction;
import java.util.function.Consumer;
import java.util.function.Function;
import java.util.function.Supplier;

import au.edu.jcu.v4l4j.api.control.CompositeControl.CompositeControlAccessor;
import au.edu.jcu.v4l4j.api.control.ContinuousControl.ContinuousRange;
import au.edu.jcu.v4l4j.api.control.Control.ControlAccessor;
import au.edu.jcu.v4l4j.api.control.DiscreteControl.DiscreteOption;

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
	
	default ControlAccessor<Void, T, T> get() {
		return access().get();
	}
	
	ControlAccessor<Void, T, Void> access();
	
	public static interface ControlAccessor<P, T, R> extends Callable<R> {
		
		/**
		 * Get the control that this is accessing
		 * @return accessed control
		 */
		Control<T> getControl();
		
		/**
		 * Set the timeout
		 * @param timeout
		 * @return self
		 */
		ControlAccessor<P, T, R> setTimeout(Duration timeout);
		
		/**
		 * Read the current in-memory value into the result value.
		 * The actual read action will occur when the access operation happens.
		 * Note that this reads the latest accessed value, not necessarily the
		 * current state of the control.
		 * @return self
		 */
		ControlAccessor<P, T, T> get();
		
		/**
		 * Read the current in-memory value into the handler.
		 */
		ControlAccessor<P, T, R> get(Consumer<T> handler);
		
		/**
		 * Merge the current in-memory value with the current result value
		 */
		<E> ControlAccessor<P, T, E> get(BiFunction<T, R, E> merger);
		
		/**
		 * Set the value. Note that this does NOT update the control
		 * until set() is called.
		 * @return self
		 */
		default ControlAccessor<P, T, R> set(T value) {
			return write(()->value);
		}
		
		/**
		 * Set the value. Note that this  does NOT update the control
		 * until set() is called.
		 * @return self
		 */
		ControlAccessor<P, T, R> set(Supplier<T> supplier);
		
		/**
		 * Atomic value update.
		 * @return self
		 */
		ControlAccessor<P, T, R> update(Function<T, T> mappingFunction);
		
		/**
		 * Atomic value update, with access to the control backing this accessor.
		 */
		ControlAccessor<P, T, R> update(BiFunction<? extends Control<T>, T, T> mappingFunction);
		
		/**
		 * Attempt to increase value. If this is not possible to implement,
		 * this method should not have any other side-effects.
		 * @see #decrease()
		 * @return self
		 */
		ControlAccessor<P, T, R> increase();
		
		/**
		 * Attempt to decrease value. If this is not possible to implement,
		 * this method should not have any other side-effects.
		 * @see #increase()
		 * @return self
		 */
		ControlAccessor<P, T, R> decrease();
		
		
		/**
		 * Perform a 'read' operation, loading the value of this accessor
		 * from the component.
		 * The return of {@link #call()} will be set to the return of this method.
		 * @return self
		 */
		ControlAccessor<P, T, R> read();
		
		/**
		 * Actually writes the value of the control
		 * @return self
		 */
		ControlAccessor<P, T, R> write();
		
		/**
		 * Equivalent to {@code accessor.set().get()}.
		 * On some implementations, this may be a faster operation.
		 * @return self
		 */
		default ControlAccessor<P, T, R> writeAndRead() {
			return write().read();
		}
		
		default <Ct, C extends ControlAccessor<CompositeControlAccessor<P, T, R>, Ct, R>> C withChild(String name) {
			throw new UnsupportedOperationException("Control has no children");
		}
		
		<C extends ControlAccessor<ControlAccessor<P, T, R>, T, R> C thenIf(Predicate<ControlAccessor<P, T, R>> condition);
		
		/**
		 * Return to the parent control, if available.
		 * @return parent
		 * @throws IllegalStateException if this is the highest control level
		 */
		P and() throws IllegalStateException;
		
		/**
		 * Execute action, and return result
		 */
		@Override
		R call() throws Exception;
	}
}
