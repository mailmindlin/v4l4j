package au.edu.jcu.v4l4j.api.control;

public interface ContinuousControl<T> extends Control<T> {
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
