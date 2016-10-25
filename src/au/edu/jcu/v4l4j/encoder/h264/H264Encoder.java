package au.edu.jcu.v4l4j.encoder.h264;

import java.nio.BufferUnderflowException;
import java.nio.ByteBuffer;

import au.edu.jcu.v4l4j.BaseVideoFrame;
import au.edu.jcu.v4l4j.ImagePalette;
import au.edu.jcu.v4l4j.VideoFrame;
import au.edu.jcu.v4l4j.encoder.V4lconvertBuffer;
import au.edu.jcu.v4l4j.encoder.VideoFrameEncoder;
import au.edu.jcu.v4l4j.exceptions.BufferOverflowException;
import au.edu.jcu.v4l4j.exceptions.V4L4JException;

public class H264Encoder implements VideoFrameEncoder {
	
	protected final long object;
	protected final long csp;
	protected int frameNum = 0;
	
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
			params.initWithPreset(X264.Preset.MEDIUM, X264.Tune.ZERO_LATENCY);
			params.setCsp(csp);
			params.setInputDimension(width, height);
			params.setVfrInput(false);
			params.setRepeatHeaders(true);
			params.setAnnexb(true);
			params.applyProfile(X264.Profile.HIGH);
		
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
	@Override
	public int encode(V4lconvertBuffer buffer)
			throws BufferUnderflowException, BufferOverflowException, NullPointerException, V4L4JException {
		// TODO Auto-generated method stub
		return 0;
	}
	@Override
	public ImagePalette getSourcePalette() {
		// TODO Auto-generated method stub
		return null;
	}
	@Override
	public int getSourceWidth() {
		// TODO Auto-generated method stub
		return 0;
	}
	@Override
	public int getSourceHeight() {
		// TODO Auto-generated method stub
		return 0;
	}
	@Override
	public ImagePalette getOutputPalette() {
		// TODO Auto-generated method stub
		return null;
	}
	@Override
	public int getOutputWidth() {
		// TODO Auto-generated method stub
		return 0;
	}
	@Override
	public int getOutputHeight() {
		// TODO Auto-generated method stub
		return 0;
	}
	@Override
	public int minimumSourceBufferLength() {
		// TODO Auto-generated method stub
		return 0;
	}
	@Override
	public int minimumOutputBufferLength() {
		// TODO Auto-generated method stub
		return 0;
	}
	@Override
	public int minimumIntermediateBufferLength() {
		// TODO Auto-generated method stub
		return 0;
	}
}
