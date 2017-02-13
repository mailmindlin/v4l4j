package au.edu.jcu.v4l4j.api.component;

public enum ComponentState {
	/**
	 * State where the component has decided that it is unable to
	 * operate in any way (e.g., its internal data structures have been
	 * corrupted)
	 */
	INVALID,
	/**
	 * 
	 */
	UNLOADED,
	/**
	 * State that a component is in after being acquired, but before 
	 */
	LOADED,
	WAIT_FOR_RESOURCES,
	IDLE,
	EXECUTING,
	PAUSED;
}
