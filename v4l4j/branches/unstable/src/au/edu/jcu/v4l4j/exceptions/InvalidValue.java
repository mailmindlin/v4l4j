package au.edu.jcu.v4l4j.exceptions;

/**
 * Exceptions of this type are thrown when trying ti set an incorrect value on a control. 
 * @author gilles
 *
 */
public class InvalidValue extends RuntimeException {
	private static final long serialVersionUID = -3506474708008499397L;

	public InvalidValue(String message) {
		super(message);
	}

	public InvalidValue(String message, Throwable throwable) {
		super(message, throwable);
	}

	public InvalidValue(Throwable throwable) {
		super(throwable);
	}
}
