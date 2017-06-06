package au.edu.jcu.v4l4j.impl.omx;

import java.time.Duration;
import java.util.function.Consumer;

import au.edu.jcu.v4l4j.api.control.ControlType;

public class ChildEndpointOMXQueryControl<E> extends AbstractOMXQueryControl<E> {
	
	public ChildEndpointOMXQueryControl(AbstractOMXQueryControl<?> parent, int port, String name, String structFieldName,
			OMXOptionEnumeratorPrototype<E> enumerator) {
		super(parent.component, port, parent, name, structFieldName, enumerator);
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
	public ControlType getType() {
		return ControlType.SLIDER;
	}
	
	@Override
	@SuppressWarnings("unchecked")
	protected <P, R> AbstractOMXQueryControlAccessor<P, E, R> access(AbstractOMXQueryControlAccessor<P, ?, R> parentAccessor) {
		return new ChildEndpointOMXQueryControlAccessor<>((P) parentAccessor, this.name, null, parentAccessor.timeout, null);
	}
	
	public class ChildEndpointOMXQueryControlAccessor<P, R> extends AbstractOMXQueryControlAccessor<P, E, R> {
		
		protected ChildEndpointOMXQueryControlAccessor(P owner, String name, AbstractOMXQueryControlAccessor<?, ?, ?> parent, Duration timeout,
				Consumer<OMXQueryControlAccessorState> mutator) {
			super(owner, name, parent, timeout, mutator);
		}
		
		@Override
		public ChildEndpointOMXQueryControl<E> getControl() {
			return ChildEndpointOMXQueryControl.this;
		}
		
		@Override
		protected <X, Z> AbstractOMXQueryControlAccessor<X, E, Z> chained(X owner, String name,
				AbstractOMXQueryControlAccessor<?, ?, ?> parent, Duration timeout, Consumer<OMXQueryControlAccessorState> mutator) {
			return new ChildEndpointOMXQueryControlAccessor<>(owner, name, parent, timeout, mutator);
		}
	}
}
