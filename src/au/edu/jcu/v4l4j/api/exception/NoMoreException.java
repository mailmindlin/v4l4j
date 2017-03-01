package au.edu.jcu.v4l4j.api.exception;

public class NoMoreException extends V4L4JException {
	private static final long serialVersionUID = 6893738802622280438L;
	
	public NoMoreException() {
		super();
	}
	
	public NoMoreException(String message) {
		super(message);
	}
}
