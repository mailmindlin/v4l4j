package au.edu.jcu.v4l4j.encoder.h264;

import java.nio.ByteBuffer;

import au.edu.jcu.v4l4j.ImagePalette;
import au.edu.jcu.v4l4j.V4L4JUtils;
import au.edu.jcu.v4l4j.encoder.VideoFrameTransformer;
import au.edu.jcu.v4l4j.exceptions.BufferOverflowException;
import au.edu.jcu.v4l4j.exceptions.BufferUnderflowException;

public class H264Encoder implements VideoFrameTransformer {
	
	protected final long object;
	protected final long csp;
	protected int frameNum = 0;
	
	static {
		V4L4JUtils.loadLibrary();
	}
	
	protected final ByteBuffer buffer;
	/**
	 * Allocate & initialize the native object with the given parameters
	 * @param params pointer to parameters object
	 * @return pointer to allocated memory
	 */
	private static native long doInit(long params);
	protected static native long doGetParams(long object);
	protected static native void doSetParams(long object, long pointer);
	
	@Override
	public native void close() throws Exception;
	/**
	 * Encode 
	 * @param object Pointer to x264_t
	 * @param picInObject Pointer to input x264_picture_t
	 * @param output ByteBuffer to write to. Should be direct; may only probably work if not.
	 * @return Output frame size
	 */
	private static native int doEncode(long object, long picInObject, ByteBuffer output);
	
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
		this.buffer = ByteBuffer.allocate(width * height * 3);
	}
	
	public H264Encoder(H264Parameters params) {
		this.csp = params.getCsp();
		this.object = doInit(params.object);
		int width = params.getWidth();
		int height = params.getHeight();
		this.buffer = ByteBuffer.allocateDirect(width * height * 3);//TODO better buffer size
	}

	/**
	 * Get a copy of the parameters
	 * @return parameters
	 */
	public H264Parameters getParameters() {
		return new H264Parameters(this.doGetParams(this.object));
	}
	
	public void setParameters(H264Parameters params) {
		this.doSetParams(this.object, params.object);
	}
	
	@Override
	public int getSourceWidth() {
		return this.getParameters().getWidth();
	}
	
	@Override
	public int getSourceHeight() {
		return this.getParameters().getHeight();
	}
	
	public int encode(H264Picture in, ByteBuffer out) {
		in.setPts(this.frameNum++);
		return doEncode(this.object, in.object, out);
	}
	@Override
	public int apply(ByteBuffer src, ByteBuffer dst) throws BufferUnderflowException, BufferOverflowException, IllegalArgumentException {
		// TODO Auto-generated method stub
		return 0;
	}
	
	@Override
	public int getConverterId() {
		//Not applicable
		return -1;
	}
	
	@Override
	public ImagePalette getSourceFormat() {
		return null;
	}
	
	@Override
	public int estimateSourceLength() {
		// TODO Auto-generated method stub
		return -1;
	}
	
	@Override
	public ImagePalette getDestinationFormat() {
		return ImagePalette.OTHER_KNOWN;
	}
	
	@Override
	public int getDestinationWidth() {
		return getSourceWidth();
	}
	
	@Override
	public int getDestinationHeight() {
		return getSourceHeight();
	}
	
	@Override
	public int estimateDestinationLength() {
		// TODO Auto-generated method stub
		return -1;
	}
	
	@Override
	public long getPointer() {
		//Even though we have a pointer, it's not to a v4lconvert_encoder
		return 0;
	}
}
