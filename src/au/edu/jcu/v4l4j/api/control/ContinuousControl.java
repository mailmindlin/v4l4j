package au.edu.jcu.v4l4j.api.control;

import java.util.concurrent.Future;

public interface ContinuousControl<T> extends Control<T> {
	
	Future<ContinuousRange<T>> getRange();
	
	@Override
	default ControlType getType() {
		return ControlType.CONTINUOUS;
	}
	
	public static interface ContinuousRange<T> {
		T minimum();
		T step();
		T defaultValue();
		T maximum();
	}
}
