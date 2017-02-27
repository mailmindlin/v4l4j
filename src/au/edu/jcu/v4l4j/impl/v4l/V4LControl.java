package au.edu.jcu.v4l4j.impl.v4l;

import java.time.Duration;
import java.util.Arrays;
import java.util.Iterator;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.Future;
import java.util.function.BiFunction;
import java.util.function.Consumer;
import java.util.function.Function;
import java.util.function.Predicate;
import java.util.function.Supplier;

import au.edu.jcu.v4l4j.api.control.ContinuousControl;
import au.edu.jcu.v4l4j.api.control.Control;
import au.edu.jcu.v4l4j.api.control.ControlType;
import au.edu.jcu.v4l4j.api.control.DiscreteControl;

public abstract class V4LControl<V> implements Control<V> {
	private static native int doGetValueInt(long pointer, int ctrlId);
	
	private static native String doGetValueString(long pointer, int ctrlId);
	
	private static native long doGetValueLong(long pointer, int ctrlId);
	
	private static native void doSetValueInt(long pointer, int ctrlId, int value);
	
	private static native void doSetValueString(long pointer, int ctrlId, String value);
	
	private static native void doSetValueLong(long pointer, int ctrlId, long value);
	
	protected final VideoDevice device;
	protected final int ctrlId;
	protected final String name;
	
	protected V4LControl(VideoDevice device, String name, int ctrlId) {
		this.device = device;
		this.name = name;
		this.ctrlId = ctrlId;
	}
	
	@Override
	public String getName() {
		return this.name;
	}
	
	@Override
	public void close() {
		// TODO Auto-generated method stub
		
	}
	
	protected int getValueInt() {
		return doGetValueInt(this.device.pointer, this.ctrlId);
	}
	
	protected String getValueString() {
		return doGetValueString(this.device.pointer, this.ctrlId);
	}
	
	protected long getValueLong() {
		return doGetValueLong(this.device.pointer, this.ctrlId);
	}
	
	protected void setValueInt(int value) {
		doSetValueInt(this.device.pointer, this.ctrlId, value);
	}
	
	protected void setValueString(String value) {
		doSetValueString(this.device.pointer, this.ctrlId, value);
	}
	
	protected void setValueLong(long value) {
		doSetValueLong(this.device.pointer, this.ctrlId, value);
	}
	
	protected static class V4LControlAccessorState {
		Object result = null;
		Object value = null;
		
		public <V> V setValue(V value) {
			this.value = value;
			return value;
		}
		
		@SuppressWarnings("unchecked")
		public <V> V getValue() {
			return (V) this.value;
		}
		
		public <T> T setResult(T value) {
			this.result = value;
			return value;
		}
		
		@SuppressWarnings("unchecked")
		public <T> T getResult() {
			return (T) this.result;
		}
	}
	
	protected abstract class V4LControlAccessor<P, T, R> implements ControlAccessor<P, T, R> {
		protected final V4LControlAccessor<?, ?, ?> parent;
		protected final Duration timeout;
		protected final Consumer<V4LControlAccessorState> mutator;
		
		protected V4LControlAccessor(V4LControlAccessor<?, ?, ?> parent, Duration timeout, Consumer<V4LControlAccessorState> mutator) {
			this.parent = parent;
			this.timeout = timeout;
			this.mutator = mutator;
		}
		
		@Override
		public V4LControlAccessor<P, T, R> setTimeout(Duration timeout) {
			return chained(this.mutator == null ? this.parent : this, timeout, null);
		}
		
		@Override
		public V4LControlAccessor<P, T, T> get() {
			return andThen(state->state.setResult(state.getValue()));
		}
		
		@Override
		public V4LControlAccessor<P, T, R> get(Consumer<T> handler) {
			return andThen(state->handler.accept(state.getValue()));
		}
		
		@Override
		public <E> ControlAccessor<P, T, E> get(BiFunction<T, R, E> merger) {
			return andThen(state->state.setResult(merger.apply(state.getValue(), state.getResult())));
		}

		@Override
		public V4LControlAccessor<P, T, R> set(T value) {
			return andThen(state->state.setValue(value));
		}
		
		@Override
		public ControlAccessor<P, T, R> set(Supplier<T> supplier) {
			return andThen(state->state.setValue(supplier.get()));
		}
		
		@Override
		public ControlAccessor<P, T, R> update(Function<T, T> mappingFunction) {
			return andThen(state->state.setValue(mappingFunction.apply(state.getValue())));
		}
		
		@Override
		public V4LControlAccessor<P, T, R> update(BiFunction<Control<T>, T, T> mappingFunction) {
			return andThen(state->state.setValue(mappingFunction.apply(getControl(), state.getValue())));
		}
		
		@Override
		public ControlAccessor<P, T, R> increase() {
			return this;
		}
		
		@Override
		public ControlAccessor<P, T, R> decrease() {
			return this;
		}
		
		@Override
		public ControlAccessor<P, T, R> read() {
			return andThen(this::doWrite);
		}

		@Override
		public ControlAccessor<P, T, R> write() {
			return andThen(this::doRead);
		}

		@Override
		public <C extends ControlAccessor<ControlAccessor<P, T, R>, T, R>> C thenIf(Predicate<ControlAccessor<P, T, R>> condition) {
			// TODO Auto-generated method stub
			return null;
		}
		
		@Override
		public P and() throws IllegalStateException {
			throw new UnsupportedOperationException("Not implemented (yet?)");
		}
		
		protected abstract void doWrite(V4LControlAccessorState state);
		
		protected abstract void doRead(V4LControlAccessorState state);
		
		protected void doCall(V4LControlAccessorState state) {
			if (this.parent != null)
				this.parent.doCall(state);
			if (this.mutator != null)
				this.mutator.accept(state);
		}
		
		@Override
		public R call() throws Exception {
			V4LControlAccessorState state = new V4LControlAccessorState();
			this.doCall(state);
			return state.getResult();
		}
		
		protected <X, Z> V4LControlAccessor<X, T, Z> andThen(Consumer<V4LControlAccessorState> mutator) {
			return chained(this.mutator == null ? this.parent : this, this.timeout, mutator);
		}
		
		protected abstract <X, Z> V4LControlAccessor<X, T, Z> chained(V4LControlAccessor<?, ?, ?> parent, Duration timeout,
				Consumer<V4LControlAccessorState> mutator);
	}
	
	public static class V4LStringControl extends V4LControl<String> {
		
		protected V4LStringControl(VideoDevice device, String name, int ctrlId) {
			super(device, name, ctrlId);
		}
		
		@Override
		public ControlType getType() {
			return ControlType.STRING;
		}
		
		@Override
		public ControlAccessor<Void, String, Void> access() {
			return new V4LStringControlAccessor<>(null, null, null);
		}

		@Override
		public boolean isDiscrete() {
			return false;
		}

		@Override
		public boolean isContinuous() {
			return false;
		}
		
		public class V4LStringControlAccessor<P, R> extends V4LControlAccessor<P, String, R> {
			
			protected V4LStringControlAccessor(V4LControlAccessor<?, ?, ?> parent, Duration timeout, Consumer<V4LControlAccessorState> mutator) {
				super(parent, timeout, mutator);
			}

			@Override
			protected <X, Z> V4LControl<String>.V4LControlAccessor<X, String, Z> chained(V4LControlAccessor<?, ?, ?> parent, Duration timeout,
					Consumer<V4LControlAccessorState> mutator) {
				return new V4LStringControlAccessor<>(parent, timeout, mutator);
			}
			
			@Override
			public V4LStringControl getControl() {
				return V4LStringControl.this;
			}

			@Override
			protected void doWrite(V4LControlAccessorState state) {
				V4LStringControl.this.setValueString(state.getValue());
			}

			@Override
			protected void doRead(V4LControlAccessorState state) {
				state.setResult(V4LStringControl.this.getValueString());
			}
		}
	}
	
	public static class V4LSwitchControl extends V4LControl<Boolean> implements DiscreteControl<Boolean> {

		protected V4LSwitchControl(VideoDevice device, String name, int ctrlId) {
			super(device, name, ctrlId);
		}

		@Override
		public ControlType getType() {
			return ControlType.SWITCH;
		}

		@Override
		public ControlAccessor<Void, Boolean, Void> access() {
			return new V4LSwitchControlAccesssor<>(null, null, null);
		}

		@Override
		public Future<Iterator<Boolean>> options() {
			return CompletableFuture.completedFuture(Arrays.asList(true, false).iterator());
		}

		@Override
		public boolean isDiscrete() {
			return false;
		}

		@Override
		public boolean isContinuous() {
			return false;
		}
		
		protected class V4LSwitchControlAccesssor<P, R> extends V4LControlAccessor<P, Boolean, R> {

			protected V4LSwitchControlAccesssor(V4LControlAccessor<?, ?, ?> parent, Duration timeout, Consumer<V4LControlAccessorState> mutator) {
				super(parent, timeout, mutator);
			}
			
			@Override
			protected <X, Z> V4LControl<Boolean>.V4LControlAccessor<X, Boolean, Z> chained(V4LControlAccessor<?, ?, ?> parent, Duration timeout,  Consumer<V4LControlAccessorState> mutator) {
				return new V4LSwitchControlAccesssor<>(parent, timeout, mutator);
			}

			@Override
			public Control<Boolean> getControl() {
				return V4LSwitchControl.this;
			}
			
			@Override
			public ControlAccessor<P, Boolean, R> increase() {
				return set(true);
			}
			
			@Override
			public ControlAccessor<P, Boolean, R> decrease() {
				return set(false);
			}

			@Override
			protected void doWrite(V4LControlAccessorState state) {
				Object _value = state.getValue();
				boolean bValue;
				try {
					bValue = (Boolean) _value;
				} catch (NullPointerException | ClassCastException e) {
					bValue = Boolean.parseBoolean("" + _value);
				}
				V4LSwitchControl.this.setValueInt(bValue ? 1 : 0);
			}

			@Override
			protected void doRead(V4LControlAccessorState state) {
				int iValue = V4LSwitchControl.this.getValueInt();
				state.setValue(iValue != 0);
			}
		}
	}
	
	public static abstract class V4LSliderControl extends V4LControl<Number> implements ContinuousControl<Number> {
		protected final Number minValue;
		protected final Number maxValue;
		protected final Number step;
		
		protected V4LSliderControl(VideoDevice device, String name, int ctrlId, Number min, Number max, Number step) {
			super(device, name, ctrlId);
			this.minValue = min;
			this.maxValue = max;
			this.step = step;
		}

		@Override
		public ControlType getType() {
			return ControlType.SLIDER;
		}

		@Override
		public ControlAccessor<Void, Number, Void> access() {
			return new V4LSliderControlAccessor<>(null, null, null);
		}

		@Override
		public Future<ContinuousRange<Number>> getRange() {
			return CompletableFuture.completedFuture(new ContinuousRange<Number>() {
				@Override
				public Number minimum() {
					return minValue;
				}

				@Override
				public Number step() {
					return step;
				}

				@Override
				public Number defaultValue() {
					return 0;
				}

				@Override
				public Number maximum() {
					return maxValue;
				}
				
			});
		}
		
		protected class V4LSliderControlAccessor<P, R> extends V4LControlAccessor<P, Number, R> {

			protected V4LSliderControlAccessor(V4LControlAccessor<?, ?, ?> parent, Duration timeout, Consumer<V4LControlAccessorState> mutator) {
				super(parent, timeout, mutator);
			}

			@Override
			public Control<Number> getControl() {
				return V4LSliderControl.this;
			}

			@Override
			protected void doRead(V4LControlAccessorState state) {
				state.setValue(V4LSliderControl.this.getValueLong());
			}
			
			protected void doWrite(V4LControlAccessorState state) {
				Object value = state.getValue();
				long lValue;
				if (value == null)
					lValue = 0;
				else
					lValue = ((Number)value).longValue();
				V4LSliderControl.this.setValueLong(lValue);
			}

			@Override
			protected <X, Z> V4LControlAccessor<X, Number, Z> chained(V4LControl<Number>.V4LControlAccessor<?, ?, ?> parent, Duration timeout,
					Consumer<V4LControlAccessorState> mutator) {
				return new V4LSliderControlAccessor<>(parent, timeout, mutator);
			}
		}
	}
}
