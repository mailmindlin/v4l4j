package au.edu.jcu.v4l4j.exceptions;

public class ReleaseException extends RuntimeException {

	private static final long serialVersionUID = -7410432021368200123L;

	public ReleaseException(String message) {
		super(message);
	}

	public ReleaseException(String message, Throwable throwable) {
		super(message, throwable);
	}

	public ReleaseException(Throwable throwable) {
		super(throwable);
	}
}
