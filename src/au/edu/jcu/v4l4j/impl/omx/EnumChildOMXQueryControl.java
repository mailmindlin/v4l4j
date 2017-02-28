package au.edu.jcu.v4l4j.impl.omx;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.time.Duration;
import java.util.Arrays;
import java.util.Iterator;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.Future;
import java.util.function.Consumer;

import au.edu.jcu.v4l4j.api.control.Control;
import au.edu.jcu.v4l4j.api.control.ControlType;
import au.edu.jcu.v4l4j.api.control.DiscreteControl;
import au.edu.jcu.v4l4j.impl.jni.NativeWrapper;

/**
 * A root composite control that when push or pull is called, executes a
 * 
 * @author mailmindlin
 * @param <T>
 */
public class EnumChildOMXQueryControl<T extends Enum<T>> extends AbstractOMXQueryControl<T> implements DiscreteControl<T> {
	protected final Class<T> enumClass;
	protected transient T[] values;
	protected final String structFieldName;

	public EnumChildOMXQueryControl(AbstractOMXQueryControl<?> parent, String name, Class<T> enumClass, String structFieldName) {
		super(parent.component, parent, name);
		this.enumClass = enumClass;
		this.structFieldName = structFieldName;
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
		return ControlType.DISCRETE;
	}

	@Override
	@SuppressWarnings("unchecked")
	public Future<Iterator<T>> options() {
		CompletableFuture<Iterator<T>> result = new CompletableFuture<>();
		try {
			if (values == null) {
				Method valuesMethod = this.enumClass.getMethod("values", new Class<?>[0]);
				this.values = (T[]) valuesMethod.invoke(null, new Object[0]);
			}
			result.complete(Arrays.asList(this.values).iterator());
		} catch (NoSuchMethodException | SecurityException | IllegalAccessException | IllegalArgumentException | InvocationTargetException e) {
			result.completeExceptionally(e);
		}
		return result;
	}

	@Override
	protected <P, R> AbstractOMXQueryControlAccessor<P, T, R> access(AbstractOMXQueryControlAccessor<P, ?, R> parentAccessor) {
		return new ChildOMXQueryControlAccessor<>(false, this.name, parentAccessor, parentAccessor.timeout, null);
	}

	public class ChildOMXQueryControlAccessor<P, R> extends AbstractOMXQueryControlAccessor<P, T, R> {
		protected ChildOMXQueryControlAccessor(boolean isParentOwner, String name, AbstractOMXQueryControlAccessor<?, ?, ?> parent,
				Duration timeout, Consumer<OMXQueryControlAccessorState> mutator) {
			super(isParentOwner, name, parent, timeout, mutator);
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
		public Control<T> getControl() {
			return EnumChildOMXQueryControl.this;
		}

		@Override
		protected <X, Z> AbstractOMXQueryControlAccessor<X, T, Z> chained(boolean isParentOwner, String name,
				AbstractOMXQueryControlAccessor<?, ?, ?> parent, Duration timeout, Consumer<OMXQueryControlAccessorState> mutator) {
			return new ChildOMXQueryControlAccessor<>(isParentOwner, name, parent, timeout, mutator);
		}
	}

}