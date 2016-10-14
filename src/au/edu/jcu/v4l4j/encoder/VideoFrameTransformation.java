package au.edu.jcu.v4l4j.encoder;

public class VideoFrameTransformation {
	protected final long ptr;
	protected VideoFrameTransformation(long ptr) {
		this.ptr = ptr;
	}
	public native int getConverterId();
}
