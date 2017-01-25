package au.edu.jcu.v4l4j.api.component;

public enum ComponentRole {
	UNKNOWN,
	ANY,
	/**
	 * Any source of data (e.g., camera, microphone, tty, clock).
	 * Note that a source can accept clock data, but should not rely on it
	 * (a clock splitter is NOT a source component, even though it may accept
	 * only clock data)
	 */
	SOURCE,
	/**
	 * Any sink of data (e.g., display, speakers, stdout)
	 */
	SINK,
	/**
	 * A component that splits one stream into multiple, identical streams.
	 */
	SPLITTER,
	/**
	 * A component that switches between 
	 */
	SCHEDULER,
	MIXER,
	ENCODER,
	PROCESSOR;
}
