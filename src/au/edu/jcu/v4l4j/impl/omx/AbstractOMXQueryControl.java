package au.edu.jcu.v4l4j.impl.omx;

import java.time.Duration;
import java.util.Stack;
import java.util.function.BiFunction;
import java.util.function.Consumer;
import java.util.function.Function;
import java.util.function.Supplier;

import au.edu.jcu.v4l4j.api.control.CompositeControl.CompositeControlAccessor;
import au.edu.jcu.v4l4j.api.control.Control;
import au.edu.jcu.v4l4j.impl.jni.NativePointer;
import au.edu.jcu.v4l4j.impl.jni.NativeStruct;

public abstract class AbstractOMXQueryControl<T> implements Control<T> {
	protected final OMXComponent component;
	protected final AbstractOMXQueryControl<?> parent;
	protected final String name;
	
	protected AbstractOMXQueryControl(OMXComponent componet, AbstractOMXQueryControl<?> parent, String name) {
		this.component = component;
		this.parent = parent;
		this.name = name;
	}
	
	@Override
	public String getName() {
		return this.name;
	}
	
	@Override
	public void close() {
		// TODO Auto-generated method stub
		
	}
	
	@Override
	public AbstractOMXQueryControlAccessor<Void, T, Void> access() {
		return access(parent.access());
	}
	
	protected abstract <P, R> AbstractOMXQueryControlAccessor<P, T, R> access(AbstractOMXQueryControlAccessor<P, ?, R> parentAccessor);
	
	public static abstract class AbstractOMXQueryControlAccessor<P, T, R> implements CompositeControlAccessor<P, T, R> {
		protected final boolean isParentOwner;
		protected final String name;
		protected final AbstractOMXQueryControlAccessor<?, ?, ?> parent;
		protected final Duration timeout;
		protected final Consumer<OMXQueryControlAccessorState> mutator;
		
		protected AbstractOMXQueryControlAccessor(String name, AbstractOMXQueryControlAccessor<?, ?, ?> parent,
				Duration timeout) {
			this(name, parent, timeout, null);
		}
		
		protected AbstractOMXQueryControlAccessor(String name, AbstractOMXQueryControlAccessor<?, ?, ?> parent,
				Duration timeout, Consumer<OMXQueryControlAccessorState> mutator) {
			this(false, name, parent, timeout, mutator);
		}
		
		protected AbstractOMXQueryControlAccessor(boolean isParentOwner, String name,
				AbstractOMXQueryControlAccessor<?, ?, ?> parent, Duration timeout,
				Consumer<OMXQueryControlAccessorState> mutator) {
			this.isParentOwner = isParentOwner;
			this.name = name;
			this.parent = parent;
			this.timeout = timeout;
			this.mutator = mutator;
		}
		
		@Override
		public AbstractOMXQueryControlAccessor<P, T, R> setTimeout(Duration timeout) {
			// We can pass our parent ref to the child because we have the same
			// state
			return chained(timeout, null);
		}
		
		@Override
		public AbstractOMXQueryControlAccessor<P, T, T> get() {
			return thenApply(state -> state.setResult(state.getValue()));
		}
		
		@Override
		public AbstractOMXQueryControlAccessor<P, T, R> get(Consumer<T> handler) {
			return thenApply(state -> handler.accept(state.getValue()));
		}
		
		@Override
		public AbstractOMXQueryControlAccessor<P, T, R> get(BiFunction<T, R, E> merger) {
			return thenApply(state -> state.setResult(merger.apply(state.getValue(), state.getResult())));
		}
		
		@Override
		public AbstractOMXQueryControlAccessor<P, T, R> set(T value) {
			return thenApply(state -> state.setValue(value));
		}
		
		@Override
		public <E> AbstractOMXQueryControlAccessor<P, T, R> set(String name, E value) {
			return set(name, () -> value);
		}
		
		@Override
		public AbstractOMXQueryControlAccessor<P, T, R> set(Supplier<T> supplier) {
			return this.thenApply(state -> state.setValue(supplier.get()));
		}
		
		@Override
		public abstract <E> AbstractOMXQueryControlAccessor<P, T, R> set(String name, Supplier<E> supplier);
		
		@Override
		public AbstractOMXQueryControlAccessor<P, T, R> update(Function<T, T> mappingFunction) {
			return thenApply(state -> state.setValue(mappingFunction.apply(state.getValue())));
		}
		
		@Override
		@SuppressWarnings("unchecked")
		public <E extends Object> AbstractOMXQueryControlAccessor<P, T, R> update(String name, BiFunction<String, E, E> mappingFunction) {
			return thenApply(
					state -> state.basePointer.compute(name, (BiFunction<String, Object, Object>) mappingFunction));
		}
		
		@Override
		public AbstractOMXQueryControlAccessor<P, T, R> read() {
			return this.thenApply(state -> state.setValue(state.localPointer().get()));
		}
		
		@Override
		public AbstractOMXQueryControlAccessor<P, T, R> write() {
			return thenApply(state -> state.localPointer.set(state.getValue()));
		}
		
		@Override
		public AbstractOMXQueryControlAccessor<P, T, R> increase() {
			return this;
		}
		
		@Override
		public AbstractOMXQueryControlAccessor<P, T, R> decrease() {
			return this;
		}
		
		@Override
		public abstract AbstractOMXQueryControlAccessor<P, T, T> writeAndRead();
		
		protected OMXQueryControlAccessorState enterFromParent(OMXQueryControlAccessorState parentState) {
			OMXQueryControlAccessorState result = new OMXQueryControlAccessorState();
			result.basePointer = parentState.basePointer;
			result.result = parentState.result;
		}
		
		protected void exitToParent(OMXQueryControlAccessorState parentState, OMXQueryControlAccessorState childState) {
			parentState.result = childState.result;
		}
		
		@Override
		@SuppressWarnings("unchecked")
		public P and() {
			// Traverse backwards, finding owner
			AbstractOMXQueryControlAccessor<?, ?, ?> root = this;
			while (!root.isParentOwner) {
				if (root.parent == null)
					throw new IllegalStateException("No owner exists for this accessor");
				root = root.parent;
			}
			AbstractOMXQueryControlAccessor<?, ?, ?> owner = root.parent;
			// Add the call chain of the child methods up to the root
			// to the owner, and return
			return (P) owner.thenApply(parentState -> {
				OMXQueryControlAccessorState childState = enterFromParent(parentState);
				doCall(childState);
				exitToParent(parentState, childState);
			});
		}
		
		/**
		 * Get the parent reference that should be used for child objects
		 * generated by this object.
		 * 
		 * @return
		 */
		protected AbstractOMXQueryControlAccessor<?, ?, ?> doGetChildParent() {
			if (this.mutator == null)
				return this.parent;
			return this;
		}
		
		/**
		 * Push mutator action onto stack
		 */
		protected <X, Z> AbstractOMXQueryControlAccessor<X, T, Z> thenApply(Consumer<OMXQueryControlAccessorState> mutator) {
			return chained(timeout, mutator);
		}
		
		protected <X, Z> AbstractOMXQueryControlAccessor<X, T, Z> chained(Duration timeout, Consumer<OMXQueryControlAccessorState> mutator) {
			AbstractOMXQueryControlAccessor<?, ?, ?> parent = doGetChildParent();
			return chained(this.isParentOwner && parent != this, this.name, parent, timeout, mutator);
		}
		
		protected abstract <X, Z> AbstractOMXQueryControlAccessor<X, T, Z> chained(boolean isParentOwner, String name,
				AbstractOMXQueryControlAccessor<?, ?, ?> parent, Duration timeout, Consumer<OMXQueryControlAccessorState> mutator);
		
		/**
		 * Method that actually does stuff when invoked.
		 * 
		 * @throws Exception
		 */
		protected void doCall(OMXQueryControlAccessorState state) {
			if (this.parent != null && !this.isParentOwner)
				this.parent.doCall(state);
			if (this.mutator != null)
				this.mutator.accept(state);
		}
		
		@Override
		@SuppressWarnings("unchecked")
		public R call() throws Exception {
			try (OMXQueryControlAccessorState state = new OMXQueryControlAccessorState()) {
				doCall(state);
				return (R) state.result;
			}
		}
		
	}
	
	protected static class OMXQueryControlAccessorState implements AutoCloseable {
		NativeStruct basePointer;
		Object result = null;
		Object value = null;
		NativePointer<?> localPointer = null;
		
		@SuppressWarnings("unchecked")
		public <P extends NativePointer<?>> P localPointer() {
			return (P) localPointer;
		}
		
		public <T> T setValue(T value) {
			this.value = value;
			return value;
		}
		
		public <T> T getValue() {
			return (T) this.value;
		}
		
		public <T> T setResult(T value) {
			this.result = value;
			return value;
		}
		
		@SuppressWarnings("unchecked")
		public <T> T getResult() {
			return (T) this.result;
		}
		
		@Override
		public void close() throws Exception {
			basePointer.close();
		}
	}
}
