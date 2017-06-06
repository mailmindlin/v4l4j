package au.edu.jcu.v4l4j.impl.omx;

import java.time.Duration;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import java.util.function.Consumer;

import au.edu.jcu.v4l4j.api.control.CompositeControl;
import au.edu.jcu.v4l4j.api.control.Control;
import au.edu.jcu.v4l4j.api.control.ControlType;
import au.edu.jcu.v4l4j.impl.jni.NativeWrapper;

public class CompositeOMXQueryControl extends AbstractOMXQueryControl<Map<String, Object>> implements CompositeControl{
	protected final Set<AbstractOMXQueryControl<?>> children = new HashSet<>();
	protected transient Map<String, AbstractOMXQueryControl<?>> childMap = new HashMap<>();
	
	protected CompositeOMXQueryControl(AbstractOMXQueryControl<?> parent, int port, String name, String structFieldName, OMXOptionEnumeratorPrototype<Map<String, Object>> enumerator) {
		super(parent.component, port, parent, name, structFieldName, enumerator);
	}
	
	public void registerChild(AbstractOMXQueryControl<?> child) {
		children.add(child);
		childMap.put(child.name, child);
	}
	
	@Override
	public ControlType getType() {
		return CompositeControl.super.getType();
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
	public Control<?> getChildByName(String name) {
		return childMap.get(name);
	}
	
	@Override
	public ControlAccessor<Void, Map<String, Object>, Map<String, Object>> get() {
		return access().read().get();
	}
	
	@Override
	@SuppressWarnings("unchecked")
	protected <P, R> AbstractOMXQueryControlAccessor<P, Map<String, Object>, R> access(AbstractOMXQueryControlAccessor<P, ?, R> parentAccessor) {
		return new CompositeOMXQueryControlAccessor<>((P) parentAccessor, this.name, null, parentAccessor.timeout, null);
	}

	public class CompositeOMXQueryControlAccessor<P, R> extends AbstractOMXQueryControlAccessor<P, Map<String, Object>, R> {
		protected CompositeOMXQueryControlAccessor(P owner, String name, AbstractOMXQueryControlAccessor<?, ?, ?> parent, Duration timeout, Consumer<OMXQueryControlAccessorState> mutator) {
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
		protected OMXQueryControlAccessorState enterFromParent(OMXQueryControlAccessorState parentState) {
			OMXQueryControlAccessorState state = super.enterFromParent(parentState);
			NativeWrapper<String, ?> parentStruct = parentState.localPointer();
			state.localPointer = parentStruct.getChildRemote(structFieldName);
			if (state.localPointer == null)
				state.localPointer = parentStruct.getChild(structFieldName);
			if (state.localPointer == null)
				throw new RuntimeException(
						"Control " + getName() + ": Can't find field " + structFieldName + " on parent struct " + parentStruct);
			return state;
		}
		
		@Override
		public CompositeOMXQueryControl getControl() {
			return CompositeOMXQueryControl.this;
		}
		
		@Override
		protected <X, Z> AbstractOMXQueryControlAccessor<X, Map<String, Object>, Z> chained(X owner, String name,
				AbstractOMXQueryControlAccessor<?, ?, ?> parent, Duration timeout, Consumer<OMXQueryControlAccessorState> mutator) {
			return new CompositeOMXQueryControlAccessor<>(owner, name, parent, timeout, mutator);
		}
	}
}
