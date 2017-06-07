package au.edu.jcu.v4l4j.impl.omx;

import java.util.HashMap;
import java.time.Duration;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import java.util.function.BiFunction;
import java.util.function.Consumer;
import java.util.function.Supplier;

import au.edu.jcu.v4l4j.api.control.CompositeControl;
import au.edu.jcu.v4l4j.api.control.Control;
import au.edu.jcu.v4l4j.api.control.ControlType;
import au.edu.jcu.v4l4j.api.control.Control.ControlAccessor;
import au.edu.jcu.v4l4j.impl.jni.NativeStruct;
import au.edu.jcu.v4l4j.impl.jni.StructPrototype;

/**
 * A root composite control that when push or pull is called, executes a
 * 
 * @author mailmindlin
 */
public class BaseOMXQueryControl extends AbstractOMXQueryControl<Map<String, Object>> implements CompositeControl {
	protected final Set<AbstractOMXQueryControl<?>> children = new HashSet<>();
	protected transient Map<String, AbstractOMXQueryControl<?>> childMap = new HashMap<>();
	protected final boolean isConfig = false;
	protected final int queryId;
	protected final StructPrototype struct;
	
	public BaseOMXQueryControl(OMXComponent component, String rootName, int queryId, int portIdx, StructPrototype struct, OMXOptionEnumeratorPrototype<Map<String, Object>> enumerator) {
		super(component, portIdx, null, rootName, null, enumerator);
		this.queryId = queryId;
		this.struct = struct;
	}

	@Override
	public boolean isContinuous() {
		return false;
	}
	
	@Override
	public Set<? extends Control<?>> getChildren() {
		return this.children;
	}
	
	protected <T extends AbstractOMXQueryControl<?>> T registerChild(T child) {
		this.children.add(child);
		this.childMap.put(child.name, child);
		return child;
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
		this.childMap.put(name, (AbstractOMXQueryControl<?>) child);
		if (child.getType() == ControlType.COMPOSITE)
			for (Control<?> c : ((CompositeControl) child).getChildren())
				addToMap(name, c);
	}
	
	@Override
	public ControlAccessor<Void, Map<String, Object>, Map<String, Object>> get() {
		return access().read().get();
	}
	
	@Override
	public AbstractOMXQueryControlAccessor<Void, Map<String, Object>, Void> access() {
		return new OMXQueryControlBaseAccessor<Void, Void>(null, this.name, null, null, null);
	}
	
	@Override
	protected <P, R> AbstractOMXQueryControlAccessor<P, Map<String, Object>, R> access(AbstractOMXQueryControlAccessor<P, ?, R> parent) {
		throw new UnsupportedOperationException("Control is base");
	}
	
	@Override
	public ControlType getType() {
		return ControlType.COMPOSITE;
	}
	
	protected void doWrite(NativeStruct query) {
		if (this.port >= 0)
			query.put("nPortIndex", this.port);
		this.component.setConfig(this.isConfig, this.queryId, query.buffer());
	}
	
	protected void doRead(NativeStruct query) {
		if (this.port >= 0)
			query.put("nPortIndex", this.port);
		try {
			this.component.getConfig(this.isConfig, this.queryId, query.buffer());
		} catch (Exception e) {
			for (String key : query.keySet())
				System.out.println(key + "|" + query.get(key));
			throw e;
		}
	}
	
	@Override
	public String toString() {
		return new StringBuffer()
				.append(getClass().getName()).append("{")
				.append("name=").append(this.getName())
				.append(", query=").append(this.queryId)
				.append(", type=").append(getType())
				.append(", port=").append(this.port)
				.append(", struct=").append(this.struct)
				.append(", children=").append(this.childMap)
				//TODO add more
				.append("}")
				.toString();
	}
	
	/**
	 * @author mailmindlin
	 *
	 * @param <P>
	 * @param <R>
	 */
	protected class OMXQueryControlBaseAccessor<P, R> extends AbstractOMXQueryControlAccessor<P, Map<String, Object>, R> {
		
		protected OMXQueryControlBaseAccessor(P owner, String name,
				AbstractOMXQueryControlAccessor<?, ?, ?> parent, Duration timeout,
				Consumer<OMXQueryControlAccessorState> mutator) {
			super(owner, name, parent, timeout, mutator);
		}

		@Override
		@SuppressWarnings("unchecked")
		public <Ct> ControlAccessor<ControlAccessor<P, Map<String, Object>, R>, Ct, R> withChild(String name) {
			AbstractOMXQueryControl<Ct> child = (AbstractOMXQueryControl<Ct>) childMap.get(name);
			if (child == null)
				throw new IllegalArgumentException("No child exists with for the name: " + name);
			
			return (ControlAccessor<ControlAccessor<P, Map<String, Object>, R>, Ct, R>) child.access(this);
		}
		
		@Override
		protected <X, Z> AbstractOMXQueryControlAccessor<X, Map<String, Object>, Z> chained(X owner, String name, AbstractOMXQueryControlAccessor<?, ?, ?> parent, Duration timeout, Consumer<OMXQueryControlAccessorState> mutator) {
			return new OMXQueryControlBaseAccessor<>(owner, name, parent, timeout, mutator);
		}
		
		@Override
		public <E> AbstractOMXQueryControlAccessor<P, Map<String, Object>, R> set(String name, Supplier<E> supplier) {
			return this.thenApply(state->state.<NativeStruct>localPointer().put(name, supplier.get()));
		}

		@Override
		public AbstractOMXQueryControlAccessor<P, Map<String, Object>, R> set(Supplier<Map<String, Object>> supplier) {
			return this.thenApply(state->state.<NativeStruct>localPointer().putAll(supplier.get()));
		}

		@Override
		public AbstractOMXQueryControlAccessor<P, Map<String, Object>, R> write() {
			return thenApply(state -> BaseOMXQueryControl.this.doWrite(state.localPointer()));
		}
		
		@Override
		public AbstractOMXQueryControlAccessor<P, Map<String, Object>, R> read() {
			return thenApply(state -> BaseOMXQueryControl.this.doRead(state.localPointer()));
		}

		@Override
		@SuppressWarnings("unchecked")
		public <E extends Object> AbstractOMXQueryControlAccessor<P, Map<String, Object>, R> update(String name,
				BiFunction<String, E, E> mappingFunction) {
			return thenApply(state -> state.<NativeStruct>localPointer().compute(name, (BiFunction<String, Object, Object>) mappingFunction));
		}

		@Override
		public BaseOMXQueryControl getControl() {
			return BaseOMXQueryControl.this;
		}

		@Override
		public AbstractOMXQueryControlAccessor<P, Map<String, Object>, R> writeAndRead() {
			return thenApply(state -> {
				BaseOMXQueryControl.this.doWrite(state.localPointer());
				BaseOMXQueryControl.this.doRead(state.localPointer());
			});
		}

		@Override
		protected void doCall(OMXQueryControlAccessorState state) {
			if (state.localPointer == null)
				state.localPointer = new NativeStruct(struct);
			super.doCall(state);
		}
		
	}
}
