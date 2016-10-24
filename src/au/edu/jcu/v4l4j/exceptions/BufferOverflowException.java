package au.edu.jcu.v4l4j.exceptions;

public class BufferOverflowException extends RuntimeException {
	private static final long serialVersionUID = -1123590213642777566L;
	
	public BufferOverflowException() {
		super();
	}
	
	public BufferOverflowException(String msg) {
		super(msg);
	}
	
	public BufferOverflowException(Throwable cause) {
		super(cause);
	}
	
	public BufferOverflowException(Throwable cause, String msg) {
		super(msg);
		super.initCause(cause);
	}
}
