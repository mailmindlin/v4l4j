package au.edu.jcu.v4l4j.api.control;

public enum ControlType {
	/**
	 * A button control.
	 */
	BUTTON,
	/**
	 * A range slider. Accepts a range of values between a defined minimum and maximum, with
	 * a specified minimum step.
	 */
	SLIDER,
	/**
	 * A switch that has two discrete values: 0 and 1
	 */
	SWITCH,
	DISCRETE,
	CONTINUOUS,
	STRING,
	LONG,
	BITMASK,
	/**
	 * A control that has no inherent value, yet contains sub-controls that can be accessed
	 */
	COMPOSITE;
}
