package au.edu.jcu.v4l4j.impl.omx;

import java.nio.ByteBuffer;
import java.time.Duration;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import java.util.Stack;
import java.util.function.BiFunction;
import java.util.function.Consumer;
import java.util.function.Function;
import java.util.function.Supplier;

import au.edu.jcu.v4l4j.api.control.CompositeControl;
import au.edu.jcu.v4l4j.api.control.Control;
import au.edu.jcu.v4l4j.api.control.ControlType;
import au.edu.jcu.v4l4j.impl.jni.NativePointer;
import au.edu.jcu.v4l4j.impl.jni.NativeStruct;
import au.edu.jcu.v4l4j.impl.jni.NativeWrapper;
import au.edu.jcu.v4l4j.impl.jni.StructPrototype;

/**
 * A root composite control that when push or pull is called, executes a
 * 
 * @author mailmindlin
 */
public class OMXQueryControl implements CompositeControl {
	protected final OMXComponent component;
	protected final Set<Control<?>> children = new HashSet<>();
	protected transient Map<String, Control<?>> childMap;
	protected final String rootName;
	protected final boolean isConfig = false;
	protected final int queryId;
	protected final StructPrototype struct;
	
	public static OMXQueryControlBuilder builder(StructPrototype struct) {
		return new OMXQueryControlBuilder(struct);
	}
	
	protected OMXQueryControl(OMXComponent component, String rootName, int queryId, StructPrototype struct) {
		this.component = component;
		this.rootName = rootName;
		this.queryId = queryId;
		this.struct = struct;
	}
	
	@Override
	public String getName() {
		return this.rootName;
	}
	
	@Override
	public Set<Control<?>> getChildren() {
		return this.children;
	}
	
	@Override
	public void close() {
		// TODO Auto-generated method stub
		
	}
	
	@Override
	public Control<?> getChildByName(String name) {
		if (childMap == null) {
			for (Control<?> child : this.children)
				addToMap("", child);
		}
		
		return childMap.get(name);
	}
	
	protected void addToMap(String prefix, Control<?> child) {
		String name = prefix + (prefix.isEmpty() ? "" : ".") + child.getName();
		this.childMap.put(name, child);
		if (child.getType() == ControlType.COMPOSITE)
			for (Control<?> c : ((CompositeControl) child).getChildren())
				addToMap(name, c);
	}
	
	@Override
	public CompositeControlAccessor<Void, Map<String, Object>, Map<String, Object>> get() {
		return access().get();
	}
	
	@Override
	public AbstractOMXQueryControlAccessor<Void, Map<String, Object>, Void> access() {
		//TODO finish
		return null;//new OMXQueryControlAccessor<Void, Map<String, Object>, Void>(null, null, null);
	}
	
	public static class OMXQueryControlBuilder {
		protected final StructPrototype struct;
		
		public OMXQueryControlBuilder(StructPrototype struct) {
			this.struct = struct;
		}
		
		public OMXQueryControl build(OMXComponent component, String rootName, int queryId) {
			OMXQueryControl control = new OMXQueryControl(component, rootName, queryId, struct);
			
			return control;
		}
	}
	
	/**
	 * @author mailmindlin
	 *
	 * @param <P>
	 * @param <T>
	 * @param <R>
	 */
	public abstract class AbstractOMXQueryControlAccessor<P, T, R> implements CompositeControlAccessor<P, T, R> {
		protected final boolean isParentOwner;
		protected final String name;
		protected final AbstractOMXQueryControlAccessor<?, ?, ?> parent;
		protected final Duration timeout;
		protected final Consumer<OMXQueryControlAccessorState> mutator;
		
		AbstractOMXQueryControlAccessor(String name, AbstractOMXQueryControlAccessor<?, ?, ?> parent, Duration timeout) {
			this(name, parent, timeout, null);
		}
		
		protected AbstractOMXQueryControlAccessor(String name, AbstractOMXQueryControlAccessor<?, ?, ?> parent, Duration timeout,
				Consumer<OMXQueryControlAccessorState> mutator) {
			this(false, name, parent, timeout, mutator);
		}
		
		protected AbstractOMXQueryControlAccessor(boolean isParentOwner, String name, AbstractOMXQueryControlAccessor<?, ?, ?> parent,
				Duration timeout, Consumer<OMXQueryControlAccessorState> mutator) {
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
		@SuppressWarnings("unchecked")
		public AbstractOMXQueryControlAccessor<P, T, T> get() {
			// Kajigger the casting chain, because casting is hard.
			// Note that this should be correct
			return (AbstractOMXQueryControlAccessor<P, T, T>) (Object) thenApply(state -> OMXQueryControl.this.component
					.accessConfig(false, true, OMXQueryControl.this.queryId, state.valueMap.buffer()));
		}
		
		@Override
		@SuppressWarnings("unchecked")
		public AbstractOMXQueryControlAccessor<P, T, R> read(Consumer<T> handler) {
			return this.thenApply(state -> handler.accept((T) state.localPointer.peek().get()));
		}
		
		@Override
		public abstract <E> AbstractOMXQueryControlAccessor<P, T, R> read(String name, Consumer<E> handler);
		
		@Override
		public AbstractOMXQueryControlAccessor<P, T, R> write(T value) {
			return this.write(() -> value);
		}
		
		@Override
		public <E> AbstractOMXQueryControlAccessor<P, T, R> write(String name, E value) {
			return write(name, () -> value);
		}
		
		@Override
		public abstract <E> AbstractOMXQueryControlAccessor<P, T, R> write(String name, Supplier<E> supplier);
		

		@Override
		@SuppressWarnings("unchecked")
		public AbstractOMXQueryControlAccessor<P, T, R> write(Supplier<T> supplier) {
			return this.thenApply(state->((NativePointer<T>)state.localPointer.peek()).set(supplier.get()));
		}
		
		@Override
		public AbstractOMXQueryControlAccessor<P, T, R> update(Function<T, T> mappingFunction) {
			// I'm thinking some kind of copy-mark changed-merge scheme, if I
			// ever get around to it...
			throw new UnsupportedOperationException("This one is hard to implement");
		}
		
		@Override
		@SuppressWarnings("unchecked")
		public <E extends Object> AbstractOMXQueryControlAccessor<P, T, R> update(String name,
				BiFunction<String, E, E> mappingFunction) {
			return thenApply(
					state -> state.valueMap.compute(name, (BiFunction<String, Object, Object>) mappingFunction));
		}
		
		@Override
		public AbstractOMXQueryControlAccessor<P, T, R> set() {
			return thenApply(state -> OMXQueryControl.this.component.accessConfig(false, false,
					OMXQueryControl.this.queryId, state.valueMap.buffer()));
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
		public AbstractOMXQueryControlAccessor<P, T, T> setAndGet() {
			return thenApply(state -> {
				OMXComponent component = OMXQueryControl.this.component;
				int queryId = OMXQueryControl.this.queryId;
				ByteBuffer buffer = state.valueMap.buffer();
				component.accessConfig(false, false, queryId, buffer);// set
				component.accessConfig(false, true, queryId, buffer);// get
			});
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
			return (P) owner.thenApply(this::doCall);
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
		
		protected <X, Z> AbstractOMXQueryControlAccessor<X, T, Z> chained(Duration timeout,
				Consumer<OMXQueryControlAccessorState> mutator) {
			AbstractOMXQueryControlAccessor<?, ?, ?> parent = doGetChildParent();
			return chained(this.isParentOwner && parent != this, this.name, parent, timeout, mutator);
		}
		
		protected abstract <X, Z> AbstractOMXQueryControlAccessor<X, T, Z> chained(boolean isParentOwner, String name,
				AbstractOMXQueryControlAccessor<?, ?, ?> parent, Duration timeout,
				Consumer<OMXQueryControlAccessorState> mutator);
		
		/**
		 * Method that actually does stuff when invoked.
		 * 
		 * @throws Exception
		 */
		protected void doCall(OMXQueryControlAccessorState state) {
			if (this.parent != null && !this.isParentOwner)
				this.parent.doCall(state);
			else if (this.isParentOwner) {
				//Push state
				state.localPointer.push(((NativeWrapper<String, ?>)state.localPointer.peek()).getChild(this.name));
			}
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
	
	protected class OMXQueryControlBaseAccessor<P, T, R> extends AbstractOMXQueryControlAccessor<P, T, R> {
		
		protected OMXQueryControlBaseAccessor(boolean isParentOwner, String name,
				AbstractOMXQueryControlAccessor<?, ?, ?> parent, Duration timeout,
				Consumer<OMXQueryControlAccessorState> mutator) {
			super(isParentOwner, name, parent, timeout, mutator);
		}
		
		@Override
		public <Ct, C extends ControlAccessor<CompositeControlAccessor<P, T, R>, Ct, R>> C withChild(String name) {
			// TODO Auto-generated method stub
			return null;
		}
		
		@Override
		protected <X, Z> AbstractOMXQueryControlAccessor<X, T, Z> chained(boolean isParentOwner, String name, AbstractOMXQueryControlAccessor<?, ?, ?> parent, Duration timeout, Consumer<OMXQueryControlAccessorState> mutator) {
			return new OMXQueryControlBaseAccessor<>(isParentOwner, name, parent, timeout, mutator);
		}
		
		@Override
		@SuppressWarnings("unchecked")
		public <E> AbstractOMXQueryControlAccessor<P, T, R> read(String name, Consumer<E> handler) {
			return this.thenApply(state -> handler.accept((E) ((NativeStruct) state.localPointer.peek()).get(name)));
		}
		
		@Override
		public <E> AbstractOMXQueryControlAccessor<P, T, R> write(String name, Supplier<E> supplier) {
			return this.thenApply(state -> {
				
			});
		}
		
	}
	
	protected static class OMXQueryControlAccessorState implements AutoCloseable {
		NativeStruct valueMap;
		Object result = null;
		Stack<NativePointer<?>> localPointer = new Stack<>();
		
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
			valueMap.close();
		}
	}
}
