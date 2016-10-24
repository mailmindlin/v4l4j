package au.edu.jcu.v4l4j.exceptions;

public class BufferUnderflowException extends RuntimeException {
	private static final long serialVersionUID = -4560848386021921029L;
	
	public BufferUnderflowException() {
		super();
	}
	
	public BufferUnderflowException(String msg) {
		super(msg);
	}
	
	public BufferUnderflowException(Throwable cause) {
		super(cause);
	}
	
	public BufferUnderflowException(Throwable cause, String msg) {
		super(msg);
		super.initCause(cause);
	}
}
