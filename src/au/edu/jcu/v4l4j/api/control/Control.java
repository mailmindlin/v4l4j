package au.edu.jcu.v4l4j.api.control;

public interface Control extends AutoCloseable {
	/**
	 * Get the type of this control.
	 * The returned value for this method should be constant across calls.
	 * @return Type of this control
	 */
	ControlType getType();
	
	/**
	 * Get the name of this control.
	 * The returned value for this method should be constant across calls.
	 * @return this control's name
	 */
	String getName();
	
	/**
	 * Pushes the control's state in memory to the device.
	 * Operates on sub-controls
	 */
	void push();
	
	/**
	 * Updates the control's memory state from the device
	 * Operates on sub-controls.
	 */
	void pull();
	
	void close();
}
