package au.edu.jcu.v4l4j.api.control;

public interface Control extends AutoCloseable {
	ControlType getType();
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
	
	void increase();
	
	void decrease();
	
	void close();
}
