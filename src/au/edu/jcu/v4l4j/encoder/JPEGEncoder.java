package au.edu.jcu.v4l4j.encoder;

import au.edu.jcu.v4l4j.BaseVideoFrame;
import au.edu.jcu.v4l4j.ImagePalette;
import au.edu.jcu.v4l4j.V4L4JConstants;
import au.edu.jcu.v4l4j.VideoFrame;
import au.edu.jcu.v4l4j.exceptions.V4L4JException;

public class JPEGEncoder extends AbstractVideoFrameEncoder {
	
	protected int quality;
	
	public static JPEGEncoder from(int width, int height, ImagePalette from) {
		return new JPEGEncoder(width, height, from);
	}
	
	public static JPEGEncoder to(int width, int height, ImagePalette to) {
		return new JPEGEncoder(width, height, ImagePalette.JPEG, to);
	}
	
	protected JPEGEncoder(int width, int height, ImagePalette from, ImagePalette to) {
		super(width, height, from, to);
	}
	
	public JPEGEncoder(int width, int height, ImagePalette from) {
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
		super.putBuffer(frame.getBytes(), frame.getFrameLength());
		
		AbstractConvertedVideoFrame output = getAvailableVideoFrame();
		int frameSize = super.getBuffer(output.getBytes());
		
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
