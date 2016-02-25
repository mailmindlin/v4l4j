package au.edu.jcu.v4l4j.encoder.h264;

import java.nio.ByteBuffer;

import au.edu.jcu.v4l4j.BaseVideoFrame;
import au.edu.jcu.v4l4j.VideoFrame;
import au.edu.jcu.v4l4j.encoder.VideoFrameEncoder;
import au.edu.jcu.v4l4j.exceptions.V4L4JException;

public class H264Encoder implements VideoFrameEncoder {
	
	protected final long object;
	
	static {
		try {
			System.loadLibrary("v4l4j");
		} catch (UnsatisfiedLinkError e) {
			System.err.println("Cant load v4l4j JNI library");
			throw e;
		}
	}
	
	protected final ByteBuffer buffer;
	/**
	 * Allocate & initialize the native object with the given parameters
	 * @param params pointer to parameters object
	 * @return pointer to allocated memory
	 */
	protected native long doInit(long params);
	protected native long doGetParams();
	protected native void doSetParams(long ptr);
	/**
	 * Update 
	 * @param params
	 */
	protected native void updateParams(long params);
	protected native void doRelease();
	protected native int doEncode(ByteBuffer buffer);
	
	public H264Encoder(H264Parameters params) {
		this.object = doInit(params.object);
		int width = params.getWidth();
		int height = params.getHeight();
		buffer = ByteBuffer.allocateDirect(width * height * 3);//TODO better buffer size
	}

	@Override
	public VideoFrame encode(BaseVideoFrame frame) throws V4L4JException {
		buffer.reset();
		buffer.put(frame.getBytes(), 0, frame.getFrameLength());
		doEncode(buffer);
		return null;
	}
	
	@Override
	public native void close() throws Exception;
	
	
	/**
	 * Get a copy of the parameters
	 * @return parameters
	 */
	public H264Parameters getParameters() {
		return new H264Parameters(this.doGetParams());
	}
	
	public void setParameters(H264Parameters params) {
		this.doSetParams(params.object);
	}
}
