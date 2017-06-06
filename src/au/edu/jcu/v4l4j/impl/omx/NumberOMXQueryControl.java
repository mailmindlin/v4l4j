package au.edu.jcu.v4l4j.impl.omx;

import java.time.Duration;
import java.util.function.Consumer;

import au.edu.jcu.v4l4j.api.control.Control;
import au.edu.jcu.v4l4j.api.control.ControlType;

/**
 * @author mailmindlin
 */
public class NumberOMXQueryControl extends AbstractOMXQueryControl<Number> {
	
	public NumberOMXQueryControl(AbstractOMXQueryControl<?> parent, int port, String name, String structFieldName, OMXOptionEnumeratorPrototype<Number> enumerator) {
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
	protected <P, R> AbstractOMXQueryControlAccessor<P, Number, R> access(AbstractOMXQueryControlAccessor<P, ?, R> parentAccessor) {
		return new NumberOMXQueryControlAccessor<>((P) parentAccessor, this.name, null, parentAccessor.timeout, null);
	}

	public class NumberOMXQueryControlAccessor<P, R> extends AbstractOMXQueryControlAccessor<P, Number, R> {

		protected NumberOMXQueryControlAccessor(P owner, String name, AbstractOMXQueryControlAccessor<?, ?, ?> parent,
				Duration timeout, Consumer<OMXQueryControlAccessorState> mutator) {
			super(owner, name, parent, timeout, mutator);
		}

		@Override
		public Control<Number> getControl() {
			return NumberOMXQueryControl.this;
		}

		@Override
		protected <X, Z> AbstractOMXQueryControlAccessor<X, Number, Z> chained(X owner, String name,
				AbstractOMXQueryControlAccessor<?, ?, ?> parent, Duration timeout, Consumer<OMXQueryControlAccessorState> mutator) {
			return new NumberOMXQueryControlAccessor<>(owner, name, parent, timeout, mutator);
		}

	}
}
