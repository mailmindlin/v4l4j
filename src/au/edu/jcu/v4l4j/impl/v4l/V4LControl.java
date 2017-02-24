package au.edu.jcu.v4l4j.impl.v4l;

import java.time.Duration;
import java.util.function.Consumer;
import java.util.function.Function;
import java.util.function.Supplier;

import au.edu.jcu.v4l4j.api.control.Control;
import au.edu.jcu.v4l4j.api.control.ControlType;

public abstract class V4LControl<V> implements Control<V> {
	private static native int doGetValueInt(long pointer, int ctrlId);
	
	private static native int doGetValueString(long pointer, int ctrlId);
	
	private static native int doGetValueLong(long pointer, int ctrlId);
	
	private static native int doSetValueInt(long pointer, int ctrlId, int value);
	
	private static native int doSetValueString(long pointer, int ctrlId, int value);
	
	private static native int doSetValueLong(long pointer, int ctrlId, int value);
	
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
			return this.setValue(value);
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
		public ControlAccessor<P, T, R> setTimeout(Duration timeout) {
			return chained(this.mutator == null ? this.parent : this, timeout, null);
		}
		
		@Override
		public ControlAccessor<P, T, R> read(Consumer<T> handler) {
			return andThen(state->handler.accept(state.getValue()));
		}
		
		@Override
		public ControlAccessor<P, T, R> write(Supplier<T> supplier) {
			return andThen(state->state.setValue(supplier.get()));
		}
		
		@Override
		public ControlAccessor<P, T, R> update(Function<T, T> mappingFunction) {
			return andThen(state->state.setValue(mappingFunction.apply(state.getValue())));
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
		public P and() throws IllegalStateException {
			throw new UnsupportedOperationException("Not implemented (yet?)");
		}
		
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
			public ControlAccessor<P, String, String> get() {
				return andThen(state->state.setResult(V4LControl.doGetValueString(V4LStringControl.this.device.pointer, V4LStringControl.this.ctrlId)));
			}

			@Override
			public ControlAccessor<P, String, R> set() {
				return andThen(state->V4LControl.doSetValueString(V4LStringControl.this.device.pointer, V4LStringControl.this.ctrlId, state.getValue()));
			}
			
		}
	}
	
	public static class V4LSwitchControl extends V4LControl<Boolean> {

		protected V4LSwitchControl(VideoDevice device, String name, int ctrlId) {
			super(device, name, ctrlId);
		}

		@Override
		public ControlType getType() {
			return ControlType.SWITCH;
		}

		@Override
		public ControlAccessor<Void, Boolean, Void> access() {
			return null;
		}
		
		protected class V4LSwitchControlAccesssor<P, R> extends V4LControlAccessor<P, Boolean, R> {

			protected V4LSwitchControlAccesssor(V4LControlAccessor<?, ?, ?> parent, Duration timeout, Consumer<V4LControlAccessorState> mutator) {
				super(parent, timeout, mutator);
			}

			@Override
			public ControlAccessor<P, Boolean, R> set() {
				// TODO Auto-generated method stub
				return null;
			}

			@Override
			protected <X, Z> V4LControl<Boolean>.V4LControlAccessor<X, Boolean, Z> chained(V4LControlAccessor<?, ?, ?> parent, Duration timeout,  Consumer<V4LControlAccessorState> mutator) {
				return new V4LSwitchControlAccesssor<>(parent, timeout, mutator);
			}

			@Override
			public ControlAccessor<P, Boolean, Boolean> get() {
				return andThen(state->{
					//TODO finish
					Object _value = state.getValue();
					//TODO map to boolean
					V4LControl.doSetValueInt(V4LSwitchControl.this.device.pointer, V4LSwitchControl.this.ctrlId, -1);
				});
			}
			
		}
		
	}
}
