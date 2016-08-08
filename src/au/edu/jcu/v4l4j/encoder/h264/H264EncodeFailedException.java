package au.edu.jcu.v4l4j.encoder.h264;

public class H264EncodeFailedException extends RuntimeException {
	private static final long serialVersionUID = 4899262301372561043L;

	public H264EncodeFailedException(String reason) {
		super(reason);
	}
}
