package au.edu.jcu.v4l4j.impl.omx;

import au.edu.jcu.v4l4j.exceptions.JNIException;

public class OMXException extends JNIException {
	private static final long serialVersionUID = 5609869889551404926L;
	protected final int errorCode;
	public OMXException(String message) {
		//Let's try not to throw any exception from here
		super(message.length() > 8 ? message.substring(8) : message);
		
		int ec = 0;
		try {
			//Parse as long because parseInt throws NumberFormatException when value > INT_MAX
			ec = (int) Long.parseLong(message.substring(0, 8), 16);
		} catch (NumberFormatException e) {
			//Swallow
		} finally {
			this.errorCode = ec;
		}
	}
	
	public OMXException(String message, int code) {
		super(message);
		this.errorCode = code;
	}
	
	public int getErrorCode() {
		return this.errorCode;
	}
}
