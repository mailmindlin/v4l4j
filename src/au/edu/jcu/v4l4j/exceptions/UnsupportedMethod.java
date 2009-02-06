package au.edu.jcu.v4l4j.exceptions;

/**
 * Exceptions of this type are thrown when calling an incorrect method on a control
 * @author gilles
 *
 */
public class UnsupportedMethod extends RuntimeException {
	private static final long serialVersionUID = -8801339441741012577L;

	public UnsupportedMethod(String message) {
		super(message);
	}

	public UnsupportedMethod(String message, Throwable throwable) {
		super(message, throwable);
	}

	public UnsupportedMethod(Throwable throwable) {
		super(throwable);
	}
}
