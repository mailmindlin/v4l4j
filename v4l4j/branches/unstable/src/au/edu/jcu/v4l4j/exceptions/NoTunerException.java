/**
 * 
 */
package au.edu.jcu.v4l4j.exceptions;

/**
 * This class of exception is thrown when the <code>Input.getTuner()</code> method is invoked on
 * an input with no tuner (Input.getType() returns Input.CAMERA).
 * @author gilles
 */
public class NoTunerException extends V4L4JException {

	private static final long serialVersionUID = -4596596557974047977L;

	public NoTunerException(String message) {
		super(message);
	}

	public NoTunerException(String message, Throwable throwable) {
		super(message, throwable);
	}

	public NoTunerException(Throwable throwable) {
		super(throwable);
	}

}
