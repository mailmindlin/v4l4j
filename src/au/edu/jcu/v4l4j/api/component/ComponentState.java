package au.edu.jcu.v4l4j.api.component;

public enum ComponentState {
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
