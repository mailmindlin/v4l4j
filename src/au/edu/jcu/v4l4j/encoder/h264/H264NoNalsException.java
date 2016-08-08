package au.edu.jcu.v4l4j.encoder.h264;

public class H264NoNalsException extends RuntimeException {
	private static final long serialVersionUID = -2235575879088128945L;

	public H264NoNalsException(String reason) {
		super(reason);
	}
}
