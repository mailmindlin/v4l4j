package au.edu.jcu.v4l4j.api.control;

public enum ControlType {
	/**
	 * A button control.
	 */
	BUTTON,
	/**
	 * A range slider
	 */
	SLIDER,
	SWITCH,
	DISCRETE,
	MENU,
	STRING,
	LONG,
	BITMASK,
	COMPOSITE;
}
