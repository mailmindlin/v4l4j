package au.edu.jcu.v4l4j.exceptions;

public class InUseException extends RuntimeException {
	private static final long serialVersionUID = 5702184953142802622L;

	public InUseException(String message) {
		super(message);
	}

	public InUseException(String message, Throwable throwable) {
		super(message, throwable);
	}

	public InUseException(Throwable throwable) {
		super(throwable);
	}
}
