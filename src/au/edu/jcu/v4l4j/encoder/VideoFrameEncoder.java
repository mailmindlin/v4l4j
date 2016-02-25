package au.edu.jcu.v4l4j.encoder;

import au.edu.jcu.v4l4j.BaseVideoFrame;
import au.edu.jcu.v4l4j.VideoFrame;
import au.edu.jcu.v4l4j.exceptions.V4L4JException;

/**
 * Encode a VideoFrame of one format to another
 * @author mailmindlin
 */
public interface VideoFrameEncoder extends AutoCloseable {
	/**
	 * Encode the frame to another frame. NOTE: the input frame is not implicitly released, nor is the
	 * output frame, nor is the output frame the same as the input.
	 * @param frame
	 * @return encoded frame
	 * @throws V4L4JException
	 */
	VideoFrame encode(BaseVideoFrame frame) throws V4L4JException;
	/**
	 * Close encoder & release
	 */
	void close() throws Exception;
}
