package au.edu.jcu.v4l4j.impl.omx;

import java.nio.ByteBuffer;
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
import au.edu.jcu.v4l4j.impl.jni.NativeStruct;
import au.edu.jcu.v4l4j.impl.jni.StructPrototype;

/**
 * A root composite control that when push or pull is called, executes a
 * 
 * @author mailmindlin
 */
public class BaseOMXQueryControl extends AbstractOMXQueryControl<Map<String, Object>> implements CompositeControl {
	protected final Set<AbstractOMXQueryControl<?>> children = new HashSet<>();
	protected transient Map<String, AbstractOMXQueryControl<?>> childMap;
	protected final boolean isConfig = false;
	protected final int queryId;
	protected final StructPrototype struct;
	
	public static OMXQueryControlBuilder builder(StructPrototype struct) {
		return new OMXQueryControlBuilder(struct);
	}
	
	public BaseOMXQueryControl(OMXComponent component, String rootName, int queryId, StructPrototype struct) {
		super(component, null, rootName, null);
		this.queryId = queryId;
		this.struct = struct;
	}

	@Override
	public boolean isDiscrete() {
		return false;
	}

	@Override
	public boolean isContinuous() {
		return false;
	}
	
	@Override
	public Set<? extends Control<?>> getChildren() {
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
		this.childMap.put(name, (AbstractOMXQueryControl<?>) child);
		if (child.getType() == ControlType.COMPOSITE)
			for (Control<?> c : ((CompositeControl) child).getChildren())
				addToMap(name, c);
	}
	
	@Override
	public ControlAccessor<Void, Map<String, Object>, Map<String, Object>> get() {
		return access().get();
	}
	
	@Override
	public AbstractOMXQueryControlAccessor<Void, Map<String, Object>, Void> access() {
		return new OMXQueryControlBaseAccessor<Void, Void>(false, this.name, null, null, null);
	}
	
	@Override
	protected <P, R> AbstractOMXQueryControlAccessor<P, Map<String, Object>, R> access(AbstractOMXQueryControlAccessor<P, ?, R> parent) {
		throw new UnsupportedOperationException("Control is base");
	}
	
	protected void doWrite(ByteBuffer buffer) {
		this.component.setConfig(this.isConfig, this.queryId, buffer);
	}
	
	public static class OMXQueryControlBuilder {
		protected final StructPrototype struct;
		
		public OMXQueryControlBuilder(StructPrototype struct) {
			this.struct = struct;
		}
		
		public OMXQueryControlBuilder add(String ctrlName) {
			return this;
		}
		
		public BaseOMXQueryControl build(OMXComponent component, String rootName, int queryId) {
			BaseOMXQueryControl control = new BaseOMXQueryControl(component, rootName, queryId, struct);
			
			return control;
		}
	}
	
	/**
	 * @author mailmindlin
	 *
	 * @param <P>
	 * @param <R>
	 */
	protected class OMXQueryControlBaseAccessor<P, R> extends AbstractOMXQueryControlAccessor<P, Map<String, Object>, R> {
		
		protected OMXQueryControlBaseAccessor(boolean isParentOwner, String name,
				AbstractOMXQueryControlAccessor<?, ?, ?> parent, Duration timeout,
				Consumer<OMXQueryControlAccessorState> mutator) {
			super(isParentOwner, name, parent, timeout, mutator);
		}

		@Override
		@SuppressWarnings("unchecked")
		public <Ct, C extends ControlAccessor<ControlAccessor<P, Map<String, Object>, R>, Ct, R>> C withChild(String name) {
			return (C) childMap.get(name).access(this);
		}
		
		@Override
		protected <X, Z> AbstractOMXQueryControlAccessor<X, Map<String, Object>, Z> chained(boolean isParentOwner, String name, AbstractOMXQueryControlAccessor<?, ?, ?> parent, Duration timeout, Consumer<OMXQueryControlAccessorState> mutator) {
			return new OMXQueryControlBaseAccessor<>(isParentOwner, name, parent, timeout, mutator);
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
			return thenApply(state -> BaseOMXQueryControl.this.doWrite(state.basePointer.buffer()));
		}
		
		@Override
		@SuppressWarnings("unchecked")
		public <E extends Object> AbstractOMXQueryControlAccessor<P, Map<String, Object>, R> update(String name,
				BiFunction<String, E, E> mappingFunction) {
			return thenApply(state -> state.basePointer.compute(name, (BiFunction<String, Object, Object>) mappingFunction));
		}

		@Override
		public BaseOMXQueryControl getControl() {
			return BaseOMXQueryControl.this;
		}

		@Override
		public AbstractOMXQueryControlAccessor<P, Map<String, Object>, R> writeAndRead() {
			return thenApply(state -> {
				OMXComponent component = BaseOMXQueryControl.this.component;
				int queryId = BaseOMXQueryControl.this.queryId;
				ByteBuffer buffer = state.basePointer.buffer();
				component.setConfig(false, queryId, buffer);
				component.getConfig(false, queryId, buffer);
			});
		}
		
	}
}
