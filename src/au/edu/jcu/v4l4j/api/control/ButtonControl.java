package au.edu.jcu.v4l4j.api.control;

public interface ButtonControl extends Control<Void> {
	/**
	 * Trigger this button, and invoke any event
	 */
	void trigger();
}
