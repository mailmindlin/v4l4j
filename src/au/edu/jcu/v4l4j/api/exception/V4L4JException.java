package au.edu.jcu.v4l4j.api.exception;

public class V4L4JException extends RuntimeException {
	private static final long serialVersionUID = -6499207881416390904L;
	
	public V4L4JException() {
		super();
	}

	public V4L4JException(String msg) {
		super(msg);
	}

	public V4L4JException(Throwable cause) {
		super(cause);
	}

	public V4L4JException(String message, Throwable cause) {
		super(message, cause);
	}

	protected V4L4JException(String msg, Throwable cause, boolean enableSuppression, boolean writableStackTrace) {
		super(msg, cause, enableSuppression, writableStackTrace);
	}
}
