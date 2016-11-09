package au.edu.jcu.v4l4j.encoder;

import java.nio.ByteBuffer;

import au.edu.jcu.v4l4j.ImagePalette;
import au.edu.jcu.v4l4j.exceptions.BufferOverflowException;
import au.edu.jcu.v4l4j.exceptions.BufferUnderflowException;

/**
 * Encode a VideoFrame of one format to another
 * 
 * @author mailmindlin
 */
public interface VideoFrameEncoder extends AutoCloseable {
	
	default int apply(V4lconvertBuffer buf) {
		buf.pull();
		int result = apply(buf.getSourceBuffer(), buf.getOutputBuffer());
		buf.push();
		return result;
	}
	
	int apply(ByteBuffer src, ByteBuffer dst) throws BufferUnderflowException, BufferOverflowException, IllegalArgumentException;
	
	int getConverterId();
	
	ImagePalette getSourceFormat();
	
	int getSourceWidth();
	
	int getSourceHeight();
	
	int estimateSourceLength();
	
	ImagePalette getDestinationFormat();
	
	int getDestinationWidth();
	
	int getDestinationHeight();
	
	int estimateDestinationLength();
	
	/**
	 * Close encoder & release
	 */
	void close() throws Exception;
}
