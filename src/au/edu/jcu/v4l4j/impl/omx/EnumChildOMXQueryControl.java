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
public abstract class EnumChildOMXQueryControl<T extends Enum<T>> extends AbstractOMXQueryControl<T> {
	protected final Class<T> enumClass;
	protected final String structFieldName;
	
	public ChildOMXQueryControl(AbstractOMXQueryControl<?> parent, String name, Class<T> enumClass, String structFieldName) {
		super(parent.control, parent, name);
		this.enumClass = enumClass;
		this.structFieldName = structFieldName;
	}
	
	public class ChildOMXQueryControlAccessor<P, R> extends AbstractOMXQueryControlAccessor<P, T, R> {
		protected ChildOMXQueryControlAccessor(boolean isParentOwner, String name,
				AbstractOMXQueryControlAccessor<?, ?, ?> parent, Duration timeout,
				Consumer<OMXQueryControlAccessorState> mutator) {
			super(isParentOwner, name, parent, timeout, mutator);
		}
		
		@Override
		protected OMXQueryControlAccessorState enterFromParent(OMXQueryControlAccessorState parentState) {
			OMXQueryControlAccessorState state = super.enterFromParent(parentState);
			NativeWrapper parentStruct = parentState.localPointer();
			state.localPointer = parentStruct.getChildRemote(structFieldName);
			if (state.localPointer == null)
				state.localPointer = parentState.getChild(structFieldName);
			if (state.localPointer == null)
				throw new RuntimeException("Control " + getName() + ": Can't find field " + structFieldName + " on parent struct " + parentStruct);
			return state;
		}
	}
}