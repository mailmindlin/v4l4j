package au.edu.jcu.v4l4j.exceptions;

/**
 * Exceptions of this type are thrown when an JNI-related error occurs (unable to find a class
 * , unable to instantiate an object, ...) 
 * @author gilles
 *
 */
public class JNIException extends RuntimeException {

	private static final long serialVersionUID = -389938018936315368L;

	public JNIException(String message) {
		super(message);
	}

	public JNIException(String message, Throwable throwable) {
		super(message, throwable);
	}

	public JNIException(Throwable throwable) {
		super(throwable);
	}
}