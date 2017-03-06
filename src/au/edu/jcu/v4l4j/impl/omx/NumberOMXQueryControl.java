package au.edu.jcu.v4l4j.impl.omx;

import java.time.Duration;
import java.util.function.Consumer;

import au.edu.jcu.v4l4j.api.control.Control;
import au.edu.jcu.v4l4j.api.control.ControlType;

public class NumberOMXQueryControl extends AbstractOMXQueryControl<Number> {

	public NumberOMXQueryControl(AbstractOMXQueryControl<?> parent, String name, String structFieldName) {
		super(parent.component, parent, name, structFieldName);
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
	protected <P, R> AbstractOMXQueryControlAccessor<P, Number, R> access(AbstractOMXQueryControlAccessor<P, ?, R> parentAccessor) {
		return new NumberOMXQueryControlAccessor<>(false, this.name, parentAccessor, parentAccessor.timeout, null);
	}

	public class NumberOMXQueryControlAccessor<P, R> extends AbstractOMXQueryControlAccessor<P, Number, R> {

		protected NumberOMXQueryControlAccessor(boolean isParentOwner, String name, AbstractOMXQueryControlAccessor<?, ?, ?> parent,
				Duration timeout, Consumer<OMXQueryControlAccessorState> mutator) {
			super(isParentOwner, name, parent, timeout, mutator);
		}

		@Override
		public Control<Number> getControl() {
			return NumberOMXQueryControl.this;
		}

		@Override
		protected <X, Z> AbstractOMXQueryControlAccessor<X, Number, Z> chained(boolean isParentOwner, String name,
				AbstractOMXQueryControlAccessor<?, ?, ?> parent, Duration timeout, Consumer<OMXQueryControlAccessorState> mutator) {
			return new NumberOMXQueryControlAccessor<>(isParentOwner, name, parent, timeout, mutator);
		}

	}

}
