package au.edu.jcu.v4l4j.encoder;

public interface VideoFrameConverter extends VideoFrameTransformer {
	/**
	 * Wrap a pointer in a Java object
	 * @param ptr
	 * @return
	 */
	public static VideoFrameConverter wrap(long ptr) {
		//TODO finish
		return null;
	}
}
