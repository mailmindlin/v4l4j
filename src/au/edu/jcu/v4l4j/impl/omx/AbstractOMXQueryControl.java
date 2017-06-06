package au.edu.jcu.v4l4j.impl.omx;

import java.time.Duration;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.Future;
import java.util.function.BiFunction;
import java.util.function.Consumer;
import java.util.function.Function;
import java.util.function.Predicate;
import java.util.function.Supplier;

import au.edu.jcu.v4l4j.api.CloseableIterator;
import au.edu.jcu.v4l4j.api.control.Control;
import au.edu.jcu.v4l4j.api.control.DiscreteControl;
import au.edu.jcu.v4l4j.impl.jni.NativePointer;
import au.edu.jcu.v4l4j.impl.jni.NativeStruct;
import au.edu.jcu.v4l4j.impl.jni.NativeWrapper;

public abstract class AbstractOMXQueryControl<T> implements DiscreteControl<T> {
	protected final OMXComponent component;
	protected final int port;
	protected final AbstractOMXQueryControl<?> parent;
	protected final String name;
	protected final String structFieldName;
	protected final OMXOptionEnumeratorPrototype<T> enumerator;
	
	protected AbstractOMXQueryControl(OMXComponent component, int port, AbstractOMXQueryControl<?> parent, String name, String structFieldName, OMXOptionEnumeratorPrototype<T> enumerator) {
		this.component = component;
		this.port = port;
		this.parent = parent;
		this.name = name;
		this.structFieldName = structFieldName;
		this.enumerator = enumerator;
	}
	
	@Override
	public String getName() {
		return this.name;
	}
	
	@Override
	public boolean isDiscrete() {
		return enumerator != null;
	}
	
	@Override
	public Future<CloseableIterator<T>> options() {
		return CompletableFuture.completedFuture(enumerator.iterate(this.component, this.port));
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
	
	public static abstract class AbstractOMXQueryControlAccessor<P, T, R> implements ControlAccessor<P, T, R> {
		protected final String name;
		protected final P owner;
		protected final AbstractOMXQueryControlAccessor<?, ?, ?> parent;
		protected final Duration timeout;
		protected final Consumer<OMXQueryControlAccessorState> mutator;
		
		protected AbstractOMXQueryControlAccessor(String name, AbstractOMXQueryControlAccessor<?, ?, ?> owner, Duration timeout) {
			this(name, owner, timeout, null);
		}
		
		protected AbstractOMXQueryControlAccessor(String name, AbstractOMXQueryControlAccessor<?, ?, ?> parent,
				Duration timeout, Consumer<OMXQueryControlAccessorState> mutator) {
			this(null, name, parent, timeout, mutator);
		}
		
		protected AbstractOMXQueryControlAccessor(P owner, String name,
				AbstractOMXQueryControlAccessor<?, ?, ?> parent, Duration timeout,
				Consumer<OMXQueryControlAccessorState> mutator) {
			this.owner = owner;
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
		public <E> AbstractOMXQueryControlAccessor<P, T, E> get(BiFunction<T, ? super R, E> merger) {
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
		public <E> AbstractOMXQueryControlAccessor<P, T, R> set(String name, Supplier<E> supplier) {
			throw new UnsupportedOperationException();
		}
		
		@Override
		public AbstractOMXQueryControlAccessor<P, T, R> update(Function<T, T> mappingFunction) {
			return thenApply(state -> state.setValue(mappingFunction.apply(state.getValue())));
		}
		
		@Override
		@SuppressWarnings("unchecked")
		public <E> AbstractOMXQueryControlAccessor<P, T, R> update(String name, BiFunction<String, E, E> mappingFunction) {
			//TODO check correctness for nested children
			return thenApply(state -> state.<NativeStruct>localPointer().compute(name, (BiFunction<String, Object, Object>) mappingFunction));
		}
		
		@Override
		public ControlAccessor<P, T, R> update(BiFunction<Control<T>, T, T> mappingFunction) {
			return thenApply(state -> state.setValue(mappingFunction.apply(getControl(), state.getValue())));
		}
		
		@Override
		public AbstractOMXQueryControlAccessor<P, T, R> read() {
			//TODO we need to call up the ownership chain
			/*return this.thenApply(state -> {
				state.setValue(state.localPointer().get());
			});*/
			/* 
			 * XXX This method is horribly broken. Please don't use it, because it doesn't do what
			 * it says it does.
			 * I have to rewrite it to call write() all the way up the ownership chain.
			 */
			throw new UnsupportedOperationException("Things are broken now, but I promise to fix them later");
		}
		
		@Override
		public AbstractOMXQueryControlAccessor<P, T, R> write() {
			//TODO we can do a TCO-like optimization by only invoking thenApply() on the base accessor
			/*return thenApply(state -> {
				//TODO is this actually needed?
				//state.localPointer.set(state.getValue());
				if (this.owner != null)
					((AbstractOMXQueryControlAccessor<?,?,?>)this.owner).write();
			});*/
			/* 
			 * XXX This method is horribly broken. Please don't use it, because it doesn't do what
			 * it says it does.
			 * I have to rewrite it to call write() all the way up the ownership chain.
			 */
			throw new UnsupportedOperationException("Things are broken now, but I promise to fix them later");
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
		public AbstractOMXQueryControlAccessor<P, T, R> writeAndRead() {
			/*return thenApply(state -> {
				//TODO do we need to call up the ownership chain? (I think so)
				state.localPointer.set(state.getValue());
				state.setValue(state.localPointer.get());
			});*/
			/* 
			 * XXX This method is horribly broken. Please don't use it, because it doesn't do what
			 * it says it does.
			 * I have to rewrite it to call write() all the way up the ownership chain.
			 */
			throw new UnsupportedOperationException("Things are broken now, but I promise to fix them later");
		}

		@Override
		public <C extends ControlAccessor<ControlAccessor<P, T, R>, T, R>> C thenIf(Predicate<ControlAccessor<P, T, R>> condition) {
			//TODO This is going to be hard to implement. Imma do it eventually, but just not right now
			throw new UnsupportedOperationException("Coming soon!");
		}

		protected OMXQueryControlAccessorState enterFromParent(OMXQueryControlAccessorState parentState) {
			OMXQueryControlAccessorState result = new OMXQueryControlAccessorState();
			//Override me for different behavior
			result.localPointer = parentState.<NativeWrapper<String, ?>>localPointer().getChild(((AbstractOMXQueryControl<?>)this.getControl()).structFieldName);
			result.result = parentState.result;
			return result;
		}
		
		protected void exitToParent(OMXQueryControlAccessorState parentState, OMXQueryControlAccessorState childState) {
			parentState.result = childState.result;
		}
		
		@Override
		@SuppressWarnings("unchecked")
		public P and() {
			if (this.owner == null)
				throw new UnsupportedOperationException();
			// Add the call chain of the child methods up to the root
			// to the parent, and return
			return (P) ((AbstractOMXQueryControlAccessor<?,?,?>)this.owner).thenApply(parentState -> {
				OMXQueryControlAccessorState childState = enterFromParent(parentState);
				this.doCall(childState);
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
		protected <S> AbstractOMXQueryControlAccessor<P, T, S> thenApply(Consumer<OMXQueryControlAccessorState> mutator) {
			return chained(timeout, mutator);
		}
		
		protected <S> AbstractOMXQueryControlAccessor<P, T, S> chained(Duration timeout, Consumer<OMXQueryControlAccessorState> mutator) {
			AbstractOMXQueryControlAccessor<?, ?, ?> parent = doGetChildParent();
			return chained(this.owner, this.name, parent, timeout, mutator);
		}
		
		protected abstract <X, Z> AbstractOMXQueryControlAccessor<X, T, Z> chained(X owner, String name,
				AbstractOMXQueryControlAccessor<?, ?, ?> parent, Duration timeout, Consumer<OMXQueryControlAccessorState> mutator);
		
		/**
		 * Method that actually does stuff when invoked.
		 * 
		 * @throws Exception
		 */
		protected void doCall(OMXQueryControlAccessorState state) {
			if (this.parent != null)
				this.parent.doCall(state);
			if (this.mutator != null)
				this.mutator.accept(state);
		}
		
		@Override
		@SuppressWarnings("unchecked")
		public R call() throws Exception {
			//Pop up stack
			AbstractOMXQueryControlAccessor<?, ?, R> current = this;
			while (current.owner != null)
				current = (AbstractOMXQueryControlAccessor<?, ?, R>) current.and();
			
			try (OMXQueryControlAccessorState state = new OMXQueryControlAccessorState()) {
				current.doCall(state);
				return (R) state.result;
			}
		}
		
	}
	
	protected static class OMXQueryControlAccessorState implements AutoCloseable {
		Object result = null;
		NativePointer<?> localPointer = null;
		
		@SuppressWarnings("unchecked")
		public <P extends NativePointer<?>> P localPointer() {
			return (P) localPointer;
		}
		
		@SuppressWarnings("unchecked")
		public <K, P extends NativePointer<?>> P childPointer(K key) {
			NativeWrapper<K, P> wrapper = localPointer();
			P result = (P) wrapper.getChildRemote(key);
			if (result != null)
				return result;
			return (P) wrapper.getChild(key);
		}
		
		public <T> T setValue(T value) {
			this.<NativePointer<T>>localPointer().set(value);
			return value;
		}
		
		public <T> T getValue() {
			return (T) this.<NativePointer<T>>localPointer().get();
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
			if (localPointer != null)
				localPointer.close();
		}
	}
}
