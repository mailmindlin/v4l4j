package au.edu.jcu.v4l4j.encoder;

import au.edu.jcu.v4l4j.BaseVideoFrame;
import au.edu.jcu.v4l4j.ImagePalette;
import au.edu.jcu.v4l4j.V4L4JConstants;
import au.edu.jcu.v4l4j.VideoFrame;
import au.edu.jcu.v4l4j.exceptions.V4L4JException;

public class JPEGEncoder extends AbstractVideoFrameEncoder {
	
	static {
		try {
			System.loadLibrary("v4l4j");
		} catch (UnsatisfiedLinkError e) {
			System.err.println("Cant load v4l4j JNI library");
			throw e;
		}
	}
	
	static {
		try {
			System.loadLibrary("v4l4j");
		} catch (UnsatisfiedLinkError e) {
			System.err.println("Cant load v4l4j JNI library");
			throw e;
		}
	}
	
	protected int quality;
	
	protected JPEGEncoder(int width, int height, ImagePalette from) {
		super(width, height, from, ImagePalette.JPEG);
	}

	@Override
	public void init() {
		super.init();
	}

	public void setQuality(int quality) {
		if (quality < V4L4JConstants.MIN_JPEG_QUALITY)
			quality = V4L4JConstants.MIN_JPEG_QUALITY;
		if (quality > V4L4JConstants.MAX_JPEG_QUALITY)
			quality = V4L4JConstants.MAX_JPEG_QUALITY;
		this.quality = quality;
	}
	@Override
	public VideoFrame encode(BaseVideoFrame frame) throws V4L4JException {
		super.putBuffer(super.object, frame.getBytes(), frame.getFrameLength());
		
		AbstractConvertedVideoFrame output = getAvailableVideoFrame();
		int frameSize = super.getBuffer(object, output.getBytes());
		
		int bufferIndex= (frame instanceof BaseVideoFrame) ? ((BaseVideoFrame)frame).getBufferIndex() : -1;
		long sequenceNumber = frame.getSequenceNumber();
		long captureUs = frame.getCaptureTime();
		// mark the video frame as available for use
		output.prepareForDelivery(frameSize, bufferIndex, sequenceNumber, captureUs);
		
		return output;
	}

	@Override
	protected void createBuffers(int buffers) {
		for (int i = 0; i < buffers; i++)
			this.frames.add(new ConvertedJPEGVideoFrame(i));
	}
	
}
