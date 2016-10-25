package au.edu.jcu.v4l4j.encoder;

import java.nio.BufferUnderflowException;
import java.nio.ByteBuffer;

import au.edu.jcu.v4l4j.ImagePalette;
import au.edu.jcu.v4l4j.exceptions.BufferOverflowException;
import au.edu.jcu.v4l4j.exceptions.V4L4JException;

/**
 * Encode a VideoFrame of one format to another
 * @author mailmindlin
 */
public interface VideoFrameEncoder extends AutoCloseable {
	/**
	 * 
	 * @throws V4L4JException
	 */
	int encode(V4lconvertBuffer buffer) throws BufferUnderflowException, BufferOverflowException, NullPointerException, V4L4JException;
	ImagePalette getSourcePalette();
	int getSourceWidth();
	int getSourceHeight();
	ImagePalette getOutputPalette();
	int getOutputWidth();
	int getOutputHeight();
	int minimumSourceBufferLength();
	int minimumOutputBufferLength();
	int minimumIntermediateBufferLength();
	/**
	 * Close encoder & release
	 */
	void close() throws Exception;
}
