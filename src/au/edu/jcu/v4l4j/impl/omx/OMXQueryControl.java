package au.edu.jcu.v4l4j.impl.omx;

import java.nio.ByteBuffer;
import java.time.Duration;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import java.util.function.BiFunction;
import java.util.function.Consumer;
import java.util.function.Function;
import java.util.function.Supplier;

import au.edu.jcu.v4l4j.api.control.CompositeControl;
import au.edu.jcu.v4l4j.api.control.Control;
import au.edu.jcu.v4l4j.api.control.ControlType;
import au.edu.jcu.v4l4j.impl.jni.NativeStruct;
import au.edu.jcu.v4l4j.impl.jni.StructField;
import au.edu.jcu.v4l4j.impl.jni.StructPrototype;

/**
 * A root composite control that when push or pull is called, executes a 
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
	
	public static OMXQueryControlBuilder builder(OMXComponent component, String rootName, int queryId, StructPrototype struct) {
		return new OMXQueryControlBuilder(OMXComponent component, String rootName, int queryId, StructPrototype struct);
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
			for (Control<?> c : ((CompositeControl)child).getChildren())
				addToMap(name, c);
	}

	@Override
	public CompositeControlGetter<Map<String, Object>, Map<String, Object>> get() {
		return access().get();
	}

	@Override
	public CompositeControlAccessor<Map<String, Object>, Void> access() {
		return new OMXQueryControlAccessor<>(null, null, null);
	}
	
	public static class OMXQueryControlBuilder {
		protected final StructPrototype struct;
		
		public OMXQueryControlBuilder(StructPrototype struct) {
			
		}
		
		public OMXQueryControl build(OMXComponent component, String rootName, int queryId) {
			OMXQueryControl control = new OMXQueryControl(component, rootName, queryId, struct);
			
			return control;
		}
	}
	
	public class OMXQueryControlAccessor<P, T, R> implements CompositeControlAccessor<P, T, R> {
		protected final boolean isParentOwner;
		protected final String name;
		protected final OMXQueryControlAccessor<?, ?, ?> parent;
		protected final Duration timeout;
		protected final Consumer<OMXQueryControlAccessorState> reader;
		
		protected OMXQueryControlAccessor(String name, OMXQueryControlAccessor<?, ?, ?> parent, Duration timeout, Consumer<OMXQueryControlAccessorState> reader) {
			this(false, name, parent, timeout, reader);
		}
		
		protected OMXQueryControlAccessor(boolean isParentOwner, String name, OMXQueryControlAccessor<?, ?, ?> parent, Duration timeout, Consumer<OMXQueryControlAccessorState> reader) {
			this.isParentOwner = isParentOwner;
			this.name = name;
			this.parent = parent;
			this.timeout = timeout;
			this.reader = reader;
		}
		
		@Override
		public OMXQueryControlAccessor<P, T, R> setTimeout(Duration timeout) {
			//We can pass our parent ref to the child because we have the same state
			return chained(timeout, null);
		}

		@Override
		public OMXQueryControlAccessor<P, T, T> get() {
			//Kajigger the casting chain, because casting is hard.
			//Note that this should be correct
			return (OMXQueryControlAccessor<P, T, T>) (Object) thenApply(state->OMXQueryControl.this.component.accessConfig(false, true, OMXQueryControl.this.queryId, state.valueMap.buffer()));
		}
		
		@Override
		public OMXQueryControlAccessor<P, T, R> read(Consumer<Map<String, Object>> handler) {
			return thenApply(state->handler.accept(state.valueMap));
		}

		@Override
		@SuppressWarnings("unchecked")
		public <E> OMXQueryControlAccessor<P, T, R> read(String name, Consumer<E> handler) {
			return read(map->handler.accept((E)map.get(name)));
		}

		@Override
		public OMXQueryControlAccessor<P, T, R> write(Map<String, Object> value) {
			return thenApply(state->state.valueMap.putAll(value));
		}

		@Override
		public OMXQueryControlAccessor<P, T, R> write(Supplier<Map<String, Object>> supplier) {
			return thenApply(state->state.valueMap.putAll(supplier.get()));
		}

		@Override
		public <E> OMXQueryControlAccessor<P, T, R> write(String name, Supplier<E> supplier) {
			return thenApply(state->state.valueMap.put(name, supplier.get()));
		}

		@Override
		public <E> OMXQueryControlAccessor<P, T, R> write(String name, E value) {
			return thenApply(state->state.valueMap.put(name, value));
		}

		@Override
		public OMXQueryControlAccessor<P, T, R> update(Function<Map<String, Object>, Map<String, Object>> mappingFunction) {
			//I'm thinking some kind of copy-mark changed-merge scheme, if I ever get around to it...
			throw new UnsupportedOperationException("This one is hard to implement");
		}

		@Override
		@SuppressWarnings("unchecked")
		public <E extends Object> OMXQueryControlAccessor<P, T, R> update(String name, BiFunction<String, E, E> mappingFunction) {
			return thenApply(state->state.valueMap.compute(name, (BiFunction<String, Object, Object>)mappingFunction));
		}

		@Override
		public OMXQueryControlAccessor<P, T, R> set() {
			return thenApply(state->OMXQueryControl.this.component.accessConfig(false, false, OMXQueryControl.this.queryId, state.valueMap.buffer()));
		}

		@Override
		public OMXQueryControlAccessor<P, T, R> setAndGet() {
			return thenApply(state->{
				OMXComponent component = OMXQueryControl.this.component;
				int queryId = OMXQueryControl.this.queryId;
				ByteBuffer buffer = state.valueMap.buffer();
				component.accessConfig(false, false, queryId, buffer);//set
				component.accessConfig(false, true, queryId, buffer);//get
			});
		}
		
		@Override
		public P and() {
			//Traverse backwards, finding owner
			OMXQueryControlAccessor<?, ?, ?> root = this;
			while (!root.isParentOwner) {
				if (root.parent = null)
					throw new IllegalStateException("No owner exists for this accessor");
				root = root.parent;
			}
			OMXQueryControlAccessor owner = root.parent;
			//Add the call chain of the child methods up to the root
			//to the owner, and return
			return (P) owner.thenApply(this::doCall);
		}
		
		/**
		 * Get the parent reference that should be used for child objects
		 * generated by this object.
		 * @return
		 */
		protected OMXQueryControlAccessor<?, ?, ?> doGetChildParent() {
			if (this.reader == null)
				return this.parent;
			return this;
		}
		
		/**
		 * Push mutator action onto stack
		 */
		protected OMXQueryControlAccessor<P, T, R> thenApply(Consumer<OMXQueryControlAccessorState> mutator) {
			return chained(timeout, mutator);
		}
		
		protected OMXQueryControlAccessor<P, T, R> chained(Duration timeout, Consumer<OMXQueryControlAccessorState> mutator) {
			OMXQueryControlAccessor<?, ?, ?> parent = doGetChildParent();
			return chained(this.isParentOwner && parent != this, this.name, parent, timeout, mutator);
		}
		
		protected abstract OMXQueryControlAccessor<P, T, R> chained(boolean isParentOwner, String name, OMXQueryControlAccessor<?, ?, ?> parent, Duration timeout, Consumer<OMXQueryControlAccessorState> mutator);
		
		/**
		 * Method that actually does stuff when invoked.
		 * @throws Exception
		 */
		protected void doCall(OMXQueryControlAccessorState state) {
			if (this.parent != null && !this.isParentOwner)
				this.parent.doCall(state);
			if (this.reader != null)
				this.reader.accept(state);
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
		NativeStruct valueMap;
		Set<ByteBuffer> unmanagedRefs = new HashSet<>();
		Object result = null;
		@Override
		public void close() throws Exception {
			valueMap.close();
		}
	}
}
