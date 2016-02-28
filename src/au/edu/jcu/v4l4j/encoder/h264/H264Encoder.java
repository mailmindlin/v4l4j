package au.edu.jcu.v4l4j.encoder.h264;

import java.nio.ByteBuffer;

import au.edu.jcu.v4l4j.BaseVideoFrame;
import au.edu.jcu.v4l4j.VideoFrame;
import au.edu.jcu.v4l4j.encoder.VideoFrameEncoder;
import au.edu.jcu.v4l4j.exceptions.V4L4JException;

public class H264Encoder implements VideoFrameEncoder {
	
	protected final long object;
	protected final long csp;
	
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
	protected native void doSetParams(long pointer);
	
	@Override
	public native void close() throws Exception;
	/**
	 * Encode 
	 * @param in pointer to the input picture
	 * @param out pointer to the output picture
	 * @return success
	 */
	protected native int doEncode(long in, long out);
	
	public H264Encoder(int width, int height, int csp) {
		this.csp = csp;
		try (H264Parameters params = new H264Parameters()) {
			params.initWithPreset(X264.PRESET_MEDIUM, X264.TUNE_ZERO_LATENCY);
			params.setCsp(csp);
			params.setInputDimension(width, height);
			params.setVfrInput(false);
			params.setRepeatHeaders(true);
			params.setAnnexb(true);
			params.applyProfile(X264.PROFILE_HIGH);
		
			this.object = doInit(params.object);
		}
		buffer = ByteBuffer.allocate(width * height * 3);
	}
	public H264Encoder(H264Parameters params) {
		this.csp = params.getCsp();
		this.object = doInit(params.object);
		int width = params.getWidth();
		int height = params.getHeight();
		buffer = ByteBuffer.allocateDirect(width * height * 3);//TODO better buffer size
	}

	@Override
	public VideoFrame encode(BaseVideoFrame frame) throws V4L4JException {
		return null;
	}
	
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
