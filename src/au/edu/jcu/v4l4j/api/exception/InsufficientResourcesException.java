package au.edu.jcu.v4l4j.api.exception;

public class InsufficientResourcesException extends V4L4JException {
	private static final long serialVersionUID = -1416806797205471022L;
	
	public InsufficientResourcesException() {
		super();
	}
	
	public InsufficientResourcesException(String message) {
		super(message);
	}
}
