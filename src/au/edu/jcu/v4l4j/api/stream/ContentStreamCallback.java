package au.edu.jcu.v4l4j.api.stream;

/**
 * Callback invoked on a ContentStream event
 * @author mailmindlin
 */
@FunctionalInterface
public interface ContentStreamCallback {
	void call(ContentStream stream, ContentStreamEventType type);
}
